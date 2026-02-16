//
// Created by churunfa on 2026/2/16.
//

#ifndef SWITCH_AUTO_CORE_CACHESERVICEINTERFACE_H
#define SWITCH_AUTO_CORE_CACHESERVICEINTERFACE_H

class CacheService {
public:
    virtual ~CacheService() = default;
    // 具体的算法接口
    virtual void load() = 0;
};

#endif //SWITCH_AUTO_CORE_CACHESERVICEINTERFACE_H