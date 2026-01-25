//
// Created by churunfa on 2026/1/25.
//

#include "BaseOperate.h"

#include <utility>

#include "repo/DatabaseManager.h"

std::vector<BaseOperate> BaseOperateRepo::findAll() {
    return db.get_all<BaseOperate>();
}

std::optional<BaseOperate> BaseOperateRepo::findOneById(const int id) {
    if (const auto base_operate = db.get_pointer<BaseOperate>(id)) {
        return *base_operate;
    }
    return std::nullopt;
}

std::optional<BaseOperate> BaseOperateRepo::findOneByEname(std::string ename) {
    if (const auto results = db.get_all_pointer<BaseOperate>(where(c(&BaseOperate::ename) == std::move(ename)), limit(1)); !results.empty() && results[0] != nullptr) {
        // 返回解引用后的对象，std::optional 会处理拷贝
        return *results[0];
    }
    return std::nullopt;
}