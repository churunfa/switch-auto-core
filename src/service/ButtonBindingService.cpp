//
// Created by churunfa on 2026/2/16.
//

#include "ButtonBindingService.h"
#include "repo/base/ButtonBinding.h"
#include "lib/SwitchControlLibrary.h"
#include "controller/ControllerMonitor.h"
#include <SDL3/SDL.h>
#include <iostream>

// 枚举转换函数
namespace {
    // Proto SdlButton -> SDL_GamepadButton
    int sdlButtonToSDLGamepadButton(const base::button::SdlButton proto_btn) {
        switch (proto_btn) {
            case base::button::SDL_BUTTON_SOUTH: return SDL_GAMEPAD_BUTTON_SOUTH;
            case base::button::SDL_BUTTON_EAST: return SDL_GAMEPAD_BUTTON_EAST;
            case base::button::SDL_BUTTON_WEST: return SDL_GAMEPAD_BUTTON_WEST;
            case base::button::SDL_BUTTON_NORTH: return SDL_GAMEPAD_BUTTON_NORTH;
            case base::button::SDL_BUTTON_LEFT_SHOULDER: return SDL_GAMEPAD_BUTTON_LEFT_SHOULDER;
            case base::button::SDL_BUTTON_RIGHT_SHOULDER: return SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER;
            case base::button::SDL_BUTTON_BACK: return SDL_GAMEPAD_BUTTON_BACK;
            case base::button::SDL_BUTTON_START: return SDL_GAMEPAD_BUTTON_START;
            case base::button::SDL_BUTTON_LEFT_STICK: return SDL_GAMEPAD_BUTTON_LEFT_STICK;
            case base::button::SDL_BUTTON_RIGHT_STICK: return SDL_GAMEPAD_BUTTON_RIGHT_STICK;
            case base::button::SDL_BUTTON_GUIDE: return SDL_GAMEPAD_BUTTON_GUIDE;
            case base::button::SDL_BUTTON_MISC1: return SDL_GAMEPAD_BUTTON_MISC1;
            case base::button::SDL_BUTTON_DPAD_UP: return SDL_GAMEPAD_BUTTON_DPAD_UP;
            case base::button::SDL_BUTTON_DPAD_DOWN: return SDL_GAMEPAD_BUTTON_DPAD_DOWN;
            case base::button::SDL_BUTTON_DPAD_LEFT: return SDL_GAMEPAD_BUTTON_DPAD_LEFT;
            case base::button::SDL_BUTTON_DPAD_RIGHT: return SDL_GAMEPAD_BUTTON_DPAD_RIGHT;
            case base::button::SDL_BUTTON_RIGHT_PADDLE1: return SDL_GAMEPAD_BUTTON_RIGHT_PADDLE1;
            case base::button::SDL_BUTTON_RIGHT_PADDLE2: return SDL_GAMEPAD_BUTTON_RIGHT_PADDLE2;
            case base::button::SDL_BUTTON_LEFT_PADDLE1: return SDL_GAMEPAD_BUTTON_LEFT_PADDLE1;
            case base::button::SDL_BUTTON_LEFT_PADDLE2: return SDL_GAMEPAD_BUTTON_LEFT_PADDLE2;
            case base::button::SDL_BUTTON_TOUCHPAD: return SDL_GAMEPAD_BUTTON_TOUCHPAD;
            case base::button::SDL_BUTTON_INVALID: return -1;
            default: return -1;
        }
    }
    
    // SDL_GamepadButton -> Proto SdlButton
    base::button::SdlButton sdlGamepadButtonToSdlButton(const int sdl_btn) {
        switch (sdl_btn) {
            case SDL_GAMEPAD_BUTTON_SOUTH: return base::button::SDL_BUTTON_SOUTH;
            case SDL_GAMEPAD_BUTTON_EAST: return base::button::SDL_BUTTON_EAST;
            case SDL_GAMEPAD_BUTTON_WEST: return base::button::SDL_BUTTON_WEST;
            case SDL_GAMEPAD_BUTTON_NORTH: return base::button::SDL_BUTTON_NORTH;
            case SDL_GAMEPAD_BUTTON_LEFT_SHOULDER: return base::button::SDL_BUTTON_LEFT_SHOULDER;
            case SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER: return base::button::SDL_BUTTON_RIGHT_SHOULDER;
            case SDL_GAMEPAD_BUTTON_BACK: return base::button::SDL_BUTTON_BACK;
            case SDL_GAMEPAD_BUTTON_START: return base::button::SDL_BUTTON_START;
            case SDL_GAMEPAD_BUTTON_LEFT_STICK: return base::button::SDL_BUTTON_LEFT_STICK;
            case SDL_GAMEPAD_BUTTON_RIGHT_STICK: return base::button::SDL_BUTTON_RIGHT_STICK;
            case SDL_GAMEPAD_BUTTON_GUIDE: return base::button::SDL_BUTTON_GUIDE;
            case SDL_GAMEPAD_BUTTON_MISC1: return base::button::SDL_BUTTON_MISC1;
            case SDL_GAMEPAD_BUTTON_DPAD_UP: return base::button::SDL_BUTTON_DPAD_UP;
            case SDL_GAMEPAD_BUTTON_DPAD_DOWN: return base::button::SDL_BUTTON_DPAD_DOWN;
            case SDL_GAMEPAD_BUTTON_DPAD_LEFT: return base::button::SDL_BUTTON_DPAD_LEFT;
            case SDL_GAMEPAD_BUTTON_DPAD_RIGHT: return base::button::SDL_BUTTON_DPAD_RIGHT;
            case SDL_GAMEPAD_BUTTON_RIGHT_PADDLE1: return base::button::SDL_BUTTON_RIGHT_PADDLE1;
            case SDL_GAMEPAD_BUTTON_RIGHT_PADDLE2: return base::button::SDL_BUTTON_RIGHT_PADDLE2;
            case SDL_GAMEPAD_BUTTON_LEFT_PADDLE1: return base::button::SDL_BUTTON_LEFT_PADDLE1;
            case SDL_GAMEPAD_BUTTON_LEFT_PADDLE2: return base::button::SDL_BUTTON_LEFT_PADDLE2;
            case SDL_GAMEPAD_BUTTON_TOUCHPAD: return base::button::SDL_BUTTON_TOUCHPAD;
            default: return base::button::SDL_BUTTON_INVALID;
        }
    }
    
