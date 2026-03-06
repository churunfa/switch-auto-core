//
// Created by churunfa on 2026/3/6.
//

#ifndef SWITCH_AUTO_CORE_ITRANSPORT_H
#define SWITCH_AUTO_CORE_ITRANSPORT_H

#include <vector>

class ITransport {
public:
    virtual ~ITransport() = default;
    virtual bool connect() = 0;
    virtual bool send(const std::vector<uint8_t>& data) = 0;
    virtual bool send(const void * str, size_t len) = 0;
    virtual bool isConnected() = 0;
    virtual void close() = 0;
    virtual void serialRead() = 0;
};

#endif //SWITCH_AUTO_CORE_ITRANSPORT_H