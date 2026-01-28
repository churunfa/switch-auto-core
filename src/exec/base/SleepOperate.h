//
// Created by churunfa on 2026/1/25.
//

#ifndef SWITCH_AUTO_CORE_SLEEPOPERATE_H
#define SWITCH_AUTO_CORE_SLEEPOPERATE_H
#include "OperatorStrategy.h"
#include "repo/base/BaseOperate.h"

class SleepOperator : public OperatorStrategy {
public:
    bool execute(const BaseOperate& base_operate, const std::string& params, const bool reset) const override {
        if (base_operate.ename != "SLEEP") {
            return false;
        }
        const auto param_vector = get_param_vector(params);
        sleep(std::stoi(param_vector[0]));
        return true;
    }
};


#endif //SWITCH_AUTO_CORE_SLEEPOPERATE_H