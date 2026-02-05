//
// Created by churunfa on 2026/1/18.
//
//

#include "SwitchControlLibrary.h"
#include <libserialport.h>
#include <iostream>

long long getCurrentTime() {
    const auto now = std::chrono::system_clock::now();
    return std::chrono::time_point_cast<std::chrono::milliseconds>(now).time_since_epoch().count();

}

constexpr ImuData gravitationImuData[3] = {{0, -4090, 0}, {0, -4090, 0}, {0, -4090, 0}};

SwitchControlLibrary::SwitchControlLibrary() : switchReport{}, lastSwitchReport{} {
    resetAll();
    running = true;
    reportSize = sizeof(SwitchProReport);
    worker = std::thread(&SwitchControlLibrary::loop, this);
    imuLastCollectTime = getCurrentTime();
    buffer = std::vector<uint8_t>();
    buffer.reserve(2 + 1 + reportSize + 1);
    buffer.push_back(0xAA);
    buffer.push_back(0x55);
    for (int i = 0; i < 1 + reportSize + 1; ++i) {
        buffer.push_back(0);
    }
}

SwitchControlLibrary::~SwitchControlLibrary() {
    if (worker.joinable()) {
        // 必须等待线程结束
        worker.join();
    }
}

void SwitchControlLibrary::initSerial() {
    std::cout << "初始化连接:" << port_name << std::endl;
    if (port_name.empty() || sp_get_port_by_name(port_name.c_str(), &port) != SP_OK) {
        std::cout << "获取端口句柄失败" << std::endl;
        return;
    }

    if (sp_open(port, SP_MODE_READ_WRITE) != SP_OK) {
        std::cout << "打开端口失败" << std::endl;
        return;
    }

    sp_set_baudrate(port, 3000000); // 确保与 ESP32 一致
    sp_set_bits(port, 8);
    sp_set_parity(port, SP_PARITY_NONE);
    sp_set_stopbits(port, 1);
    sp_set_flowcontrol(port, SP_FLOWCONTROL_NONE);
    std::cout << "连接初始化成功" << std::endl;
}

void SwitchControlLibrary::cleanup() {
    // 1. 停止后台线程 (如果正在运行)
    // 注意：如果是从析构函数调用，running 应该已经设为 false
    port_name = "";

    // 2. 释放串口资源
    if (port != nullptr) {
        // 关闭串口
        sp_close(port);
        // 释放 libserialport 内部分配的内存
        sp_free_port(port);
        // 指针置空，防止野指针重复释放
        port = nullptr;
        std::cout << "[系统] 串口连接已关闭并释放资源。" << std::endl;
    }

    // 3. 重置状态标记
    // 这样 loop() 线程就能识别到断开状态并尝试重新连接
    {
        std::lock_guard lock(reportMtx);
        std::memset(&switchReport, 0, sizeof(SwitchProReport));
        std::memset(&lastSwitchReport, 0, sizeof(SwitchProReport));
    }

    // 如果有其他相关的互斥锁或条件变量，也在这里进行重置
}


