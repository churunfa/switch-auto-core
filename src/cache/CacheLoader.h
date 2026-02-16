//
// Created by churunfa on 2026/2/16.
//

#ifndef SWITCH_AUTO_CORE_CACHEPROCESS_H
#define SWITCH_AUTO_CORE_CACHEPROCESS_H
#include <iostream>
#include <thread>
#include <vector>

#include "BaseOperateCache.h"
#include "ButtonBindingCache.h"
#include "CacheServiceInterface.h"
#include "CombinationGraphCache.h"


class CacheLoader {
    std::vector<CacheService*> strategies;
    std::thread worker;
    CacheLoader() {
        strategies.push_back(&CombinationGraphCache::getInstance());
        strategies.push_back(&ButtonBindingCache::getInstance());
        strategies.push_back(&BaseOperateCache::getInstance());
        worker = std::thread(&CacheLoader::loop, this);
    }

    [[noreturn]] void loop() const {
        while (true) {
            load();
            std::this_thread::sleep_for(std::chrono::seconds(10));
        }
    }
    void load() const {
        for (const auto& strategy : strategies) {
            try {
                strategy->load();
            } catch (const std::exception& e) {
                std::cerr << "CacheLoad failed: " << e.what() << std::endl;
            }
        }
    }
public:
    static CacheLoader& getInstance() {
        static CacheLoader instance;
        return instance;
    }
    // 禁止拷贝
    CacheLoader(const CacheLoader&) = delete;
    CacheLoader& operator=(const CacheLoader&) = delete;
};

#endif //SWITCH_AUTO_CORE_CACHEPROCESS_H