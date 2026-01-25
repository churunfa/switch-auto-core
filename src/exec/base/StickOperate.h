//
// Created by churunfa on 2026/1/24.
//

#ifndef SWITCH_AUTO_CORE_STICKOPERATE_H
#define SWITCH_AUTO_CORE_STICKOPERATE_H

#include "OperatorStrategy.h"
#include "repo/base/BaseOperate.h"

class StickOperator : public OperatorStrategy {
public:
    bool execute(BaseOperate& base_operate, std::string& params, const bool reset) const override {
        if (base_operate.ename == "LEFT_STICK") {
            if (reset) {
                switch_control_library.resetLeftAnalog();
                switch_control_library.sendReport();
                return true;
            }
            const auto param_vector = get_param_vector(params);
            switch_control_library.moveLeftAnalog(std::stoi(param_vector[0]), std::stoi(param_vector[1]));
            switch_control_library.sendReport();
            return true;
        }
        if (base_operate.ename == "RIGHT_STICK") {
            if (reset) {
                switch_control_library.resetRightAnalog();
                switch_control_library.sendReport();
                return true;
            }
            const auto param_vector = get_param_vector(params);
            switch_control_library.moveRightAnalog(std::stoi(param_vector[0]), std::stoi(param_vector[1]));
            switch_control_library.sendReport();
            return true;
        }
        return false;
    }
};


#endif //SWITCH_AUTO_CORE_STICKOPERATE_H