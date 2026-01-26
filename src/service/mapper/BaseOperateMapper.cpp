//
// Created by churunfa on 2026/1/26.
//

#include "BaseOperateMapper.h"

#include "base_operate.pb.h"
#include "repo/base/BaseOperate.h"

void BaseOperateMapper::FillBaseOperateProto(const BaseOperate& src, base::operate::BaseOperate* dest) {
    dest->set_id(src.id);
    dest->set_ename(src.ename);
    dest->set_name(src.name);
    dest->set_param_size(src.param_size);
    dest->set_param_names(src.param_names);
    dest->set_init_params(src.init_params);
    dest->set_min_exec_time(src.min_exec_time);
    dest->set_min_reset_time(src.min_reset_time);
}

BaseOperate BaseOperateMapper::buildBaseOperate(const base::operate::BaseOperate &proto) {
    return {proto.id(), proto.ename(), proto.name(), proto.param_size(), proto.param_names(), proto.init_params(), proto.min_reset_time()};
}
