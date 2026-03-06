//
// Created by churunfa on 2026/3/6.
//

#ifndef SWITCH_AUTO_CORE_USBTRANSPORT_H
#define SWITCH_AUTO_CORE_USBTRANSPORT_H

#include <boost/asio.hpp>
#include <iostream>
#ifdef __APPLE__
#include <termios.h>
#include <sys/ioctl.h>
#include <IOKit/serial/ioss.h>
#endif

#ifdef _WIN32
    #include <windows.h>
    #include <iostream>
#else
    #include <fcntl.h>
    #include <termios.h>
    #include <unistd.h>
    #include <dirent.h>
#endif

#include <memory>
#include <boost/asio/serial_port.hpp>

#include "ITransport.h"

class UsbTransport : public ITransport {
    std::string port_name;
    std::unique_ptr<boost::asio::serial_port> port;
    // 添加 Asio 的 io_context
    boost::asio::io_context io_context;
    static std::string AutoDetectPort() {
#ifdef _WIN32
        // Windows: 通过注册表查找串口
        HKEY hKey;
        if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "HARDWARE\\DEVICEMAP\\SERIALCOMM", 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
            return "";
        }

        char valueName[256];
        char data[256];
        DWORD valueNameSize, dataSize, type;
        DWORD index = 0;
        std::string foundPort;

        // 遍历所有串口
        while (true) {
            valueNameSize = sizeof(valueName);
            dataSize = sizeof(data);
            const LONG result = RegEnumValueA(hKey, index, valueName, &valueNameSize, NULL, &type, (LPBYTE)data, &dataSize);

            if (result != ERROR_SUCCESS) break;

            if (type == REG_SZ) {
                const std::string portName(data);
                // 简单的过滤逻辑：通常 ESP32/USB 串口在 Windows 上就是 COMx
                // 如果你有特定的 USB VID/PID 需求，Windows 上通常需要更复杂的 SetupAPI
                // 这里默认返回找到的最后一个 COM 口，或者你可以添加逻辑优先匹配 "COM3" 等
                foundPort = portName;
            }
            index++;
        }
        RegCloseKey(hKey);
        return foundPort; // 返回找到的一个端口，如 "COM3"

#else
        DIR* dir = opendir("/dev");
        dirent* ent;
        if (dir) {
            while ((ent = readdir(dir)) != nullptr) {
                if (std::string name = ent->d_name; name.find("tty.wchusbserial") != std::string::npos  ||
                                                    name.find("cu.usbmodem") != std::string::npos ||
                                                    name.find("cu.usbserial") != std::string::npos) {
                    closedir(dir);
                    return "/dev/" + name;
                                                    }
            }
            closedir(dir);
        }
        return "";
#endif
    }

    void initSerial() {
        std::cout << "初始化连接:" << port_name << std::endl;
        if (port_name.empty()) {
            std::cout << "端口名称为空" << std::endl;
            return;
        }

        try {
            if (port && port->is_open()) {
                port->close();
            }

            port = std::make_unique<boost::asio::serial_port>(io_context, port_name);

            // 配置其他常规参数
            port->set_option(boost::asio::serial_port_base::character_size(8));
            port->set_option(boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::none));
            port->set_option(boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::one));
            port->set_option(boost::asio::serial_port_base::flow_control(boost::asio::serial_port_base::flow_control::none));

            // --- 处理波特率 ---
#ifdef __APPLE__
            // macOS 下使用底层 ioctl 设置非标准的高波特率
            const int fd = port->native_handle();
            speed_t speed = 3000000;
            if (ioctl(fd, IOSSIOSPEED, &speed) == -1) {
                std::cout << "[警告] macOS 自定义波特率 3000000 设置失败，可能会通信异常" << std::endl;
            } else {
                std::cout << "macOS 波特率 3000000 设置成功" << std::endl;
            }
#else
            // Windows 和 Linux 下，Boost 通常能直接处理高波特率
            port->set_option(boost::asio::serial_port_base::baud_rate(3000000));
#endif
            // -----------------

            std::cout << "连接初始化成功" << std::endl;
        } catch (const boost::system::system_error& e) {
            std::cout << "打开端口失败: " << e.what() << std::endl;
            port.reset();
        }
    }

public:
    UsbTransport() {
        UsbTransport::connect();
    }
    bool connect() override {
        port_name = AutoDetectPort();
        if (!port_name.empty()) {
            initSerial();
        }
        return isConnected();
    }
    bool send(const std::vector<uint8_t>& data) override {
        return send(data.data(), data.size());
    }
    bool send(const void * str, const size_t len) override {
        return boost::asio::write(*port, boost::asio::buffer(str, len));
    }

    bool isConnected() override {
        if (port_name.empty() || !port || !port->is_open()) {
            return false;
        }
        return true;
    }

    void close() override {
        // 1. 停止后台线程 (如果正在运行)
        port_name = "";

        // 2. 释放串口资源
        if (port && port->is_open()) {
            boost::system::error_code ec;

            // 【核心修改】：显式声明一个变量接收 close 的返回值！

            // 判断返回值，彻底满足 Clang-Tidy 对 "返回值不可忽略" 的要求
            if (const boost::system::error_code return_ec = port->close(ec)) {
                std::cout << "[警告] 关闭串口时出现异常: " << return_ec.message() << std::endl;
            } else {
                std::cout << "[系统] 串口已正常关闭。" << std::endl;
            }

            port.reset();    // 释放 unique_ptr
            std::cout << "[系统] 串口资源清理完毕。" << std::endl;
        }
    }

    void serialRead() override {}
};

#endif //SWITCH_AUTO_CORE_USBTRANSPORT_H