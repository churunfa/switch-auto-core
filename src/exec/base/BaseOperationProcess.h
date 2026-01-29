//
// Created by churunfa on 2026/1/24.
//

#ifndef SWITCH_AUTO_CORE_OPERATIONCONTEXT_H
#define SWITCH_AUTO_CORE_OPERATIONCONTEXT_H

#include "ButtonOperator.h"
#include "EmptyOperate.h"
#include "ImuOperator.h"
#include "OperatorStrategy.h"
#include "ResetAllOperate.h"
#include "StickOperate.h"

class OperatorStrategy;

class BaseOperationProcess {
    std::vector<std::unique_ptr<OperatorStrategy>> strategies;
    BaseOperationProcess() {
        strategies.push_back(std::make_unique<ButtonOperator>());
        strategies.push_back(std::make_unique<StickOperator>());
        strategies.push_back(std::make_unique<ImuOperator>());
        strategies.push_back(std::make_unique<ResetAllOperator>());
        strategies.push_back(std::make_unique<EmptyOperator>());
    }
public:
    void run(const BaseOperate& base_operate, const std::string& params, const bool reset) const {
        for (const auto& strategy : strategies) {
            if (strategy -> execute(base_operate, params, reset)) {
                return;
            }
        }
        throw std::out_of_range("不支持的操作类型" + base_operate.ename);
    }
    static BaseOperationProcess& getInstance() {
        static BaseOperationProcess instance;
        return instance;
    }
    // 禁止拷贝
    BaseOperationProcess(const BaseOperationProcess&) = delete;
    BaseOperationProcess& operator=(const BaseOperationProcess&) = delete;
};

#endif //SWITCH_AUTO_CORE_OPERATIONCONTEXT_H