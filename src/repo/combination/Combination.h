//
// Created by churunfa on 2026/1/25.
//

#ifndef SWITCH_AUTO_CORE_COMBINATION_H
#define SWITCH_AUTO_CORE_COMBINATION_H

#include <string>
#include <sqlite_orm/sqlite_orm.h>

#include "repo/base/BaseOperate.h"

using namespace sqlite_orm;

struct Combination {
    int id;
    std::string project_name;
    std::string combination_name;
    std::string desc;
    int min_time;

    Combination() = default;
    Combination(const Combination& other):id(other.id),project_name(other.project_name), combination_name(other.combination_name),desc(other.desc),min_time(other.min_time) {

    }

    static auto getDescription() {
        return make_table("combination",
            make_column("id", &Combination::id, primary_key()),
            make_column("project_name", &Combination::project_name),
            make_column("combination_name", &Combination::combination_name),
            make_column("desc", &Combination::desc),
            make_column("min_time", &Combination::min_time),

            sqlite_orm::unique(&Combination::project_name, &Combination::combination_name)
        );
    }
};

struct CombinationNode {
    int id;
    int node_id;
    std::string node_name;
    int combination_id;
    mutable std::shared_ptr<Combination> combination;
    std::string base_operate_ids;
    mutable std::vector<BaseOperate> base_operates;
    std::string params;
    std::vector<std::vector<int>> parse_params;
    std::string resets;
    std::vector<bool> parse_resets;
    std::string auto_resets;
    std::vector<bool> parse_auto_resets;
    int exec_hold_time;
    int loop_cnt;
    CombinationNode() = default;
    CombinationNode(const CombinationNode& other) : id(other.id), node_id(other.node_id),node_name(other.node_name),combination_id(other.combination_id),
                                                    combination(other.combination),base_operate_ids(other.base_operate_ids),
                                                    base_operates(other.base_operates), params(other.params),
                                                    parse_params(other.parse_params),resets(other.resets),
                                                    parse_resets(other.parse_resets), auto_resets(other.auto_resets), parse_auto_resets(other.parse_auto_resets), exec_hold_time(other.exec_hold_time), loop_cnt(other.loop_cnt) {
    }

    static auto getDescription() {
        return make_table("combination_node",
            make_column("id", &CombinationNode::id, primary_key()),
            make_column("node_id", &CombinationNode::node_id),
            make_column("node_name", &CombinationNode::node_name),
            make_column("combination_id", &CombinationNode::combination_id),
            make_column("base_operate_ids", &CombinationNode::base_operate_ids),
            make_column("params", &CombinationNode::params),
            make_column("exec_hold_time", &CombinationNode::exec_hold_time),
            make_column("loop_cnt", &CombinationNode::loop_cnt),
            make_column("resets", &CombinationNode::resets),
            make_column("auto_resets", &CombinationNode::auto_resets),

            unique(&CombinationNode::combination_id, &CombinationNode::node_id)
        );
    }
};

struct CombinationEdge {
    int id;
    int edge_id;
    std::string edge_name;
    int combination_id;
    mutable std::shared_ptr<Combination> combination;
    int from_combination_id;
    mutable std::shared_ptr<CombinationNode> from_combination_node;
    int next_combination_id;
    mutable std::shared_ptr<CombinationNode> next_combination_node;

    CombinationEdge() = default;
    CombinationEdge(const CombinationEdge& other) : id(other.id), edge_id(other.edge_id), edge_name(other.edge_name),
                                                    combination_id(other.combination_id), combination(other.combination),
                                                    from_combination_id(other.from_combination_id),from_combination_node(other.from_combination_node),
                                                    next_combination_id(other.next_combination_id),
                                                    next_combination_node(other.next_combination_node) {
    }

    static auto getDescription() {
        return make_table("combination_edge",
            make_column("id", &CombinationEdge::id, primary_key()),
            make_column("edge_id", &CombinationEdge::edge_id),
            make_column("edge_name", &CombinationEdge::edge_name),
            make_column("combination_id", &CombinationEdge::combination_id),
            make_column("from_combination_id", &CombinationEdge::from_combination_id),
            make_column("next_combination_id", &CombinationEdge::next_combination_id),

            unique(&CombinationEdge::combination_id, &CombinationEdge::edge_id)
        );
    }
};

#endif //SWITCH_AUTO_CORE_COMBINATION_H