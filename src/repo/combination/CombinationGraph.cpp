//
// Created by churunfa on 2026/1/25.
//

#include "CombinationGraph.h"

#include "combination_graph.pb.h"
#include "exec/base/BaseOperationProcess.h"
#include "exec/coroutine/CoroutineUtils.h"
#include "repo/DatabaseManager.h"

namespace asio = boost::asio;

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

std::vector<CombinationNode> copyCombinationNodes(const std::vector<const CombinationNode*>& combination_nodes) {
    std::vector<CombinationNode> combination_nodes_copy;
    combination_nodes_copy.reserve(combination_nodes.size());
for (const auto combination_node : combination_nodes) {
        combination_nodes_copy.emplace_back(CombinationNode(*combination_node));
    }
    return combination_nodes_copy;
}

std::vector<CombinationEdge> copyCombinationEdges(const std::vector<const CombinationEdge*>& combination_edges) {
    std::vector<CombinationEdge> combination_edges_copy;
    combination_edges_copy.reserve(combination_edges.size());
    for (const auto combination_edge : combination_edges) {
        combination_edges_copy.emplace_back(CombinationEdge(*combination_edge));
    }
    return combination_edges_copy;
}


void updateOrSaveGraphCore(const CombinationGraph &graph, const bool insert) {
    const std::optional<const Combination>& combination = graph.getCombination();
    const std::vector<const CombinationNode*> combination_nodes = graph.getCombinationNode();
    const std::vector<const CombinationEdge*> combination_edges = graph.getCombinationEdge();

    auto copy_combination = Combination(*combination);
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

CombinationGraph:: // 深拷贝构造函数
    CombinationGraph(const CombinationGraph& other)
        : combination(other.combination),
          start_node(other.start_node),
          end_node(other.end_node),
          node_map(other.node_map),
          edge_map(other.edge_map),
          out_edge(other.out_edge) {
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

asio::awaitable<void> TopoSession::runNode(const CombinationNode &node) {
    for (int i = 0; i < node.loop_cnt; i++) {
        if (node.exec) {
            BaseOperationProcess::getInstance().run(*node.base_operate, node.params, false);
            if (node.exec_hold_time != 0) {
                int exec_sleep_time = node.exec_hold_time;
                co_await coroutine::async_sleep_task(exec_sleep_time);
            }
        }
        if (node.reset) {
            BaseOperationProcess::getInstance().run(*node.base_operate, node.params, true);

            if (node.reset_hold_time != 0) {
                const int reset_sleep_time = node.reset_hold_time;
                co_await coroutine::async_sleep_task(reset_sleep_time);
            }
        }
    }
}

asio::awaitable<void> TopoSession::execCore(const CombinationGraph &graph) {
    std::unordered_map<int,std::atomic<int>> in_cnt_map;
    for (const auto edge : graph.getCombinationEdge()) {
        ++in_cnt_map[edge->next_combination_id];
    }
    const auto ctx = co_await asio::this_coro::executor;
    asio::co_spawn(ctx, runNode(*graph.getStartNode()), asio::detached);
}

void TopoSession::check_async_running() {
    std::lock_guard lock(async_running_mtx);
    if (async_running) {
        throw std::out_of_range("已存在运行中的任务：" + std::to_string(running_graph_id));
    }
}

void TopoSession::exec(const CombinationGraph &graph) {
    check_async_running();

    if (!graph.getStartNode()) {
        throw std::runtime_error("拓扑图结构异常，不存在起始节点");
    }

    try {
        asio::io_context ctx;
        const auto session = std::make_shared<TopoSession>(ctx, graph);
        session->run();
        ctx.run();
    } catch (std::exception& e) {
        std::cerr << "拓扑图计算异常: " << e.what() << std::endl;
    }
}

void TopoSession::asyncExec(const int graph_id) {
    check_async_running();

    const auto combination_graph = CombinationRepo::getGraphById(graph_id);
    if (!combination_graph) {
        throw std::out_of_range("任务不存在，id：" + std::to_string(graph_id));
    }

    std::lock_guard lock(async_running_mtx);
    async_running = true;

    // 获取当前时间戳
    const auto now = std::chrono::system_clock::now();
    async_start_time = std::chrono::time_point_cast<std::chrono::milliseconds>(now).time_since_epoch().count();

    async_exec_cnt = 0;
    worker = std::thread(&TopoSession::exec, *combination_graph);
}

void TopoSession::stopAsyncExec() {
    std::lock_guard lock(async_running_mtx);
    async_running = false;
    worker.join();
}

asio::awaitable<void> TopoSession::execute_node(int node_id) {

    // 捕获 shared_ptr 保证协程运行期间 session 不被销毁
    const auto self = shared_from_this();

    auto node = graph.getNodeById(node_id);

    std::cout << "[DEBUG] Start execute_node: " << node.node_id
            << ", ename: " << node.base_operate->ename << ",reset:" << node.reset << std::endl;

    try {
        co_await runNode(node);
    } catch (const std::exception& e) {
        std::cerr << "Exception in runNode: " << e.what() << std::endl;
    }

    const auto next_edges = graph.outEdge(node.node_id);
    if (next_edges.empty()) {
        // 执行完了
        std::lock_guard lock(async_running_mtx);

        if (async_running) {
            // 需要异步执行，从头执行
            in_degrees.clear();
            for (const auto edge : graph.getCombinationEdge()) {
                ++in_degrees[edge->next_combination_id];
            }
            asio::co_spawn(ctx, self->execute_node(graph.getStartNode() -> node_id), asio::detached);
            ++async_exec_cnt;
        }
        co_return;
    }

    for (const auto next_edge : next_edges) {
        if (const auto next_node = graph.getNodeById(next_edge.next_combination_id); --in_degrees[next_node.node_id] == 0) {
            asio::co_spawn(ctx, self->execute_node(next_node.node_id), asio::detached);
        }
    }
}

TopoSession::TopoSession(asio::io_context& c, const CombinationGraph& g) : ctx(c), graph(g) {}

void TopoSession::run() {
    std::lock_guard lock(async_running_mtx);
    for (const auto edge : graph.getCombinationEdge()) {
        ++in_degrees[edge->next_combination_id];
    }
    asio::co_spawn(ctx, execute_node(graph.getStartNode()->node_id), asio::detached);
}

void TopoSession::setAsyncExecStatus(combination::graph::GetAsyncExecStatusResponse *response) {
    std::lock_guard lock(async_running_mtx);
    response->set_async_running(async_running);
    response->set_graph_id(running_graph_id);
    response->set_async_start_time(async_start_time);
    response->set_async_exec_cnt(async_exec_cnt);
}