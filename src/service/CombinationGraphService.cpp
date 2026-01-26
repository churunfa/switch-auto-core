//
// Created by churunfa on 2026/1/26.
//

#include "CombinationGraphService.h"

#include "mapper/CombinationGraphMapper.h"
#include "repo/combination/CombinationGraph.h"

namespace service {
    grpc::Status CombinationGraphServiceImpl::GetAllProject(grpc::ServerContext *context, const google::protobuf::Empty *request,
                                                                     combination::graph::GetAllProjectResponse *response) {
        const auto all_project = CombinationRepo::allProject();
        for (auto project : all_project) {
            response -> add_project_names(project);
        }
        return grpc::Status::OK;
    }

    grpc::Status CombinationGraphServiceImpl::GetAllGraph(grpc::ServerContext *context, const combination::graph::StringValue *request,
                                                                   combination::graph::GetAllGraphResponse *response) {
        for (const auto combinations = CombinationRepo::allGraph(request -> value()); auto combination : combinations) {
            CombinationGraphMapper::FillCombinationProto(combination, response->add_combinations());
        }
        return grpc::Status::OK;
    }

    grpc::Status CombinationGraphServiceImpl::GetGraphById(grpc::ServerContext *context, const combination::graph::IntValue *request,
                                                                    combination::graph::CombinationGraph *response) {
        const auto combination_graph = CombinationRepo::getGraphById(request -> value());
        if (!combination_graph.has_value()) {
            return grpc::Status::CANCELLED;
        }
        CombinationGraphMapper::FillCombinationProto(*combination_graph->getCombination(), response->mutable_combination());

        // 添加node
        for (const auto combination_nodes = combination_graph->getCombinationNode(); auto node : combination_nodes) {
            CombinationGraphMapper::FillNodeProto(*node, response->add_combination_node());
        }

        // 添加边
        for (const auto combination_edges = combination_graph->getCombinationEdge(); auto combination_edge : combination_edges) {
            CombinationGraphMapper::FillEdgeProto(*combination_edge, response->add_combination_edges());
        }
        return grpc::Status::OK;
    }

    grpc::Status CombinationGraphServiceImpl::InsertGraph(grpc::ServerContext *context, const combination::graph::CombinationGraph *request,
                                                                   base::SimpleResponse *response) {
        auto combination = request->combination();
        auto combination_nodes = request->combination_node();
        auto combination_edges = request->combination_edges();

        response->set_success(true);
        return grpc::Status::OK;
    }

    grpc::Status CombinationGraphServiceImpl::UpdateGraph(grpc::ServerContext *context, const combination::graph::CombinationGraph *request,
                                                                   base::SimpleResponse *response) {
        response->set_success(true);
        return grpc::Status::OK;
    }

    grpc::Status CombinationGraphServiceImpl::DeleteGraph(grpc::ServerContext *context, const combination::graph::IntValue *request,
                                                                   base::SimpleResponse *response) {
        CombinationRepo::deleteGraph(request -> value());
        response->set_success(true);
        return grpc::Status::OK;
    }
}