void SwitchControlLibrary::loop(){
    while (running) {
        if (port_name.empty()) {
            port_name = SerialPort::AutoDetectPort();
            if (!port_name.empty()) {
                initSerial();
            }
        }
        if (port_name.empty()) {
            std::cout << "[搜索中] 等待设备... \n";
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }
        if (resetImuStatus) {
            std::lock_guard lock(resetImuMtx);
            setIMUCore(0, 0, -4096, 0, 0, 0);
            if (memcmp(switchReport.imuData, gravitationImuData, sizeof(ImuData) * 3) == 0) {
                // 三组Imu全部重置完成
                resetImuStatus = false;
            }

        }
        sendReport();
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}

void SwitchControlLibrary::sendReport() {
    std::lock_guard lock(reportMtx);

    if (memcmp(&lastSwitchReport, &switchReport, reportSize) == 0) {
        return;
    }
    buffer[2] = 0;
    const auto switch_report = reinterpret_cast<uint8_t *>(&switchReport);
    uint8_t checkSum = 0;
    for (size_t i = 0; i < reportSize; ++i) {
        buffer[3+i] = switch_report[i];
        checkSum ^= switch_report[i];
    }
    buffer[3+reportSize] = checkSum;

    if (const sp_return result = sp_blocking_write(port, buffer.data(), 2 + 1 + reportSize + 1, 5); result < 0) {
        std::cout << "发送失败" << std::endl;
        return;
    }
    std::cout<<std::endl;
    // 更新lastSwitchReport
    memcpy(&lastSwitchReport, &switchReport, sizeof(SwitchProReport));
}


void SwitchControlLibrary::resetAll() {
    std::lock_guard lock(reportMtx);

    std::memset(&switchReport, 0, sizeof(SwitchProReport));

    // 这些值一般不改
    switchReport.inputs.dummy = 0;
    switchReport.inputs.chargingGrip = 1;
    switchReport.inputs.buttonLeftSL = 1;
    switchReport.inputs.buttonLeftSR = 1;
    // 摇杆居中
    resetLeftAnalog();
    resetRightAnalog();
    // 体感只留重力
    resetIMU();
}

void SwitchControlLibrary::pressButton(const ButtonType button) {
    std::lock_guard lock(reportMtx);

    auto* ptr = reinterpret_cast<uint8_t*>(&switchReport);
    const int byteIdx = button / 8;
    const int bitOffset = button % 8;
    ptr[byteIdx] |= 1 << bitOffset;  // 设置为 1

}

void SwitchControlLibrary::releaseButton(const ButtonType button) {
    std::lock_guard lock(reportMtx);

    auto* ptr = reinterpret_cast<uint8_t*>(&switchReport);
    const int byteIdx = button / 8;
    const int bitOffset = button % 8;
    ptr[byteIdx] &= ~(1 << bitOffset);  // 设置为 0
}

void SwitchControlLibrary::setIMU(const int16_t accX, const int16_t accY, const int16_t accZ, const int16_t gyroX, const int16_t gyroY, const int16_t gyroZ) {
    std::lock_guard lock(resetImuMtx);
    setIMUCore(accX, accY, accZ, gyroX, gyroY, gyroZ);
    resetImuStatus = false;
}

void SwitchControlLibrary::setIMUCore(const int16_t accX, const int16_t accY, const int16_t accZ, const int16_t gyroX, const int16_t gyroY, const int16_t gyroZ) {
    std::lock_guard lock(reportMtx);
    const long long currentTime = getCurrentTime();
    // IMU数据正常5ms采集一次，这里模拟下
    if (currentTime > imuLastCollectTime + 5) {
        for (int i = 0; i < 2; i++) {
            switchReport.imuData[i].accX = switchReport.imuData[i + 1].accX;
            switchReport.imuData[i].accY = switchReport.imuData[i + 1].accY;
            switchReport.imuData[i].accZ = switchReport.imuData[i + 1].accZ;
            switchReport.imuData[i].gyroX = switchReport.imuData[i + 1].gyroX;
            switchReport.imuData[i].gyroY = switchReport.imuData[i + 1].gyroY;
            switchReport.imuData[i].gyroZ = switchReport.imuData[i + 1].gyroZ;
        }
        imuLastCollectTime = currentTime;
    }
    switchReport.imuData[2].accX = accX;
    switchReport.imuData[2].accY = accY;
    switchReport.imuData[2].accZ = accZ;
    switchReport.imuData[2].gyroX = gyroX;
    switchReport.imuData[2].gyroY = gyroY;
    switchReport.imuData[2].gyroZ = gyroZ;
}

void SwitchControlLibrary::resetIMU() {
    std::lock_guard lock(resetImuMtx);
    setIMUCore(0, 0, -4096, 0, 0, 0);
    resetImuStatus = true;
}

void SwitchControlLibrary::setAnalogX(SwitchAnalog& stick, const uint16_t x) {
    uint8_t *data = stick.data;
    data[0] = x & 0xFF;
    data[1] = (data[1] & 0xF0) | ((x >> 8) & 0x0F);
}

void SwitchControlLibrary::setAnalogY(SwitchAnalog& stick, const uint16_t y) {
    uint8_t *data = stick.data;
    data[1] = (data[1] & 0x0F) | ((y & 0x0F) << 4);
    data[2] = (y >> 4) & 0xFF;
}

uint16_t standardAnalog(int x) {
    // 标准化到 -2047 ~ 2047
    x = std::min(x, 2047);
    x = std::max(x, -2047);
    // 坐标转化
    return x + 2048;
}

void SwitchControlLibrary::moveLeftAnalog(const int x, const int y) {
    std::lock_guard lock(reportMtx);
    setAnalogX(switchReport.leftStick, standardAnalog(x));
    setAnalogY(switchReport.leftStick, standardAnalog(y));
}

void SwitchControlLibrary::resetLeftAnalog() {
    std::lock_guard lock(reportMtx);
    moveLeftAnalog(0, 0);
}

void SwitchControlLibrary::moveRightAnalog(const int x, const int y) {
    std::lock_guard lock(reportMtx);
    setAnalogX(switchReport.rightStick, standardAnalog(x));
    setAnalogY(switchReport.rightStick, standardAnalog(y));
}

void SwitchControlLibrary::resetRightAnalog() {
    std::lock_guard lock(reportMtx);
    moveRightAnalog(0, 0);
}

void SwitchControlLibrary::delayTest() {
    std::lock_guard lock(reportMtx);

    while (port_name.empty()) {
        std::cout<<"未连接"<<std::endl;
        sleep(1);
    }
    const long long startTime = getCurrentTime();

    // 类型
    buffer[2] = 1;
    if (const sp_return result = sp_blocking_write(port, buffer.data(), 49, 5); result < 0) {
        std::cout<<"发送失败"<<std::endl;
    }
    const long long sendFinishedTime = getCurrentTime();

    uint8_t delayTest[45] = {0};
    sp_blocking_read(port, delayTest, 45, 500);

    for (int i = 0; i < 45; i++) {
        if (delayTest[i] != buffer[i + 3]) {
            std::cout << "\n校验失败" << std::endl;
            return;
        }
    }
    const long long sendTime = getCurrentTime();
    std::cout<<"消息处理耗时:" << sendFinishedTime - startTime <<",消息发送耗时"<<(sendTime - startTime)/2<<std::endl;
}

SwitchControlLibrary& SwitchControlLibrary::getInstance() {
    static SwitchControlLibrary instance;
    return instance;
}