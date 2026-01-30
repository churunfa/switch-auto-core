//
// Created by churunfa on 2026/1/26.
//


#include "CombinationGraphMapper.h"

#include "BaseOperateMapper.h"
#include "combination_graph.pb.h"
#include "repo/combination/Combination.h"

void CombinationGraphMapper::FillCombinationProto(const Combination& src, combination::graph::Combination* dest) {
    dest->set_id(src.id);
    dest->set_project_name(src.project_name);
    dest->set_combination_name(src.combination_name);
    dest->set_desc(src.desc);
    dest->set_min_time(src.min_time);
}
void CombinationGraphMapper::FillEdgeProto(const CombinationEdge& src, combination::graph::CombinationEdge* dest) {
    dest->set_edge_id(src.edge_id);
    dest->set_edge_name(src.edge_name);
    dest->set_from_node_id(src.from_combination_id);
    dest->set_next_node_id(src.next_combination_id);
}

void CombinationGraphMapper::FillNodeProto(const CombinationNode& src, combination::graph::CombinationNode* dest) {
    dest->set_node_id(src.node_id);
    dest->set_node_name(src.node_name);
    BaseOperateMapper::FillBaseOperateProto(*src.base_operate, dest->mutable_base_operate());
    dest->set_params(src.params);
    dest->set_exec_hold_time(src.exec_hold_time);
    dest->set_reset_hold_time(src.reset_hold_time);
    dest->set_loop_cnt(src.loop_cnt);
    dest->set_exec(src.exec);
    dest->set_reset(src.reset);
}

CombinationGraph CombinationGraphMapper::buildGraph(const combination::graph::CombinationGraph *graph) {
    const auto& combination = graph->combination();
    const auto& combination_nodes = graph->combination_node();
    const auto& combination_edges = graph->combination_edges();

    Combination build_combination = buildCombination(combination);
    auto build_nodes = std::vector<CombinationNode>();
    build_nodes.reserve(combination_nodes.size());
    for (int i = 0; i < combination_nodes.size(); i++) {
        build_nodes.push_back(buildNode(build_combination, combination_nodes.Get(i)));
    }

    auto build_edges = std::vector<CombinationEdge>();
    build_edges.reserve(combination_edges.size());
    for (int i = 0; i < combination_edges.size(); i++) {
        build_edges.push_back(buildEdge(build_combination, combination_edges.Get(i)));
    }

    return CombinationGraph(build_combination, build_nodes, build_edges);
}

Combination CombinationGraphMapper::buildCombination(const combination::graph::Combination& combination) {
    Combination res;
    res.id = combination.id();
    res.project_name = combination.project_name();
    res.combination_name = combination.combination_name();
    res.desc = combination.desc();
    res.min_time = combination.min_time();
    return res;
}

CombinationNode CombinationGraphMapper::buildNode(const Combination& combination, const combination::graph::CombinationNode& node) {
    CombinationNode res;
    res.id = 0;
    res.node_id = node.node_id();
    res.node_name = node.node_name();
    res.combination_id = combination.id;
    res.combination = std::make_shared<Combination>(combination);
    res.base_operate_id = node.base_operate().id();
    res.base_operate = std::make_shared<BaseOperate>(BaseOperateMapper::buildBaseOperate(node.base_operate()));
    res.params = node.params();
    res.exec_hold_time = node.exec_hold_time();
    res.reset_hold_time = node.reset_hold_time();
    res.loop_cnt = node.loop_cnt();
    res.exec = node.exec();
    res.reset = node.reset();
    return res;
}

CombinationEdge CombinationGraphMapper::buildEdge(const Combination& combination, const combination::graph::CombinationEdge& edge) {
    CombinationEdge res;
    res.id = 0;
    res.edge_id = edge.edge_id();
    res.edge_name = edge.edge_name();
    res.combination_id = combination.id;
    res.combination = std::make_shared<Combination>(combination);
    res.from_combination_id = edge.from_node_id();
    res.next_combination_id = edge.next_node_id();
    return res;
}

CombinationGraph CombinationGraphMapper::buildGraph(const combination::graph::Combination& combination,
                           const google::protobuf::RepeatedPtrField<combination::graph::CombinationNode>& nodes,
                           const google::protobuf::RepeatedPtrField<combination::graph::CombinationEdge>& edges) {

    auto build_combination = buildCombination(combination);

    auto build_nodes = std::vector<CombinationNode>();
    build_nodes.reserve(nodes.size());
    for (int i = 0; i < nodes.size(); ++i) {
        build_nodes.push_back(buildNode(build_combination, nodes.Get(i)));
    }

    auto build_edges = std::vector<CombinationEdge>();
    for (int i = 0; i < edges.size(); ++i) {
        build_edges.push_back(buildEdge(build_combination, edges.Get(i)));
    }
    return CombinationGraph(build_combination, build_nodes, build_edges);
}