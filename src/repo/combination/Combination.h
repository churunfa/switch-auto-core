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
    int combination_id;
    mutable std::shared_ptr<Combination> combination;
    int base_operate_id;
    mutable std::shared_ptr<BaseOperate> base_operate;
    std::string params;
    int hold_time;
    int loop_cnt;
    bool auto_reset;

    static auto getDescription() {
        return make_table("combination_node",
            make_column("id", &CombinationNode::id, primary_key()),
            make_column("node_id", &CombinationNode::node_id),
            make_column("combination_id", &CombinationNode::combination_id),
            make_column("base_operate_id", &CombinationNode::base_operate_id),
            make_column("params", &CombinationNode::params),
            make_column("hold_time", &CombinationNode::hold_time),
            make_column("loop_cnt", &CombinationNode::loop_cnt),
            make_column("auto_reset", &CombinationNode::auto_reset, default_value(false)),

            unique(&CombinationNode::combination_id, &CombinationNode::node_id)
        );
    }
};

struct CombinationEdge {
    int id;
    int edge_id;
    int combination_id;
    mutable std::shared_ptr<Combination> combination;
    int from_combination_id;
    mutable std::shared_ptr<CombinationNode> from_combination_node;
    int next_combination_id;
    mutable std::shared_ptr<CombinationNode> next_combination_node;

    static auto getDescription() {
        return make_table("combination_edge",
            make_column("id", &CombinationEdge::id, primary_key()),
            make_column("edge_id", &CombinationEdge::edge_id),
            make_column("combination_id", &CombinationEdge::combination_id),
            make_column("from_combination_id", &CombinationEdge::from_combination_id),
            make_column("next_combination_id", &CombinationEdge::next_combination_id),

            unique(&CombinationEdge::combination_id, &CombinationEdge::edge_id)
        );
    }
};

#endif //SWITCH_AUTO_CORE_COMBINATION_H