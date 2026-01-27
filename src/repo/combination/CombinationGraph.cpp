//
// Created by churunfa on 2026/1/25.
//

#include "CombinationGraph.h"

#include <unistd.h>

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
    return Combination(*combination);
}

std::vector<CombinationNode> copyCombinationNodes(const std::vector<const CombinationNode*> combination_nodes) {
    std::vector<CombinationNode> combination_nodes_copy;
    combination_nodes_copy.reserve(combination_nodes.size());
for (const auto combination_node : combination_nodes) {
        combination_nodes_copy.emplace_back(*combination_node);
    }
    return combination_nodes_copy;
}

std::vector<CombinationEdge> copyCombinationEdges(const std::vector<const CombinationEdge*> combination_edges) {
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
    const auto combination_nodes = allCombinationNode(id);
    const auto combination_edges = allCombinationEdge(id);
    return CombinationGraph(*combination, combination_nodes, combination_edges);
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