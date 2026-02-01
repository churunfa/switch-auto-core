//
// Created by churunfa on 2026/1/24.
//

#ifndef SWITCH_AUTO_CORE_IMUOPERATE_H
#define SWITCH_AUTO_CORE_IMUOPERATE_H


#include "OperatorStrategy.h"

class ImuOperator : public OperatorStrategy {
public:
    [[nodiscard]] bool execute(const BaseOperate& base_operate, const std::string& params, const bool reset) const override {
        if (base_operate.ename != "IMU") {
            return false;
        }
        if (reset) {
            switch_control_library.resetIMU();
        } else {
            const auto param_vector = get_param_vector(params);
            const auto accX = static_cast<int16_t>(std::stoi(param_vector[0]));
            const auto accY = static_cast<int16_t>(std::stoi(param_vector[1]));
            const auto accZ = static_cast<int16_t>(std::stoi(param_vector[2]));
            const auto gyroX = static_cast<int16_t>(std::stoi(param_vector[3]));
            const auto gyroY = static_cast<int16_t>(std::stoi(param_vector[4]));
            const auto gyroZ = static_cast<int16_t>(std::stoi(param_vector[5]));
            switch_control_library.setIMU(accX, accY, accZ, gyroX, gyroY, gyroZ);
        }
        return true;
    }
};

#endif //SWITCH_AUTO_CORE_IMUOPERATE_H