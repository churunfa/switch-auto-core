//
// Created by churunfa on 2026/1/24.
//

#ifndef SWITCH_AUTO_CORE_OPERATIONCONTEXT_H
#define SWITCH_AUTO_CORE_OPERATIONCONTEXT_H

#include "ButtonOperator.h"
#include "ImuOperator.h"
#include "OperatorStrategy.h"
#include "ResetAllOperate.h"
#include "StickOperate.h"

class OperatorStrategy;

class BaseOperationProcess {
    std::vector<std::unique_ptr<OperatorStrategy>> strategies;

public:
    BaseOperationProcess() {
        strategies.push_back(std::make_unique<ButtonOperator>());
        strategies.push_back(std::make_unique<StickOperator>());
        strategies.push_back(std::make_unique<ImuOperator>());
        strategies.push_back(std::make_unique<ResetAllOperator>());
    }
    // 执行业务
    void run(BaseOperate& base_operate, std::string& params, const bool reset) const {
        for (const auto& strategy : strategies) {
            if (strategy -> execute(base_operate, params, reset)) {
                return;
            }
        }
        std::cerr << "未知操作" << base_operate.ename << std::endl;
    }
};

#endif //SWITCH_AUTO_CORE_OPERATIONCONTEXT_H