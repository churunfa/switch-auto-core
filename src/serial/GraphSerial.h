//
// Created by churunfa on 2026/2/18.
//

#ifndef SWITCH_AUTO_CORE_GRAPHSERIAL_H
#define SWITCH_AUTO_CORE_GRAPHSERIAL_H
#include <vector>
#include "glaze/glaze.hpp"
#include "repo/combination/CombinationGraph.h"

struct GraphNode {
    int node_id{};
    std::vector<std::string> base_operates;
    std::vector<std::vector<int>> params;
    std::vector<bool> resets;
    std::vector<bool> auto_resets;
    int exec_hold_time{};
    int loop_cnt{};
};

template <>
struct glz::meta<GraphNode> {
    static constexpr auto value = object(
       "node_id", &GraphNode::node_id,
       "base_operates", &GraphNode::base_operates,
       "params", &GraphNode::params,
       "resets", &GraphNode::resets,
       "auto_resets", &GraphNode::auto_resets,
       "exec_hold_time", &GraphNode::exec_hold_time,
       "loop_cnt", &GraphNode::loop_cnt
    );
};

struct GraphEdge {
    int edge_id{};
    int from_node_id{};
    int next_node_id{};
};

template <>
struct glz::meta<GraphEdge> {
    static constexpr auto value = object(
       "edge_id", &GraphEdge::edge_id,
       "from_node_id", &GraphEdge::from_node_id,
       "next_node_id", &GraphEdge::next_node_id
    );
};

struct Graph {
    int id{};
    std::vector<GraphNode> graph_nodes;
    std::vector<GraphEdge> graph_edges;
};

template <>
struct glz::meta<Graph> {
    static constexpr auto value = object(
       "id", &Graph::id,
       "graph_nodes", &Graph::graph_nodes,
       "graph_edges", &Graph::graph_edges
    );
};

class GraphSerial {
    static Graph buildGraph(const CombinationGraph &combination_graph) {
        Graph graph;
        graph.id = combination_graph.combination->id;
            
        // 转换节点
        for (const auto &combination_node: combination_graph.node_map | std::views::values) {
            GraphNode graph_node;
            graph_node.node_id = combination_node.node_id;
            graph_node.exec_hold_time = combination_node.exec_hold_time;
            graph_node.loop_cnt = combination_node.loop_cnt;
                
            // 转换基础操作名称
            for (const auto& base_operate : combination_node.base_operates) {
                graph_node.base_operates.push_back(base_operate.ename);
            }
                
            // 复制参数、重置和自动重置信息
            graph_node.params = combination_node.parse_params;
            graph_node.resets = combination_node.parse_resets;
            graph_node.auto_resets = combination_node.parse_auto_resets;
                
            graph.graph_nodes.push_back(graph_node);
        }
            
        // 转换边
        for (const auto &combination_edge: combination_graph.edge_map | std::views::values) {
            GraphEdge graph_edge;
            graph_edge.edge_id = combination_edge.edge_id;
            graph_edge.from_node_id = combination_edge.from_combination_node->node_id;
            graph_edge.next_node_id = combination_edge.next_combination_node->node_id;
                
            graph.graph_edges.push_back(graph_edge);
        }
            
        return graph;
    }
};

#endif //SWITCH_AUTO_CORE_GRAPHSERIAL_H