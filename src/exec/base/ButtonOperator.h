//
// Created by churunfa on 2026/1/24.
//

#ifndef SWITCH_AUTO_CORE_BUTTONOPERATOR_H
#define SWITCH_AUTO_CORE_BUTTONOPERATOR_H
#include "OperatorStrategy.h"
#include "repo/base/BaseOperate.h"

static const std::unordered_map<std::string, ButtonType> StringToButtonTypeMap = {
    {"BUTTON_Y", ButtonType::BUTTON_Y},
    {"BUTTON_X", ButtonType::BUTTON_X},
    {"BUTTON_B", ButtonType::BUTTON_B},
    {"BUTTON_A", ButtonType::BUTTON_A},
    {"BUTTON_R", ButtonType::BUTTON_R},
    {"BUTTON_ZR", ButtonType::BUTTON_ZR},
    {"BUTTON_MINUS", ButtonType::BUTTON_MINUS},
    {"BUTTON_PLUS", ButtonType::BUTTON_PLUS},
    {"BUTTON_THUMB_R", ButtonType::BUTTON_THUMB_R},
    {"BUTTON_THUMB_L", ButtonType::BUTTON_THUMB_L},
    {"BUTTON_HOME", ButtonType::BUTTON_HOME},
    {"BUTTON_CAPTURE", ButtonType::BUTTON_CAPTURE},
    {"DPAD_DOWN", ButtonType::DPAD_DOWN},
    {"DPAD_UP", ButtonType::DPAD_UP},
    {"DPAD_RIGHT", ButtonType::DPAD_RIGHT},
    {"DPAD_LEFT", ButtonType::DPAD_LEFT},
    {"BUTTON_L", ButtonType::BUTTON_L},
    {"BUTTON_ZL", ButtonType::BUTTON_ZL},
};

// 进阶：直接获取枚举值（带安全性检查）
inline std::optional<ButtonType> getButtonType(const std::string& str) {
    if (const auto it = StringToButtonTypeMap.find(str); it != StringToButtonTypeMap.end()) return it->second;
    return std::nullopt;
}

class ButtonOperator : public OperatorStrategy {
public:
    [[nodiscard]] bool execute(const BaseOperate& base_operate, const std::vector<int>& params, const bool reset) const override {
        const auto button_type_opt = getButtonType(base_operate.ename);
        if (!button_type_opt.has_value()) {
            return false;
        }
        const auto button_type = button_type_opt.value();
        if (reset) {
            switch_control_library.releaseButton(button_type);
        } else {
            switch_control_library.pressButton(button_type);
        }
        return true;
    }
};

#endif //SWITCH_AUTO_CORE_BUTTONOPERATOR_H