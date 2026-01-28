//
// Created by churunfa on 2026/1/25.
//

#include "CombinationGraph.h"

#include "exec/base/BaseOperationProcess.h"
#include "repo/DatabaseManager.h"

void deleteGraphCore(const int combination_id) {
    const auto combination = db.get_pointer<Combination>(combination_id);
    if (!combination) {
        return;
    }
    // 删除边
    db.remove_all<CombinationEdge>(
    where(
        c(&CombinationEdge::combination_id) == combination->id
        )
    );
    // 删除节点
    db.remove_all<CombinationNode>(
    where(
        c(&CombinationNode::combination_id) == combination->id
        )
    );
    // 删除整体信息
    db.remove_all<Combination>(
    where(
        c(&Combination::project_name) == combination->project_name
        and
        c(&Combination::combination_name) == combination->combination_name
        )
    );
}

void CombinationRepo::deleteGraph(const int combination_id) {
    try {
        deleteGraphCore(combination_id);
    } catch (const std::exception& e) {
        std::cerr << "Transaction failed: " << e.what() << std::endl;
    }
}

Combination copyCombination(const std::optional<const Combination>& combination) {
    return Combination{combination->id, combination->project_name, combination->combination_name, combination->desc, combination->min_time};
}

std::vector<CombinationNode> copyCombinationNodes(const std::vector<const CombinationNode*>& combination_nodes) {
    std::vector<CombinationNode> combination_nodes_copy;
    combination_nodes_copy.reserve(combination_nodes.size());
for (const auto combination_node : combination_nodes) {
        combination_nodes_copy.emplace_back(*combination_node);
    }
    return combination_nodes_copy;
}

std::vector<CombinationEdge> copyCombinationEdges(const std::vector<const CombinationEdge*>& combination_edges) {
    std::vector<CombinationEdge> combination_edges_copy;
    combination_edges_copy.reserve(combination_edges.size());
    for (const auto combination_edge : combination_edges) {
        combination_edges_copy.emplace_back(*combination_edge);
    }
    return combination_edges_copy;
}


void updateOrSaveGraphCore(const CombinationGraph &graph, const bool insert) {
    const std::optional<const Combination>& combination = graph.getCombination();
    const std::vector<const CombinationNode*> combination_nodes = graph.getCombinationNode();
    const std::vector<const CombinationEdge*> combination_edges = graph.getCombinationEdge();

    auto copy_combination = copyCombination(combination);
    auto copy_nodes = copyCombinationNodes(combination_nodes);
    auto copy_edges = copyCombinationEdges(combination_edges);

    try {
        int combination_id = combination -> id;
        if (insert) {
             combination_id = IdGenerateRepo::generateId("combination");
        }
        db.transaction([&] {
            if (!insert) {
                deleteGraphCore(combination -> id);
            } else {
                // 检查是否重复
                if (const auto query_combination = db.get_pointer<Combination>(combination_id)) {
                    throw std::out_of_range("Duplicate primary key");
                }
                if (const auto query_combinations = db.get_all<Combination>(where(c(&Combination::project_name) == combination->project_name and c(&Combination::combination_name) == combination->combination_name)); !query_combinations.empty()) {
                    throw std::out_of_range("Duplicate primary key");
                }
            }
            copy_combination.id = combination_id;
            db.replace(copy_combination);

            for (auto& node : copy_nodes) {
                node.combination_id = copy_combination.id;
                db.insert(node);
            }

            for (auto& edge : copy_edges) {
                edge.combination_id = copy_combination.id;
                db.insert(edge);
            }
            return true; // 提交事务
        });
    } catch (const std::exception& e) {
        std::cerr << "Transaction failed: " << e.what() << std::endl;
        throw std::out_of_range("写入失败");
    }
}

void CombinationRepo::insertGraph(const CombinationGraph &graph) {
    updateOrSaveGraphCore(graph, true);
}

void CombinationRepo::updateGraph(const CombinationGraph &graph) {
    updateOrSaveGraphCore(graph, false);
}

std::vector<std::string> CombinationRepo::allProject() {
    return db.select(distinct(&Combination::project_name));
}

std::vector<Combination> CombinationRepo::allGraph(const std::string &project_name) {
    return  db.get_all<Combination>(where(c(&Combination::project_name) == project_name));
}

std::vector<CombinationNode> allCombinationNode(const int combination_id) {
    auto combination_nodes = db.get_all<CombinationNode>(where(c(&CombinationNode::combination_id) == combination_id));
    for (const auto& node : combination_nodes) {
        if (auto ptr = db.get_pointer<BaseOperate>(node.base_operate_id)) node.base_operate = std::move(ptr);
        if (auto ptr = db.get_pointer<Combination>(node.combination_id)) node.combination = std::move(ptr);
    }
    return combination_nodes;
}

std::vector<CombinationEdge> allCombinationEdge(const int combination_id) {
    const auto edges = db.get_all<CombinationEdge>(where(c(&CombinationEdge::combination_id) == combination_id));
    const auto nodes = db.get_all<CombinationNode>(where(c(&CombinationNode::combination_id) == combination_id));
    auto combination = db.get_pointer<Combination>(combination_id);
    std::map<int, CombinationNode> node_map;
    for (const auto& node : nodes) {
        node_map[node.node_id] = node;
    }
    for (auto& edge : edges) {
        edge.from_combination_node = std::make_shared<CombinationNode>(node_map.at(edge.from_combination_id));
        edge.next_combination_node = std::make_shared<CombinationNode>(node_map.at(edge.next_combination_id));
        edge.combination = std::move(combination);
    }
    return edges;
}

    std::optional<CombinationGraph> CombinationRepo::getGraphById(const int id) {
    const auto combination = db.get_pointer<Combination>(where(c(&Combination::id) == id));
    if (!combination) {
        return std::nullopt;
    }
    const auto combination_nodes = allCombinationNode(id);
    const auto combination_edges = allCombinationEdge(id);
    return std::make_optional<CombinationGraph>(*combination, combination_nodes, combination_edges);
}

