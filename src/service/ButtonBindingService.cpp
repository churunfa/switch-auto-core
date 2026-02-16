//
// Created by churunfa on 2026/2/16.
//

#include "ButtonBindingService.h"
#include "repo/base/ButtonBinding.h"
#include <iostream>

namespace service {

    grpc::Status ButtonBindingServiceImpl::GetAllButtonBindings(
        grpc::ServerContext* context,
        const google::protobuf::Empty* request,
        base::button::GetAllButtonBindingsResponse* response) {

        try {
            for (const auto bindings = ButtonBindingRepo::allButtonBinding(); const auto&[id, sdl_btn, sdl_btn_name, button_type, button_name, function_key, graph_id] : bindings) {
                auto* binding_proto = response->add_bindings();
                binding_proto->set_id(id);
                binding_proto->set_sdl_btn(sdl_btn);
                binding_proto->set_sdl_btn_name(sdl_btn_name);
                binding_proto->set_button_type(button_type);
                binding_proto->set_button_name(button_name);
                binding_proto->set_function_key(function_key);
                binding_proto->set_graph_id(graph_id);
            }
            return grpc::Status::OK;
        } catch (const std::exception& e) {
            std::cerr << "GetAllButtonBindings failed: " << e.what() << std::endl;
            return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
        }
    }

    grpc::Status ButtonBindingServiceImpl::UpdateButtonBinding(
        grpc::ServerContext* context,
        const base::button::UpdateButtonBindingRequest* request,
        base::SimpleResponse* response) {

        bool success = ButtonBindingRepo::updateBinding(
            request->id(),
            request->sdl_btn(),
            request->button_type()
        );

        response->set_success(success);
        if (!success) {
            return grpc::Status(grpc::StatusCode::NOT_FOUND, "Button binding not found or update failed");
        }
        return grpc::Status::OK;
    }

    grpc::Status ButtonBindingServiceImpl::SetFunctionKey(
        grpc::ServerContext* context,
        const base::button::SetFunctionKeyRequest* request,
        base::SimpleResponse* response) {

        bool success = ButtonBindingRepo::setFunctionKey(
            request->id(),
            request->function_key()
        );

        response->set_success(success);
        if (!success) {
            return grpc::Status(grpc::StatusCode::NOT_FOUND, "Button binding not found or update failed");
        }
        return grpc::Status::OK;
    }

    grpc::Status ButtonBindingServiceImpl::BindGraphToButton(
        grpc::ServerContext* context,
        const base::button::BindGraphToButtonRequest* request,
        base::SimpleResponse* response) {

        bool success = ButtonBindingRepo::bindGraph(
            request->id(),
            request->graph_id()
        );

        response->set_success(success);
        if (!success) {
            return grpc::Status(grpc::StatusCode::NOT_FOUND, "Button binding not found or bind failed");
        }
        return grpc::Status::OK;
    }

    grpc::Status ButtonBindingServiceImpl::UnbindGraphFromButton(
        grpc::ServerContext* context,
        const base::button::UnbindGraphFromButtonRequest* request,
        base::SimpleResponse* response) {

        bool success = ButtonBindingRepo::unbindGraph(request->id());

        response->set_success(success);
        if (!success) {
            return grpc::Status(grpc::StatusCode::NOT_FOUND, "Button binding not found or unbind failed");
        }
        return grpc::Status::OK;
    }

} // namespace service