//
// Created by churunfa on 2026/1/25.
//

#ifndef SWITCH_AUTO_CORE_COMBINATIONGRAPH_H
#define SWITCH_AUTO_CORE_COMBINATIONGRAPH_H
#include <thread>
#include <boost/asio/awaitable.hpp>
#include "Combination.h"
#include "combination_graph.pb.h"

namespace asio = boost::asio;

struct CombinationNode;
struct CombinationEdge;


class CombinationGraph {
    std::optional<Combination> combination = std::nullopt;
    std::optional<CombinationNode> start_node = std::nullopt;

    std::map<int, CombinationNode> node_map;
    std::map<int, CombinationEdge> edge_map;

    std::map<int, std::vector<CombinationEdge>> out_edge;
public:
    CombinationGraph(Combination& combination, const std::vector<CombinationNode>& nodes, const std::vector<CombinationEdge>& edges);
    CombinationGraph(const CombinationGraph& combination);
    const std::optional<Combination>& getCombination() const;
    const std::optional<CombinationNode>& getStartNode() const;
    std::vector<const CombinationNode*> getCombinationNode() const;
    std::vector<const CombinationEdge*> getCombinationEdge() const;
    const CombinationNode& getNodeById(int id) const;
    const CombinationEdge& getEdgeById(int id) const;
    const std::vector<CombinationEdge>& outEdge(int node_id) const;
};

class CombinationRepo {
public:
    static void insertGraph(const CombinationGraph &graph);
    static void updateGraph(const CombinationGraph &graph);
    static void deleteGraph(int combination_id);
    static std::vector<std::string> allProject();
    static std::vector<Combination> allGraph(const std::string &project_name);
    static std::optional<CombinationGraph> getGraphById(int id);
    static std::optional<CombinationGraph> getGraphByName(const std::string &project_name, const std::string &combination_name);
};

class TopoSession : public std::enable_shared_from_this<TopoSession> {
    asio::io_context& ctx;
    const CombinationGraph& graph;
    std::unordered_map<int, int> in_degrees;

    inline static std::recursive_mutex async_running_mtx;
    inline static std::thread worker;
    inline static std::atomic<bool> async_running{false};
    inline static std::atomic<int> running_graph_id{0};
    inline static std::atomic<long long> async_start_time{0};
    inline static std::atomic<long long> async_exec_cnt{0};

    asio::awaitable<void> execute_node(int node_id);
    void run();
    static asio::awaitable<void> execCore(const CombinationGraph &graph);
    static asio::awaitable<void> runNode(CombinationNode node);
    static void check_async_running();
public:
    TopoSession(asio::io_context& c, const CombinationGraph& g);
    static void exec(const CombinationGraph &graph, bool async=false);
    static void asyncExec(int graph_id);
    static void stopAsyncExec();
    static void setAsyncExecStatus(combination::graph::GetAsyncExecStatusResponse *response);
};

#endif //SWITCH_AUTO_CORE_COMBINATIONGRAPH_H