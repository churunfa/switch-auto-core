//
// Created by churunfa on 2026/1/18.
//
//

#include "SwitchControlLibrary.h"

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
}

SwitchControlLibrary::~SwitchControlLibrary() {
    if (worker.joinable()) {
        // 必须等待线程结束
        worker.join();
    }
}

void SwitchControlLibrary::loop() {
    while (running) {
        if (port_name.empty()) {
            port_name = SerialPort::AutoDetectPort();
        }
        if (port_name.empty()) {
            std::cout << "[搜索中] 等待设备... \n";
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }
        if (!serial.IsConnected()) {
            if (!serial.Connect(port_name)) {
                std::cout << "[连接失败] 端口: " << port_name << std::endl;
                port_name.clear();
                std::this_thread::sleep_for(std::chrono::seconds(1));
                continue;
            }
            std::cout << "[已连接] " << port_name << std::endl;
        }
        if (resetImuStatus) {
            std::lock_guard<std::recursive_mutex> lock(resetImuMtx);
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
    std::lock_guard<std::recursive_mutex> lock(reportMtx);

    if (memcmp(&lastSwitchReport, &switchReport, reportSize) == 0) {
        return;
    }
    // 先发头，失败就等下次重发
    if (!serial.Write(header, 2)) {
        std::cout << "header发送失败" << std::endl;
        return;
    }

    // 发送报表内容
    if (!serial.Write(&switchReport, reportSize)) {
        std::cout << "switchReport发送失败" << std::endl;
        return;
    }

    // 发送校验和
    uint8_t checkSum = 0;
    for (uint8_t i = 0; i < reportSize; i++) {
        checkSum ^= reinterpret_cast<uint8_t *>(&switchReport)[i];
    }
    if (!serial.Write(&checkSum, 1)) {
        std::cout << "checkSum发送失败" << std::endl;
        return;
    }
    // 更新lastSwitchReport
    memcpy(&lastSwitchReport, &switchReport, sizeof(SwitchProReport));
}


void SwitchControlLibrary::resetAll() {
    std::lock_guard<std::recursive_mutex> lock(reportMtx);

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
    std::lock_guard<std::recursive_mutex> lock(reportMtx);

    auto* ptr = reinterpret_cast<uint8_t*>(&switchReport);
    const int byteIdx = button / 8;
    const int bitOffset = button % 8;
    ptr[byteIdx] |= 1 << bitOffset;  // 设置为 1

}

void SwitchControlLibrary::releaseButton(const ButtonType button) {
    std::lock_guard<std::recursive_mutex> lock(reportMtx);

    auto* ptr = reinterpret_cast<uint8_t*>(&switchReport);
    const int byteIdx = button / 8;
    const int bitOffset = button % 8;
    ptr[byteIdx] &= ~(1 << bitOffset);  // 设置为 0
}

void SwitchControlLibrary::setIMU(const int16_t accX, const int16_t accY, const int16_t accZ, const int16_t gyroX, const int16_t gyroY, const int16_t gyroZ) {
    std::lock_guard<std::recursive_mutex> lock(resetImuMtx);
    setIMUCore(accX, accY, accZ, gyroX, gyroY, gyroZ);
    resetImuStatus = false;
}

void SwitchControlLibrary::setIMUCore(const int16_t accX, const int16_t accY, const int16_t accZ, const int16_t gyroX, const int16_t gyroY, const int16_t gyroZ) {
    std::lock_guard<std::recursive_mutex> lock(reportMtx);
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
    std::lock_guard<std::recursive_mutex> lock(resetImuMtx);
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

// 左摇杆
// 左下 -> (-2047,-2047) -> (1, 4095)
// 右上 -> (2047, 2047) -> (4095, 1)
uint16_t left_standard_x(int x) {
    // 标准化到 -2047 ~ 2047
    x = std::min(x, 2047);
    x = std::max(x, -2047);
    // 坐标转化
    return x + 2048;
}

uint16_t left_standard_y(int y) {
    // 标准化到 -2047 ~ 2047
    y = std::min(y, 2047);
    y = std::max(y, -2047);
    // 坐标转化
    return 2048 - y;
}

// 右摇杆
// 左下 -> (-2047,-2047) -> (1, 1)
// 右上 -> (2047, 2047) -> (4095, 4095)
uint16_t right_standard_x(int x) {
    // 标准化到 -2047 ~ 2047
    x = std::min(x, 2047);
    x = std::max(x, -2047);
    // 坐标转化
    return x + 2048;
}

uint16_t right_standard_y(int y) {
    // 标准化到 -2047 ~ 2047
    y = std::min(y, 2047);
    y = std::max(y, -2047);
    // 坐标转化
    return y + 2048;
}

void SwitchControlLibrary::moveLeftAnalog(const int x, const int y) {
    std::lock_guard<std::recursive_mutex> lock(reportMtx);
    setAnalogX(switchReport.leftStick, left_standard_x(x));
    setAnalogY(switchReport.leftStick, left_standard_y(y));
}

void SwitchControlLibrary::resetLeftAnalog() {
    std::lock_guard<std::recursive_mutex> lock(reportMtx);
    moveLeftAnalog(2048, 2048);
}

void SwitchControlLibrary::moveRightAnalog(const int x, const int y) {
    std::lock_guard<std::recursive_mutex> lock(reportMtx);
    setAnalogX(switchReport.rightStick, right_standard_x(x));
    setAnalogY(switchReport.rightStick, right_standard_y(y));
}

void SwitchControlLibrary::resetRightAnalog() {
    std::lock_guard<std::recursive_mutex> lock(reportMtx);
    moveRightAnalog(2048, 2048);
}

SwitchControlLibrary& SwitchControlLibrary::getInstance() {
    static SwitchControlLibrary instance;
    return instance;
}