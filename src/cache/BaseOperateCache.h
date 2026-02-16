//
// Created by churunfa on 2026/2/16.
//

#ifndef SWITCH_AUTO_CORE_BASEOPERATECACHE_H
#define SWITCH_AUTO_CORE_BASEOPERATECACHE_H
#include "CacheServiceInterface.h"
#include "repo/base/BaseOperate.h"

class BaseOperateCache : public CacheService {
    std::map<int, std::shared_ptr<BaseOperate>> base_operate_cache_map_;
    BaseOperateCache() = default;
public:
    void load() override {
        for (const auto base_operates = BaseOperateRepo::findAll(); const auto& base_operate : base_operates) {
            base_operate_cache_map_[base_operate.id] = std::make_shared<BaseOperate>(base_operate);
        }
    }

    std::shared_ptr<BaseOperate> findBaseOperateById(const int id) {
        return base_operate_cache_map_.at(id);
    }

    static BaseOperateCache& getInstance() {
        static BaseOperateCache instance;
        return instance;
    }
    // 禁止拷贝
    BaseOperateCache(const BaseOperateCache&) = delete;
    BaseOperateCache& operator=(const BaseOperateCache&) = delete;
};
#endif //SWITCH_AUTO_CORE_BASEOPERATECACHE_H