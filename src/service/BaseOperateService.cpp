//
// Created by churunfa on 2026/1/25.
//

#include "BaseOperateService.h"

#include "exec/base/BaseOperationProcess.h"
#include "repo/base/BaseOperate.h"

namespace service {
    namespace {
        /**
         * @brief 将数据库实体结构体转换为 Proto 消息对象
         * @param src  源数据（C++ Struct）
         * @param dest 目标对象（Proto Message Pointer）
         */
        void FillBaseOperateProto(const BaseOperate& src, base::operate::BaseOperate* dest) {
            dest->set_id(src.id);
            dest->set_ename(src.ename);
            dest->set_name(src.name);
            dest->set_param_size(src.param_size);
            dest->set_param_names(src.param_names);
            dest->set_min_exec_time(src.min_exec_time);
            dest->set_min_reset_time(src.min_reset_time);
        }
    }


    grpc::Status BaseOperateServiceImpl::GetAllBaseOperates(
        grpc::ServerContext* context,
        const google::protobuf::Empty* request,
        base::operate::GetAllBaseOperatesResponse* response) {
        // 2. 遍历并转换
        for (const auto base_operates = BaseOperateRepo::findAll(); const auto& item : base_operates) {
            FillBaseOperateProto(item, response->add_operates());
        }

        return grpc::Status::OK;
    }

    grpc::Status BaseOperateServiceImpl::ExecBaseOperate(
        grpc::ServerContext* context,
        const base::operate::ExecBaseOperateRequest* request,
        base::operate::ExecBaseOperateResponse* response) {
        auto entity_opt = BaseOperateRepo::findOneById(request -> id());
        if (!entity_opt.has_value()) {
            return {grpc::StatusCode::NOT_FOUND, "BaseOperate ID not found"};
        }

        BaseOperate& entity = entity_opt.value();
        std::string params = request->params();
        const bool should_reset = request->reset();

        try {
            BaseOperationProcess::getInstance().run(entity, params, should_reset);
            response->set_success(true);
            return grpc::Status::OK;

        } catch (const std::exception& e) {
            response->set_success(false);
            return {grpc::Status(grpc::StatusCode::INTERNAL, e.what())};
        }
    }

} // namespace service