//
// Created by churunfa on 2026/1/25.
//

#ifndef SWITCH_AUTO_CORE_COMBINATIONGRAPH_H
#define SWITCH_AUTO_CORE_COMBINATIONGRAPH_H
#include <boost/asio/awaitable.hpp>
#include "Combination.h"

namespace asio = boost::asio;

struct CombinationNode;
struct CombinationEdge;


class CombinationGraph {
    std::optional<Combination> combination = std::nullopt;
    std::optional<CombinationNode> start_node = std::nullopt;
    std::optional<CombinationNode> end_node = std::nullopt;

    std::map<int, CombinationNode> node_map;
    std::map<int, CombinationEdge> edge_map;

    std::map<int, std::vector<CombinationEdge>> out_edge;

    asio::awaitable<void> execCore() const;
public:
    CombinationGraph(Combination& combination, const std::vector<CombinationNode>& nodes, const std::vector<CombinationEdge>& edges);
    const std::optional<Combination>& getCombination() const;
    const std::optional<CombinationNode>& getStartNode() const;
    const std::optional<CombinationNode>& getEndNode() const;
    std::vector<const CombinationNode*> getCombinationNode() const;
    std::vector<const CombinationEdge*> getCombinationEdge() const;
    const CombinationNode& getNodeById(int id) const;
    const CombinationEdge& getEdgeById(int id) const;
    const std::vector<CombinationEdge>& outEdge(int node_id) const;
    void exec() const;
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

class TopoSession : std::enable_shared_from_this<TopoSession> {
    asio::io_context& ctx;
    const CombinationGraph* graph;
    std::unordered_map<int, int> in_degrees;

    asio::awaitable<void> execute_node(const CombinationNode& node);
public:
    TopoSession(asio::io_context& c, const CombinationGraph* g);
    void run();
};

#endif //SWITCH_AUTO_CORE_COMBINATIONGRAPH_H