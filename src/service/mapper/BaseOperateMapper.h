//
// Created by churunfa on 2026/1/26.
//

#ifndef SWITCH_AUTO_CORE_BASEOPERATEMAPPER_H
#define SWITCH_AUTO_CORE_BASEOPERATEMAPPER_H
#include "repo/base/BaseOperate.h"

namespace base::operate {
    class BaseOperate;
}

class BaseOperateMapper {
public:
    static void FillBaseOperateProto(const BaseOperate& src, base::operate::BaseOperate* dest);
    static BaseOperate buildBaseOperate(const base::operate::BaseOperate &proto);
};

#endif //SWITCH_AUTO_CORE_BASEOPERATEMAPPER_H