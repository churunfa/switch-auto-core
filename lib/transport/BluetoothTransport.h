//
// Created by churunfa on 2026/3/6.
//

#ifndef SWITCH_AUTO_CORE_BLUETOOTHTRANSPORT_H
#define SWITCH_AUTO_CORE_BLUETOOTHTRANSPORT_H

#include "ITransport.h"
#include <simpleble/SimpleBLE.h>
#include <vector>
#include <string>
#include <memory>
#include <iostream>

class BluetoothTransport : public ITransport {
    // 常量定义，匹配 vSwitch Pro Controller 硬件配置
    const std::string SERVICE_UUID = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E";
    const std::string CHARACTERISTIC_UUID = "6E400002-B5A3-F393-E0A9-E50E24DCCA9E";
    const std::string TARGET_NAME = "vSwitch Pro Controller";

    std::unique_ptr<SimpleBLE::Peripheral> _peripheral = nullptr;
    bool _is_connected = false;

public:
    BluetoothTransport() {
        BluetoothTransport::connect();
    }
    ~BluetoothTransport() override { BluetoothTransport::close(); }

    bool connect() override {
        _peripheral.reset();
        _is_connected = false;

        const auto adapters = SimpleBLE::Adapter::get_adapters();
        if (adapters.empty()) {
            std::cerr << "未找到蓝牙适配器" << std::endl;
            return false;
        }

        auto adapter = adapters[0];
        std::cout << "正在使用适配器: " << adapter.identifier() << " 扫描设备..." << std::endl;

        // 异步扫描 2.5 秒
        adapter.scan_for(2500);

        for (auto peripherals = adapter.scan_get_results(); auto& p : peripherals) {
            // 匹配设备名称
            if (p.identifier() == TARGET_NAME) {
                std::cout << "发现目标设备: " << p.identifier() << " [" << p.address() << "]" << std::endl;

                try {
                    p.connect();
                    _peripheral = std::make_unique<SimpleBLE::Peripheral>(p);
                    _is_connected = true;
                    std::cout << "连接成功！" << std::endl;
                    return true;
                } catch (...) {
                    std::cerr << "连接失败" << std::endl;
                    return false;
                }
            }
        }

        std::cerr << "未找到名为 " << TARGET_NAME << " 的设备" << std::endl;
        return false;
    }

    bool send(const std::vector<uint8_t>& data) override {
        return send(data.data(), data.size());
    }

    bool send(const void* str, const size_t len) override {
        if (!_peripheral || !_is_connected) return false;
        const std::string payload(static_cast<const char*>(str), len);
        _peripheral->write_command(SERVICE_UUID, CHARACTERISTIC_UUID, payload);
        return true;
    }

    bool isConnected() override {
        // 第一步：基础连接检查
        if (!_peripheral || !_peripheral->is_connected()) {
            close();
            return false;
        }

        // 第二步：服务表发现检查
        // 如果 services 列表为空，说明虽然物理连上了，但 GATT 还没同步完成
        if (_peripheral->services().empty()) {
            std::cout << "GATT 服务表未就绪..." << std::endl;
            close();
            return false;
        }
        return true;
    }

    void close() override {
        if (_peripheral && _is_connected) {
            _peripheral->disconnect();
            _is_connected = false;
            std::cout << "蓝牙连接已关闭" << std::endl;
        }
    }

    void serialRead() override {}
};

#endif //SWITCH_AUTO_CORE_BLUETOOTHTRANSPORT_H