    // Proto SwitchButtonType -> ButtonType
    ButtonType switchButtonTypeToButtonType(base::button::SwitchButtonType proto_btn) {
        switch (proto_btn) {
            case base::button::SWITCH_BUTTON_TYPE_Y: return BUTTON_Y;
            case base::button::SWITCH_BUTTON_TYPE_X: return BUTTON_X;
            case base::button::SWITCH_BUTTON_TYPE_B: return BUTTON_B;
            case base::button::SWITCH_BUTTON_TYPE_A: return BUTTON_A;
            case base::button::SWITCH_BUTTON_TYPE_SR: return BUTTON_SR;
            case base::button::SWITCH_BUTTON_TYPE_SL: return BUTTON_SL;
            case base::button::SWITCH_BUTTON_TYPE_R: return BUTTON_R;
            case base::button::SWITCH_BUTTON_TYPE_ZR: return BUTTON_ZR;
            case base::button::SWITCH_BUTTON_TYPE_MINUS: return BUTTON_MINUS;
            case base::button::SWITCH_BUTTON_TYPE_PLUS: return BUTTON_PLUS;
            case base::button::SWITCH_BUTTON_TYPE_THUMB_R: return BUTTON_THUMB_R;
            case base::button::SWITCH_BUTTON_TYPE_THUMB_L: return BUTTON_THUMB_L;
            case base::button::SWITCH_BUTTON_TYPE_HOME: return BUTTON_HOME;
            case base::button::SWITCH_BUTTON_TYPE_CAPTURE: return BUTTON_CAPTURE;
            case base::button::SWITCH_BUTTON_TYPE_DUMMY: return DUMMY;
            case base::button::SWITCH_BUTTON_TYPE_CHARGING_GRIP: return CHARGING_GRIP;
            case base::button::SWITCH_BUTTON_TYPE_DPAD_DOWN: return DPAD_DOWN;
            case base::button::SWITCH_BUTTON_TYPE_DPAD_UP: return DPAD_UP;
            case base::button::SWITCH_BUTTON_TYPE_DPAD_RIGHT: return DPAD_RIGHT;
            case base::button::SWITCH_BUTTON_TYPE_DPAD_LEFT: return DPAD_LEFT;
            case base::button::SWITCH_BUTTON_TYPE_LEFT_SL: return BUTTON_LEFT_SL;
            case base::button::SWITCH_BUTTON_TYPE_LEFT_SR: return BUTTON_LEFT_SR;
            case base::button::SWITCH_BUTTON_TYPE_L: return BUTTON_L;
            case base::button::SWITCH_BUTTON_TYPE_ZL: return BUTTON_ZL;
            case base::button::SWITCH_BUTTON_TYPE_NONE: return BUTTON_NONE;
            default: return BUTTON_NONE;
        }
    }
    
