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
            {0, SDL_GAMEPAD_BUTTON_SOUTH, "B", BUTTON_B, "B", false, -1},
            {0, SDL_GAMEPAD_BUTTON_EAST, "A", BUTTON_A, "A", false, -1},
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
            {0, SDL_GAMEPAD_BUTTON_RIGHT_PADDLE1, "右SR键", BUTTON_SR, "SR", false, -1},
            {0, SDL_GAMEPAD_BUTTON_LEFT_PADDLE1, "左SL键", BUTTON_LEFT_SL, "左SL键", false, -1},
            {0, SDL_GAMEPAD_BUTTON_LEFT_PADDLE2, "左SR键", BUTTON_LEFT_SR, "左SR键", false, -1},
            {0, SDL_GAMEPAD_BUTTON_RIGHT_PADDLE2, "右SL键", BUTTON_SL, "SL", false, -1},
            {0, SDL_GAMEPAD_BUTTON_TOUCHPAD, "触控板", BUTTON_NONE, "NONE", false, -1},
            {0, SDL_GAMEPAD_BUTTON_MISC2, "附加按键1", BUTTON_NONE, "NONE", false, -1},
            {0, SDL_GAMEPAD_BUTTON_MISC3, "附加按键2", BUTTON_NONE, "NONE", false, -1},
            {0, SDL_GAMEPAD_BUTTON_MISC4, "附加按键3", BUTTON_NONE, "NONE", false, -1},
            {0, SDL_GAMEPAD_BUTTON_MISC5, "附加按键4", BUTTON_NONE, "NONE", false, -1},
            {0, SDL_GAMEPAD_BUTTON_MISC6, "附加按键5", BUTTON_NONE, "NONE", false, -1},
            {0, SDL_GAMEPAD_BUTTON_COUNT, "未知按键", BUTTON_NONE, "NONE", false, -1},
            {0, SDL_GAMEPAD_BUTTON_INVALID, "无效按键", BUTTON_NONE, "NONE", false, -1},
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
    static std::unique_ptr<ButtonBinding> findById(int id);
    static bool updateBinding(int id, int sdl_btn, int button_type);
    static bool setFunctionKey(int id, bool function_key);
    static bool bindGraph(int id, int graph_id);
    static bool unbindGraph(int id);
};

#endif //SWITCH_AUTO_CORE_BUTTONBINDING_H