//
// Created by churunfa on 2026/1/25.
//

#include "CombinationGraph.h"

#include <unistd.h>

#include "repo/DatabaseManager.h"

void CombinationRepo::saveOrUpdateGroup(const CombinationGraph &graph) {
    Combination * const & combination = graph.getCombination();
    const std::vector<const CombinationNode*> combination_nodes = graph.getCombinationNode();
    const std::vector<const CombinationEdge*> combination_edges = graph.getCombinationEdge();

    try {
        db.transaction([&] {
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

            db.insert(*combination);
            for (const auto& combination_node : combination_nodes) {
                db.insert(*combination_node);
            }

            for (const auto& combination_edge : combination_edges) {
                db.insert(*combination_edge);
            }
            return true; // 提交事务
        });
    } catch (const std::exception& e) {
        std::cerr << "Transaction failed: " << e.what() << std::endl;
    }
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
    auto edges = db.get_all<CombinationEdge>(where(c(&CombinationEdge::combination_id) == combination_id));
    for (const auto& edge : edges) {
        if (auto ptr = db.get_pointer<CombinationNode>(edge.from_combination_id)) edge.from_combination_node = std::move(ptr);
        if (auto ptr = db.get_pointer<CombinationNode>(edge.next_combination_id)) edge.next_combination_node = std::move(ptr);
        if (auto ptr = db.get_pointer<Combination>(edge.combination_id)) edge.combination = std::move(ptr);
    }
    return edges;
}

std::optional<CombinationGraph> CombinationRepo::getGraphById(const int id) {
    const auto combination = db.get_pointer<Combination>(id);
    if (!combination) {
        return std::nullopt;
    }
    const auto combination_nodes = allCombinationNode(id);
    const auto combination_edges = allCombinationEdge(id);
    return CombinationGraph(*combination, combination_nodes, combination_edges);
}

std::optional<CombinationGraph> CombinationRepo::getGraphByName(std::string project_name, std::string combination_name) {
    auto combination =
        db.get_pointer<Combination>(where(
            c(&Combination::project_name) == project_name)
            and
            c(&Combination::combination_name) == combination_name
        );
    return getGraphById(combination -> id);
}


CombinationGraph::CombinationGraph(Combination& combination, const std::vector<CombinationNode>& nodes, const std::vector<CombinationEdge>& edges) {
    this->combination = &combination;
    for (const auto& node : nodes) {
        this -> node_map[node.id] = node;
        if (node.base_operate -> ename == "START_EMPTY") {
            this -> start_node = node;
        }
        if (node.base_operate->ename == "END_EMPTY") {
            this -> end_node = node;
        }
    }
    for (const auto& edge : edges) {
        this -> edge_map[edge.id] = edge;
        if (out_edge.contains(edge.from_combination_id)) {
            out_edge[edge.from_combination_id].push_back(edge);
        } else {
            auto combination_edges = std::vector<CombinationEdge>();
            combination_edges.push_back(edge);
            out_edge[edge.from_combination_id] = combination_edges;
        }
    }
}

Combination* CombinationGraph::getCombination() const {
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