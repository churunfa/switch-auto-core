//
// Created by churunfa on 2026/2/16.
//

#ifndef SWITCH_AUTO_CORE_BUTTONBINDINGSERVICE_H
#define SWITCH_AUTO_CORE_BUTTONBINDINGSERVICE_H

#include "button_binding.grpc.pb.h"

namespace service {
    class ButtonBindingServiceImpl final : public base::button::ButtonBindingService::Service {
    public:
        ButtonBindingServiceImpl() = default;

        grpc::Status GetAllButtonBindings(
            grpc::ServerContext* context,
            const google::protobuf::Empty* request,
            base::button::GetAllButtonBindingsResponse* response) override;

        grpc::Status UpdateButtonBinding(
            grpc::ServerContext* context,
            const base::button::UpdateButtonBindingRequest* request,
            base::SimpleResponse* response) override;

        grpc::Status SetFunctionKey(
            grpc::ServerContext* context,
            const base::button::SetFunctionKeyRequest* request,
            base::SimpleResponse* response) override;

        grpc::Status BindGraphToButton(
            grpc::ServerContext* context,
            const base::button::BindGraphToButtonRequest* request,
            base::SimpleResponse* response) override;

        grpc::Status UnbindGraphFromButton(
            grpc::ServerContext* context,
            const base::button::UnbindGraphFromButtonRequest* request,
            base::SimpleResponse* response) override;
    };

} // namespace service

#endif //SWITCH_AUTO_CORE_BUTTONBINDINGSERVICE_H