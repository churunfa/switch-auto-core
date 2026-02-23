//
// Created by churunfa on 2026/2/16.
//

#ifndef SWITCH_AUTO_CORE_CACHEPROCESS_H
#define SWITCH_AUTO_CORE_CACHEPROCESS_H
#include <iostream>
#include <thread>
#include <vector>
#include <atomic>

#include "BaseOperateCache.h"
#include "ButtonBindingCache.h"
#include "CacheServiceInterface.h"
#include "CombinationGraphCache.h"


class CacheLoader {
    std::vector<CacheService*> strategies;
    std::thread worker;
    std::atomic<bool> isRunning{true};
    CacheLoader() {
        strategies.push_back(&CombinationGraphCache::getInstance());
        strategies.push_back(&ButtonBindingCache::getInstance());
        strategies.push_back(&BaseOperateCache::getInstance());
        worker = std::thread(&CacheLoader::loop, this);
    }

    [[noreturn]] void loop() const {
        while (isRunning) {
            load();
            for (int i = 0; i < 10 && isRunning; ++i) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
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
    
    void stop() {
        isRunning = false;
        if (worker.joinable()) {
            worker.join();
        }
        std::cout << "缓存加载器已停止" << std::endl;
    }
    // 禁止拷贝
    CacheLoader(const CacheLoader&) = delete;
    CacheLoader& operator=(const CacheLoader&) = delete;
};

#endif //SWITCH_AUTO_CORE_CACHEPROCESS_H