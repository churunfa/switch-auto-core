//
// Created by churunfa on 2026/1/25.
//

#ifndef SWITCH_AUTO_CORE_EMPTYOPERATE_H
#define SWITCH_AUTO_CORE_EMPTYOPERATE_H
#include "OperatorStrategy.h"
#include "repo/base/BaseOperate.h"

class EmptyOperator : public OperatorStrategy {
public:
    [[nodiscard]] bool execute(const BaseOperate& base_operate, const std::string& params, const bool reset) const override {
        if (base_operate.ename != "START_EMPTY" && base_operate.ename != "END_EMPTY" && base_operate.ename != "SLEEP") {
            return false;
        }
        return true;
    }
};


#endif //SWITCH_AUTO_CORE_EMPTYOPERATE_H