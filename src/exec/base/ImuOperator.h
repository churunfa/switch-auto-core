//
// Created by churunfa on 2026/1/24.
//

#ifndef SWITCH_AUTO_CORE_IMUOPERATE_H
#define SWITCH_AUTO_CORE_IMUOPERATE_H


#include "OperatorStrategy.h"

class ImuOperator : public OperatorStrategy {
public:
    [[nodiscard]] bool execute(const BaseOperate& base_operate, const std::vector<int>& params, const bool reset) const override {
        if (base_operate.ename != "IMU") {
            return false;
        }
        if (reset) {
            switch_control_library.resetIMU();
        } else {
            const auto accX = static_cast<int16_t>(params[0]);
            const auto accY = static_cast<int16_t>(params[1]);
            const auto accZ = static_cast<int16_t>(params[2]);
            const auto gyroX = static_cast<int16_t>(params[3]);
            const auto gyroY = static_cast<int16_t>(params[4]);
            const auto gyroZ = static_cast<int16_t>(params[5]);
            switch_control_library.setIMU(accX, accY, accZ, gyroX, gyroY, gyroZ);
        }
        return true;
    }
};

#endif //SWITCH_AUTO_CORE_IMUOPERATE_H