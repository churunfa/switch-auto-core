//
// Created by churunfa on 2026/2/16.
//

#ifndef SWITCH_AUTO_CORE_BUTTONBINDINGCACHE_H
#define SWITCH_AUTO_CORE_BUTTONBINDINGCACHE_H
#include <map>
#include <vector>
#include <SDL3/SDL_gamepad.h>

#include "CacheServiceInterface.h"
#include "SwitchControlLibrary.h"
#include "repo/base/ButtonBinding.h"

struct ButtonBinding;

class ButtonBindingCache : public CacheService {
    std::map<SDL_GamepadButton, ButtonType> sdl_button_map_;
    std::map<ButtonType, int> button_graph_map_;
    ButtonType function_button_ = BUTTON_NONE;
    ButtonBindingCache() = default;
public:
    void load() override {
        for (const auto button_bindings = ButtonBindingRepo::allButtonBinding(); const auto& button_binding : button_bindings) {
            const auto sdl_btn_enum = static_cast<SDL_GamepadButton>(button_binding.sdl_btn);
            const auto button_type_enum = static_cast<ButtonType>(button_binding.button_type);
            sdl_button_map_[sdl_btn_enum] = button_type_enum;
            button_graph_map_[button_type_enum] = button_binding.graph_id;
            if (button_binding.function_key) {
                function_button_ = button_type_enum;
            }
        }
    }
    [[nodiscard]] ButtonType getButtonType(const SDL_GamepadButton sdl_button) const {
        if (sdl_button_map_.contains(sdl_button)) {
            return sdl_button_map_.at(sdl_button);
        }
        return BUTTON_NONE;
    }

    [[nodiscard]] int getGraphId(const ButtonType button) const {
        if (!button_graph_map_.contains(button)) {
            return -1;
        }
        return button_graph_map_.at(button);
    }

    [[nodiscard]] ButtonType getFunctionButton() const {
        return function_button_;
    }
    
    [[nodiscard]] bool isFunctionButton(const ButtonType button) const {
        return function_button_ == button;
    }

    static ButtonBindingCache& getInstance() {
        static ButtonBindingCache instance;
        return instance;
    }
    // 禁止拷贝
    ButtonBindingCache(const ButtonBindingCache&) = delete;
    ButtonBindingCache& operator=(const ButtonBindingCache&) = delete;
};

#endif //SWITCH_AUTO_CORE_BUTTONBINDINGCACHE_H