//
// Created by churunfa on 2026/1/26.
//

#include "CombinationGraphService.h"

#include "mapper/CombinationGraphMapper.h"
#include "repo/combination/CombinationGraph.h"

namespace service {
    grpc::Status CombinationGraphServiceImpl::GetAllProject(grpc::ServerContext *context, const google::protobuf::Empty *request,
                                                                     combination::graph::GetAllProjectResponse *response) {
        for (const auto all_project = CombinationRepo::allProject(); const auto& project : all_project) {
            response -> add_project_names(project);
        }
        return grpc::Status::OK;
    }

    grpc::Status CombinationGraphServiceImpl::GetAllGraph(grpc::ServerContext *context, const combination::graph::StringValue *request,
                                                                   combination::graph::GetAllGraphResponse *response) {
        for (const auto combinations = CombinationRepo::allGraph(request -> value()); const auto& combination : combinations) {
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
        if (!combination_graph.has_value()) {
                return {grpc::Status(grpc::StatusCode::NOT_FOUND, "Graph not found")};
            }
        const auto combination = combination_graph->getCombination();
        CombinationGraphMapper::FillCombinationProto(*combination, response->mutable_combination());

        // 添加node
        for (const auto combination_nodes = combination_graph->getCombinationNode(); const auto node : combination_nodes) {
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
        const auto& combination = request->combination();
        const auto& combination_nodes = request->combination_node();
        const auto& combination_edges = request->combination_edges();

        Combination build_combination = CombinationGraphMapper::buildCombination(combination);
        auto build_nodes = std::vector<CombinationNode>();
        build_nodes.reserve(combination_nodes.size());
        for (int i = 0; i < combination_nodes.size(); i++) {
            build_nodes.push_back(CombinationGraphMapper::buildNode(build_combination, combination_nodes.Get(i)));
        }

        auto build_edges = std::vector<CombinationEdge>();
        build_edges.reserve(combination_edges.size());
        for (int i = 0; i < combination_edges.size(); i++) {
            build_edges.push_back(CombinationGraphMapper::buildEdge(build_combination, combination_edges.Get(i)));
        }

        const auto build_graph = CombinationGraph(build_combination, build_nodes, build_edges);
        CombinationRepo::insertGraph(build_graph);

        response->set_success(true);
        return grpc::Status::OK;
    }

    grpc::Status CombinationGraphServiceImpl::UpdateGraph(grpc::ServerContext *context, const combination::graph::CombinationGraph *request,
                                                                   base::SimpleResponse *response) {
        const auto build_graph = CombinationGraphMapper::buildGraph(request);
        CombinationRepo::updateGraph(build_graph);
        response->set_success(true);
        return grpc::Status::OK;
    }

    grpc::Status CombinationGraphServiceImpl::DeleteGraph(grpc::ServerContext *context, const combination::graph::IntValue *request,
                                                                   base::SimpleResponse *response) {
        CombinationRepo::deleteGraph(request -> value());
        response->set_success(true);
        return grpc::Status::OK;
    }

    grpc::Status CombinationGraphServiceImpl::ExecGraph(grpc::ServerContext *context, const combination::graph::CombinationGraph *request,
       base::SimpleResponse *response) {
        const auto build_graph = CombinationGraphMapper::buildGraph(request);
        build_graph.exec();
        return grpc::Status::OK;
    }

    grpc::Status CombinationGraphServiceImpl::ExecGraphById(grpc::ServerContext *context, const combination::graph::IntValue *request,
        base::SimpleResponse *response) {
        const auto combination_graph = CombinationRepo::getGraphById(request -> value());
        combination_graph->exec();
        return grpc::Status::OK;
    }
}