    // ButtonType -> Proto SwitchButtonType
    base::button::SwitchButtonType buttonTypeToSwitchButtonType(ButtonType btn_type) {
        switch (btn_type) {
            case BUTTON_Y: return base::button::SWITCH_BUTTON_TYPE_Y;
            case BUTTON_X: return base::button::SWITCH_BUTTON_TYPE_X;
            case BUTTON_B: return base::button::SWITCH_BUTTON_TYPE_B;
            case BUTTON_A: return base::button::SWITCH_BUTTON_TYPE_A;
            case BUTTON_SR: return base::button::SWITCH_BUTTON_TYPE_SR;
            case BUTTON_SL: return base::button::SWITCH_BUTTON_TYPE_SL;
            case BUTTON_R: return base::button::SWITCH_BUTTON_TYPE_R;
            case BUTTON_ZR: return base::button::SWITCH_BUTTON_TYPE_ZR;
            case BUTTON_MINUS: return base::button::SWITCH_BUTTON_TYPE_MINUS;
            case BUTTON_PLUS: return base::button::SWITCH_BUTTON_TYPE_PLUS;
            case BUTTON_THUMB_R: return base::button::SWITCH_BUTTON_TYPE_THUMB_R;
            case BUTTON_THUMB_L: return base::button::SWITCH_BUTTON_TYPE_THUMB_L;
            case BUTTON_HOME: return base::button::SWITCH_BUTTON_TYPE_HOME;
            case BUTTON_CAPTURE: return base::button::SWITCH_BUTTON_TYPE_CAPTURE;
            case DUMMY: return base::button::SWITCH_BUTTON_TYPE_DUMMY;
            case CHARGING_GRIP: return base::button::SWITCH_BUTTON_TYPE_CHARGING_GRIP;
            case DPAD_DOWN: return base::button::SWITCH_BUTTON_TYPE_DPAD_DOWN;
            case DPAD_UP: return base::button::SWITCH_BUTTON_TYPE_DPAD_UP;
            case DPAD_RIGHT: return base::button::SWITCH_BUTTON_TYPE_DPAD_RIGHT;
            case DPAD_LEFT: return base::button::SWITCH_BUTTON_TYPE_DPAD_LEFT;
            case BUTTON_LEFT_SL: return base::button::SWITCH_BUTTON_TYPE_LEFT_SL;
            case BUTTON_LEFT_SR: return base::button::SWITCH_BUTTON_TYPE_LEFT_SR;
            case BUTTON_L: return base::button::SWITCH_BUTTON_TYPE_L;
            case BUTTON_ZL: return base::button::SWITCH_BUTTON_TYPE_ZL;
            case BUTTON_NONE: return base::button::SWITCH_BUTTON_TYPE_NONE;
            default: return base::button::SWITCH_BUTTON_TYPE_NONE;
        }
    }
}

namespace service {

    grpc::Status ButtonBindingServiceImpl::GetAllButtonBindings(
        grpc::ServerContext* context,
        const google::protobuf::Empty* request,
        base::button::GetAllButtonBindingsResponse* response) {

        try {
            for (const auto bindings = ButtonBindingRepo::allButtonBinding(); const auto&[id, sdl_btn, sdl_btn_name, button_type, button_name, function_key, graph_id] : bindings) {
                auto* binding_proto = response->add_bindings();
                binding_proto->set_id(id);
                binding_proto->set_sdl_btn(sdlGamepadButtonToSdlButton(sdl_btn));
                binding_proto->set_sdl_btn_name(sdl_btn_name);
                binding_proto->set_button_type(buttonTypeToSwitchButtonType(static_cast<ButtonType>(button_type)));
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

        const bool success = ButtonBindingRepo::updateBinding(
            request->id(),
            sdlButtonToSDLGamepadButton(request->sdl_btn()),
            static_cast<int>(switchButtonTypeToButtonType(request->button_type()))
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

    grpc::Status ButtonBindingServiceImpl::GetConnectedGamepadInfo(
        grpc::ServerContext* context,
        const google::protobuf::Empty* request,
        base::button::GamepadInfoResponse* response) {

        try {
            auto [connected, name, vendorId, productId, serialNumber] = ControllerMonitor::getInstance().getGamepadInfo();

            auto* info_proto = response->mutable_gamepad_info();
            info_proto->set_connected(connected);
            info_proto->set_name(name);
            info_proto->set_vendor_id(vendorId);
            info_proto->set_product_id(productId);
            info_proto->set_serial_number(serialNumber);

            response->set_success(true);
            return grpc::Status::OK;
        } catch (const std::exception& e) {
            std::cerr << "GetConnectedGamepadInfo failed: " << e.what() << std::endl;
            response->set_success(false);
            response->set_error_message(e.what());
            return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
        }
    }

    grpc::Status ButtonBindingServiceImpl::GetAllGamepadsInfo(
        grpc::ServerContext* context,
        const google::protobuf::Empty* request,
        base::button::AllGamepadsResponse* response) {

        try {
            auto gamepads = ControllerMonitor::getInstance().getAllGamepads();

            for (const auto& gamepadInfo : gamepads) {
                auto* info_proto = response->add_gamepads();
                info_proto->set_connected(gamepadInfo.connected);
                info_proto->set_name(gamepadInfo.name);
                info_proto->set_vendor_id(gamepadInfo.vendorId);
                info_proto->set_product_id(gamepadInfo.productId);
                info_proto->set_serial_number(gamepadInfo.serialNumber);
            }

            response->set_success(true);
            return grpc::Status::OK;
        } catch (const std::exception& e) {
            std::cerr << "GetAllGamepadsInfo failed: " << e.what() << std::endl;
            response->set_success(false);
            response->set_error_message(e.what());
            return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
        }
    }

} // namespace service