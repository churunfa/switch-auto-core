//
// Created by churunfa on 2026/1/24.
//

#ifndef SWITCH_AUTO_CORE_STICKOPERATE_H
#define SWITCH_AUTO_CORE_STICKOPERATE_H

#include "OperatorStrategy.h"
#include "repo/base/BaseOperate.h"

inline bool stickValid(const int x) {
    if (x < 0 || x > 4097) {
        return false;
    }
    return true;
}

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
            const int x = std::stoi(param_vector[0]);
            const int y = std::stoi(param_vector[1]);
            if (!stickValid(x) || !stickValid(y)) {
                throw "Invalid stick param";
            }

            switch_control_library.moveLeftAnalog(x, y);
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
            const int x = std::stoi(param_vector[0]);
            const int y = std::stoi(param_vector[1]);
            if (!stickValid(x) || !stickValid(y)) {
                throw "Invalid stick param";
            }
            switch_control_library.moveRightAnalog(x, y);
            switch_control_library.sendReport();
            return true;
        }
        return false;
    }
};


#endif //SWITCH_AUTO_CORE_STICKOPERATE_H