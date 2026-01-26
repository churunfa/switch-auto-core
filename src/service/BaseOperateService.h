//
// Created by churunfa on 2026/1/25.
//

#ifndef SWITCH_AUTO_CORE_BASEOPERATESERVICE_H
#define SWITCH_AUTO_CORE_BASEOPERATESERVICE_H
#include "base_operate.grpc.pb.h"

namespace service {
    class BaseOperateServiceImpl final : public base::operate::BaseOperateService::Service {
    public:
        BaseOperateServiceImpl() = default;

        // 只保留方法声明
        grpc::Status GetAllBaseOperates(
            grpc::ServerContext* context,
            const google::protobuf::Empty* request,
            base::operate::GetAllBaseOperatesResponse* response) override;

        grpc::Status ExecBaseOperate(
            grpc::ServerContext* context,
            const base::operate::ExecBaseOperateRequest* request,
            base::SimpleResponse* response) override;
    };

} // namespace service

#endif //SWITCH_AUTO_CORE_BASEOPERATESERVICE_H