//
// Created by churunfa on 2026/1/26.
//

#ifndef SWITCH_AUTO_CORE_COMBINATIONGRAPHSERVICE_H
#define SWITCH_AUTO_CORE_COMBINATIONGRAPHSERVICE_H
#include "combination_graph.grpc.pb.h"


namespace service {
    class CombinationGraphServiceImpl final : public combination::graph::CombinationGraphService::Service {
    public:
        CombinationGraphServiceImpl() = default;

        grpc::Status GetAllProject(grpc::ServerContext *context, const google::protobuf::Empty *request,
            combination::graph::GetAllProjectResponse *response) override;

        grpc::Status GetAllGraph(grpc::ServerContext *context, const combination::graph::StringValue *request,
            combination::graph::GetAllGraphResponse *response) override;

        grpc::Status GetGraphById(grpc::ServerContext *context, const combination::graph::IntValue *request,
            combination::graph::CombinationGraph *response) override;

        grpc::Status InsertGraph(grpc::ServerContext *context, const combination::graph::CombinationGraph *request,
            base::SimpleResponse *response) override;

        grpc::Status UpdateGraph(grpc::ServerContext *context, const combination::graph::CombinationGraph *request,
            base::SimpleResponse *response) override;

        grpc::Status DeleteGraph(grpc::ServerContext *context, const combination::graph::IntValue *request,
            base::SimpleResponse *response) override;

        grpc::Status ExecGraph(grpc::ServerContext *context, const combination::graph::CombinationGraph *request,
        base::SimpleResponse *response) override;

        grpc::Status ExecGraphById(grpc::ServerContext *context, const combination::graph::IntValue *request,
        base::SimpleResponse *response) override;
    };

} // namespace service


#endif //SWITCH_AUTO_CORE_COMBINATIONGRAPHSERVICE_H