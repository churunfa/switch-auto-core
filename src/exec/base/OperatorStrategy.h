//
// Created by churunfa on 2026/1/24.
//

#ifndef SWITCH_AUTO_CORE_OPERATOR_H
#define SWITCH_AUTO_CORE_OPERATOR_H
#include <string>
#include <nlohmann/json.hpp>
#include "SwitchControlLibrary.h"

struct BaseOperate;

inline auto& switch_control_library = SwitchControlLibrary::getInstance();

inline std::vector<std::string> get_param_vector(const std::string& params) {
    return nlohmann::json::parse(params);
}

// 抽象策略：操作执行器
class OperatorStrategy {
public:
    virtual ~OperatorStrategy() = default;
    // 具体的算法接口
    [[nodiscard]] virtual bool execute(const BaseOperate& base_operate, const std::vector<int>& params, bool reset) const = 0;
};

#endif //SWITCH_AUTO_CORE_OPERATOR_H