//
// Created by churunfa on 2026/2/16.
//

#ifndef SWITCH_AUTO_CORE_COMBINATIONGRAPHCACHE_H
#define SWITCH_AUTO_CORE_COMBINATIONGRAPHCACHE_H
#include "CacheServiceInterface.h"
#include "repo/combination/CombinationGraph.h"

class CombinationGraphCache : public CacheService {
    std::map<int, std::shared_ptr<CombinationGraph>> graph_map_;
    CombinationGraphCache() = default;
public:
    void load() override {
        for (const auto & combinations = CombinationRepo::allGraph(); const auto& combination : combinations) {
            if (const auto combination_graph = CombinationRepo::getGraphById(combination.id)) {
                graph_map_[combination.id] = std::make_shared<CombinationGraph>(*combination_graph);
            }
        }
    }

    std::shared_ptr<CombinationGraph> findCombinationGraphById(const int id) {
        if (!graph_map_.contains(id)) {
            return nullptr;
        }
        return graph_map_.at(id);
    }

    static CombinationGraphCache& getInstance() {
        static CombinationGraphCache instance;
        return instance;
    }
    // 禁止拷贝
    CombinationGraphCache(const CombinationGraphCache&) = delete;
    CombinationGraphCache& operator=(const CombinationGraphCache&) = delete;
};

#endif //SWITCH_AUTO_CORE_COMBINATIONGRAPHCACHE_H