//
// Created by churunfa on 2026/1/26.
//

#ifndef SWITCH_AUTO_CORE_BASEOPERATEMAPPER_H
#define SWITCH_AUTO_CORE_BASEOPERATEMAPPER_H
#include <google/protobuf/repeated_ptr_field.h>

#include "repo/base/BaseOperate.h"

namespace base::operate {
    class BaseOperate;
}

class BaseOperateMapper {
public:
    static void FillBaseOperateProto(const BaseOperate& src, base::operate::BaseOperate* dest);
    static BaseOperate buildBaseOperate(const base::operate::BaseOperate &proto);
    static std::vector<BaseOperate> buildBaseOperates(const google::protobuf::RepeatedPtrField<base::operate::BaseOperate> &protos);
};

#endif //SWITCH_AUTO_CORE_BASEOPERATEMAPPER_H