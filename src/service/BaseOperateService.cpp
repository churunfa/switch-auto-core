//
// Created by churunfa on 2026/1/25.
//

#include "BaseOperateService.h"

#include "exec/base/BaseOperationProcess.h"
#include "mapper/BaseOperateMapper.h"
#include "repo/base/BaseOperate.h"

namespace service {
    grpc::Status BaseOperateServiceImpl::GetAllBaseOperates(
        grpc::ServerContext* context,
        const google::protobuf::Empty* request,
        base::operate::GetAllBaseOperatesResponse* response) {
        // 2. 遍历并转换
        for (const auto base_operates = BaseOperateRepo::findAll(); const auto& item : base_operates) {
            BaseOperateMapper::FillBaseOperateProto(item, response->add_operates());
        }

        return grpc::Status::OK;
    }

    grpc::Status BaseOperateServiceImpl::ExecBaseOperate(
        grpc::ServerContext* context,
        const base::operate::ExecBaseOperateRequest* request,
        base::SimpleResponse* response) {
        auto entity_opt = BaseOperateRepo::findOneById(request -> id());
        if (!entity_opt.has_value()) {
            return {grpc::StatusCode::NOT_FOUND, "BaseOperate ID not found"};
        }

        const BaseOperate& entity = entity_opt.value();
        std::string params = request->params();
        const bool should_reset = request->reset();

        try {
            BaseOperationProcess::getInstance().run(entity, nlohmann::json::parse(params), should_reset);
            response->set_success(true);
            return grpc::Status::OK;

        } catch (const std::exception& e) {
            response->set_success(false);
            return {grpc::Status(grpc::StatusCode::INTERNAL, e.what())};
        }
    }

} // namespace service