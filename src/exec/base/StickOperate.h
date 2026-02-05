//
// Created by churunfa on 2026/1/24.
//

#ifndef SWITCH_AUTO_CORE_STICKOPERATE_H
#define SWITCH_AUTO_CORE_STICKOPERATE_H

#include "OperatorStrategy.h"
#include "repo/base/BaseOperate.h"

// 1~2048~4095 --> -2047~0~2047
inline int standard(const int x) {
    int standard_x = std::min(x, 2047);
    standard_x = std::max(standard_x, -2047);
    return standard_x;
}

class StickOperator : public OperatorStrategy {
public:
    [[nodiscard]] bool execute(const BaseOperate& base_operate, const std::vector<int>& params, const bool reset) const override {
        if (base_operate.ename == "LEFT_STICK") {
            if (reset) {
                switch_control_library.resetLeftAnalog();
                return true;
            }
            const int x = standard(params[0]);
            const int y = standard(params[1]);

            switch_control_library.moveLeftAnalog(x, y);
            return true;
        }
        if (base_operate.ename == "RIGHT_STICK") {
            if (reset) {
                switch_control_library.resetRightAnalog();
                return true;
            }
            const int x = standard(params[0]);
            const int y = standard(params[1]);
            switch_control_library.moveRightAnalog(x, y);
            return true;
        }
        return false;
    }
};


#endif //SWITCH_AUTO_CORE_STICKOPERATE_H