std::optional<CombinationGraph> CombinationRepo::getGraphByName(const std::string &project_name, const std::string &combination_name) {
    const auto combination =
        db.get_pointer<Combination>(where(
            c(&Combination::project_name) == project_name)
            and
            c(&Combination::combination_name) == combination_name
        );
    return getGraphById(combination -> id);
}


CombinationGraph::CombinationGraph(Combination& combination, const std::vector<CombinationNode>& nodes, const std::vector<CombinationEdge>& edges) {
    this->combination = combination;
    for (const auto& node : nodes) {
        this -> node_map[node.node_id] = node;
        if (node.base_operate -> ename == "START_EMPTY") {
            this -> start_node = node;
        }
        if (node.base_operate->ename == "END_EMPTY") {
            this -> end_node = node;
        }
    }
    for (const auto& edge : edges) {
        this -> edge_map[edge.edge_id] = edge;
        if (out_edge.contains(edge.from_combination_id)) {
            out_edge[edge.from_combination_id].push_back(edge);
        } else {
            auto combination_edges = std::vector<CombinationEdge>();
            combination_edges.push_back(edge);
            out_edge[edge.from_combination_id] = combination_edges;
        }
    }
}

const std::optional<Combination>& CombinationGraph::getCombination() const {
    return combination;
}

const std::optional<CombinationNode>& CombinationGraph::getStartNode() const {
    return start_node;
}

const std::optional<CombinationNode>& CombinationGraph::getEndNode() const {
    return end_node;
}

std::vector<const CombinationNode*> CombinationGraph::getCombinationNode() const {
    std::vector<const CombinationNode*> result;
    result.reserve(node_map.size());

    for (const auto &val: node_map | std::views::values) {
        result.push_back(&val); // 只保存指针，不拷贝
    }
    return result;
}

std::vector<const CombinationEdge*> CombinationGraph::getCombinationEdge() const {
    std::vector<const CombinationEdge*> result;
    result.reserve(node_map.size());

    for (const auto &val: edge_map | std::views::values) {
        result.push_back(&val); // 只保存指针，不拷贝
    }
    return result;
}

const CombinationNode& CombinationGraph::getNodeById(const int id) const {
    return node_map.at(id);
}

const CombinationEdge& CombinationGraph::getEdgeById(const int id) const {
    return edge_map.at(id);
}

const std::vector<CombinationEdge>& CombinationGraph::outEdge(const int node_id) const {
    return out_edge.at(node_id);
}

CombinationNode runNode(const CombinationNode &node) {
    for (int i = 0; i < node.loop_cnt; i++) {
        BaseOperationProcess::getInstance().run(*node.base_operate, node.params, false);
        int exec_sleep_time = node.base_operate->min_exec_time;
        if (node.exec_hold_time != 0) {
            exec_sleep_time = node.exec_hold_time;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(exec_sleep_time));

        if (node.auto_reset) {
            BaseOperationProcess::getInstance().run(*node.base_operate, node.params, true);

            int reset_sleep_time = node.base_operate->min_reset_time;
            if (node.reset_hold_time != 0) {
                reset_sleep_time = node.reset_hold_time;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(reset_sleep_time));
        }
    }
    return node;
}

int get_node_finish_time(const CombinationNode &node) {
    const auto high_res_now = std::chrono::high_resolution_clock::now();
    const auto high_res_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        high_res_now.time_since_epoch()
    );

    int exec_time = 0;
    if (node.exec_hold_time != 0) {
        exec_time += node.exec_hold_time;
    } else {
        exec_time += node.base_operate->min_exec_time;
    }
    if (node.auto_reset) {
        if (node.reset_hold_time != 0) {
            exec_time += node.reset_hold_time;
        } else {
            exec_time += node.base_operate->min_reset_time;
        }
    }
    exec_time *= node.loop_cnt;

    return high_res_ms.count() + exec_time;
}

// 比较器：最小堆
auto heap_compare = [](const auto& a, const auto& b) {
    return a.first > b.first;
};

void CombinationGraph::exec() const{
    std::lock_guard lock(execMtx);
    if (!start_node) {
        throw std::runtime_error("拓扑图结构异常，不存在起始节点");
    }

    std::map<int,int> in_cnt_map;
    for (auto edge : getCombinationEdge()) {
        in_cnt_map[edge->next_combination_id]++;
    }

    std::vector<std::pair<int, std::future<CombinationNode>>> heap;
    heap.reserve(getCombinationNode().size());

    auto task = std::bind(runNode, *start_node);
    auto start_node_submit_task = graph_exec_pool.submit_task(task);

    heap.emplace_back(get_node_finish_time(*start_node), std::move(start_node_submit_task));
    std::ranges::push_heap(heap, heap_compare);

    while (!heap.empty()) {
        std::ranges::pop_heap(heap, heap_compare);
        auto cur_future = std::move(heap.back().second);
        heap.pop_back();

        auto cur_node = cur_future.get();

        for (const auto next_edges = outEdge(cur_node.node_id); auto next_edge : next_edges) {
            if (--in_cnt_map[next_edge.next_combination_id] == 0) {
                auto next_node = getNodeById(next_edge.next_combination_id);

                auto next_task = std::bind(runNode, next_node);
                auto next_node_submit_task = graph_exec_pool.submit_task(task);

                heap.emplace_back(get_node_finish_time(next_node), std::move(next_node_submit_task));
                std::ranges::push_heap(heap, heap_compare);
            }
        }
    }

}