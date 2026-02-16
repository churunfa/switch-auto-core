//
// Created by churunfa on 2026/2/16.
//

#include "ButtonBinding.h"

#include "repo/DatabaseManager.h"
#include <memory>

std::vector<ButtonBinding> ButtonBindingRepo::allButtonBinding() {
    return db.get_all<ButtonBinding>();
}

std::unique_ptr<ButtonBinding> ButtonBindingRepo::findById(int id) {
    return db.get_pointer<ButtonBinding>(id);
}

bool ButtonBindingRepo::updateBinding(int id, int sdl_btn, int button_type) {
    auto binding_ptr = findById(id);
    if (!binding_ptr) {
        return false;
    }
    
    auto binding = *binding_ptr;

    binding.button_type = button_type;
    
    // 更新Switch按钮名称
    switch (button_type) {
        case BUTTON_A: binding.button_name = "A"; break;
        case BUTTON_B: binding.button_name = "B"; break;
        case BUTTON_X: binding.button_name = "X"; break;
        case BUTTON_Y: binding.button_name = "Y"; break;
        case BUTTON_L: binding.button_name = "L"; break;
        case BUTTON_R: binding.button_name = "R"; break;
        case BUTTON_ZL: binding.button_name = "ZL"; break;
        case BUTTON_ZR: binding.button_name = "ZR"; break;
        case BUTTON_MINUS: binding.button_name = "-"; break;
        case BUTTON_PLUS: binding.button_name = "+"; break;
        case BUTTON_HOME: binding.button_name = "HOME"; break;
        case BUTTON_CAPTURE: binding.button_name = "📷"; break;
        case BUTTON_THUMB_L: binding.button_name = "按下L"; break;
        case BUTTON_THUMB_R: binding.button_name = "按下R"; break;
        case DPAD_UP: binding.button_name = "⬆️"; break;
        case DPAD_DOWN: binding.button_name = "⬇️"; break;
        case DPAD_LEFT: binding.button_name = "⬅️"; break;
        case DPAD_RIGHT: binding.button_name = "➡️"; break;
        default: binding.button_name = "Unknown"; break;
    }
    
    try {
        db.update(binding);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Update binding failed: " << e.what() << std::endl;
        return false;
    }
}

bool ButtonBindingRepo::setFunctionKey(const int id, const bool function_key) {
    const auto binding_ptr = findById(id);
    if (!binding_ptr) {
        return false;
    }
    
    auto binding = *binding_ptr;
    
    // 如果要设置为功能键
    if (function_key) {
        // 先取消所有其他按键的功能键状态
        try {
            db.transaction([&]() -> bool {
                // 将所有按键的function_key设为false
                for (auto all_bindings = db.get_all<ButtonBinding>(); auto& b : all_bindings) {
                    if (b.id != id) {
                        b.function_key = false;
                        db.update(b);
                    }
                }
                
                // 设置目标按键为功能键，并确保graph_id为-1
                binding.function_key = true;
                binding.graph_id = -1;
                db.update(binding);
                
                return true;
            });
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Set function key failed: " << e.what() << std::endl;
            return false;
        }
    }
    // 取消功能键状态
    binding.function_key = false;
    try {
        db.update(binding);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Set function key failed: " << e.what() << std::endl;
        return false;
    }
}

bool ButtonBindingRepo::bindGraph(const int id, const int graph_id) {
    const auto binding_ptr = findById(id);
    if (!binding_ptr) {
        return false;
    }
    
    auto binding = *binding_ptr;
    binding.graph_id = graph_id;
    // 绑定图后自动取消功能键属性
    binding.function_key = false;
    
    try {
        db.update(binding);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Bind graph failed: " << e.what() << std::endl;
        return false;
    }
}

bool ButtonBindingRepo::unbindGraph(const int id) {
    const auto binding_ptr = findById(id);
    if (!binding_ptr) {
        return false;
    }
    
    auto binding = *binding_ptr;
    binding.graph_id = -1;
    
    try {
        db.update(binding);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Unbind graph failed: " << e.what() << std::endl;
        return false;
    }
}
