//
// Created by churunfa on 2026/1/24.
//

#ifndef SWITCH_AUTO_CORE_RESETAllOPERATE_H
#define SWITCH_AUTO_CORE_RESETAllOPERATE_H
#include "OperatorStrategy.h"
#include "repo/base/BaseOperate.h"

class ResetAllOperator : public OperatorStrategy {
public:
    bool execute(BaseOperate& base_operate, std::string& params, const bool reset) const override {
        if (base_operate.ename != "RESET_ALL") {
            return false;
        }
        switch_control_library.resetAll();
        switch_control_library.sendReport();
        return true;
    }
};

#endif //SWITCH_AUTO_CORE_RESETAllOPERATE_H