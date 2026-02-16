//
// Created by churunfa on 2026/2/16.
//

#ifndef SWITCH_AUTO_CORE_BUTTONBINDING_H
#define SWITCH_AUTO_CORE_BUTTONBINDING_H

#include <SDL3/SDL_gamepad.h>

#include "SwitchControlLibrary.h"
#include "sqlite_orm/sqlite_orm.h"

using namespace sqlite_orm;

struct ButtonBinding {
    int id;
    int sdl_btn;
    std::string sdl_btn_name;
    int button_type;
    std::string button_name;
    bool function_key;
    int graph_id;

    static auto getDescription() {
        return make_table("button_binding",
            make_column("id", &ButtonBinding::id, primary_key()),
            make_column("sdl_btn", &ButtonBinding::sdl_btn, unique()),
            make_column("sdl_btn_name", &ButtonBinding::sdl_btn_name),
            make_column("button_type", &ButtonBinding::button_type),
            make_column("button_name", &ButtonBinding::button_name),
            make_column("function_key", &ButtonBinding::function_key),
            make_column("graph_id", &ButtonBinding::graph_id)
        );
    }

    static const std::vector<ButtonBinding>& getStaticDefaults() {
        static const std::vector<ButtonBinding> defaults = {
            {0, SDL_GAMEPAD_BUTTON_EAST, "A", BUTTON_A, "A", false, -1},
            {0, SDL_GAMEPAD_BUTTON_SOUTH, "B", BUTTON_B, "B", false, -1},
            {0, SDL_GAMEPAD_BUTTON_WEST, "Y", BUTTON_Y, "Y", false, -1},
            {0, SDL_GAMEPAD_BUTTON_NORTH, "X", BUTTON_X, "X", false, -1},
            {0, SDL_GAMEPAD_BUTTON_BACK, "-", BUTTON_MINUS, "-", false, -1},
            {0, SDL_GAMEPAD_BUTTON_START, "+", BUTTON_PLUS, "+", false, -1},
            {0, SDL_GAMEPAD_BUTTON_GUIDE, "home", BUTTON_HOME, "home", false, -1},
            {0, SDL_GAMEPAD_BUTTON_MISC1, "📷", BUTTON_CAPTURE, "📷", true, -1},
            {0, SDL_GAMEPAD_BUTTON_LEFT_STICK, "按下L", BUTTON_THUMB_L, "按下L", false, -1},
            {0, SDL_GAMEPAD_BUTTON_RIGHT_STICK, "按下R", BUTTON_THUMB_R, "按下R", false, -1},
            {0, SDL_GAMEPAD_BUTTON_LEFT_SHOULDER, "L", BUTTON_L, "L", false, -1},
            {0, SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER, "R", BUTTON_R, "R", false, -1},
            {0, SDL_GAMEPAD_BUTTON_DPAD_UP, "⬆️", DPAD_UP, "⬆️", false, -1},
            {0, SDL_GAMEPAD_BUTTON_DPAD_DOWN, "⬇️", DPAD_DOWN, "⬇️", false, -1},
            {0, SDL_GAMEPAD_BUTTON_DPAD_LEFT, "⬅️", DPAD_LEFT, "⬅️", false, -1},
            {0, SDL_GAMEPAD_BUTTON_DPAD_RIGHT, "➡️", DPAD_RIGHT, "➡️", false, -1},
        };
        return defaults;
    }

    template<typename T>
    static void initData(T& db){
        db.transaction([&]() -> bool {
            auto count = db.template count<ButtonBinding>();
            if (count == 0) {
                int idx = 0;
                for (auto buttonBinding : getStaticDefaults()) {
                    buttonBinding.id = ++idx;
                    db.template replace<ButtonBinding>(buttonBinding);
                }
            }
            return true;
        });
    }
};

class ButtonBindingRepo {
public:
    static std::vector<ButtonBinding> allButtonBinding();
};

#endif //SWITCH_AUTO_CORE_BUTTONBINDING_H