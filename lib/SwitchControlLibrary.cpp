//
// Created by churunfa on 2026/1/18.
//
//

#include "SwitchControlLibrary.h"
#include <boost/asio.hpp>
#include <iostream>

#include "transport/BluetoothTransport.h"
#include "transport/UsbTransport.h"

long long getCurrentTime() {
    const auto now = std::chrono::system_clock::now();
    return std::chrono::time_point_cast<std::chrono::milliseconds>(now).time_since_epoch().count();
}

ImuData gravitationImuData[3] = {
    {0, 4096, 0, 0, 0, 0},
    {0, 4096, 0, 0, 0, 0},
    {0, 4096, 0, 0, 0, 0}
};

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


void SwitchControlLibrary::cleanup() {
    if (transport) {
        transport -> close();
    }
    // 3. 重置状态标记
    {
        std::lock_guard lock(reportMtx);
        switchReport = {};
        lastSwitchReport = {};
    }
}

void SwitchControlLibrary::init() {
    printf("开始初始化\n");
    // 1. 尝试初始化 USB
    if (transport == nullptr) {
        transport = new UsbTransport();
    }

    if (!transport->isConnected()) {
        printf("USB 未连接，尝试蓝牙...\n");

        // 2. 尝试初始化蓝牙
        transport = new BluetoothTransport();
        if (!transport->isConnected()) {
            printf("蓝牙连接失败。\n");
        }
    }
}

bool SwitchControlLibrary::connected() const {
    return transport != nullptr && transport->isConnected();
}

void SwitchControlLibrary::loop() {
    while (running) {
        if (!connected()) {
            init();
        }
        if (!connected()) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }
        if (switchReport.inputs.buttonHome) {
            wakeUp();
        }
        sendReport();
        serialRead();
        std::this_thread::sleep_for(std::chrono::milliseconds(15));
    }
}

void SwitchControlLibrary::wakeUp() const {
    constexpr static uint8_t wakeUpData[] = {0xAA, 0x55, 0x04, 0x00, 0x04};
    if (!connected()) return;

    try {
        transport->send(wakeUpData, 5);
    } catch (const std::exception& e) {
        std::cout << "发送失败："<<e.what() << std::endl;
        transport->close();
    }
}

void SwitchControlLibrary::sendReport() {
    std::lock_guard lock(reportMtx);
    if (!connected()) return;
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

    try {
        transport->send(buffer.data(), 2 + 1 + reportSize + 1);
        // 更新lastSwitchReport
        memcpy(&lastSwitchReport, &switchReport, sizeof(SwitchProReport));
    } catch (const std::exception& e) {
        std::cout << "发送失败："<<e.what() << std::endl;
        transport->close();
    }
}

void SwitchControlLibrary::sendBytes(const void *buf, const size_t count, const unsigned int timeout_ms) {
    std::lock_guard lock(reportMtx);
    if (!connected()) return;

    try {
        transport->send(buf, count);
    } catch (const std::exception& e) {
        std::cout << "发送失败："<<e.what() << std::endl;
        transport->close();
    }
}

void SwitchControlLibrary::sendVector(const std::vector<uint8_t>& in_buf, const uint8_t type) {
    std::vector<uint8_t> buf;
    buf.reserve(in_buf.size() + 4);
    buf.push_back(0xAA);
    buf.push_back(0x55);
    buf.push_back(type);
    uint8_t verifyCheckSum = 0;
    verifyCheckSum ^= type;
    for (unsigned char i : in_buf) {
        buf.push_back(i);
        verifyCheckSum ^= i;
    }
    buf.push_back(verifyCheckSum);

    sendBytes(buf.data(), buf.size(), 100);
}

void SwitchControlLibrary::sendStr(const std::string& str, const uint8_t type) {
    if (str.empty()) {
        return;
    }
    /**
     * 报文格式：0,1-当前分片   2,3-最后一个分片的编号   4-数据长度  >5-数据
     */
    constexpr uint8_t batch_size = 128;
    const uint16_t last_shard_index = str.size() / batch_size;

    for (uint16_t shard_index = 0; shard_index <= last_shard_index; shard_index++) {
        std::vector<uint8_t> in_buf;

        // 添加当前分片索引 (2字节)
        in_buf.push_back(shard_index & 0xFF);
        in_buf.push_back(shard_index >> 8 & 0xFF);

        // 添加最后一个分片索引 (2字节)
        in_buf.push_back(last_shard_index & 0xFF);
        in_buf.push_back(last_shard_index >> 8 & 0xFF);

        // 计算当前分片的数据
        const size_t start_pos = shard_index * batch_size;
        const size_t end_pos = std::min(start_pos + batch_size, str.size());
        const size_t data_length = end_pos - start_pos;

        // 添加数据长度 (1字节)
        in_buf.push_back(static_cast<uint8_t>(data_length));

        // 添加实际数据
        for (size_t i = start_pos; i < end_pos; ++i) {
            in_buf.push_back(static_cast<uint8_t>(str[i]));
        }

        // 发送当前分片
        sendVector(in_buf, type);
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}

void SwitchControlLibrary::resetAll() {
    std::lock_guard lock(reportMtx);

    std::memset(&switchReport, 0, sizeof(SwitchProReport));

    switchReport.inputs.dummy = 0;
    switchReport.inputs.chargingGrip = 1;
    switchReport.inputs.buttonLeftSL = 1;
    switchReport.inputs.buttonLeftSR = 1;
    resetLeftAnalog();
    resetRightAnalog();
    resetIMU();
}

void SwitchControlLibrary::pressButton(const ButtonType button) {
    std::lock_guard lock(reportMtx);

    if (button == BUTTON_NONE) {
        return;
    }

    auto* ptr = reinterpret_cast<uint8_t*>(&switchReport);
    const int byteIdx = button / 8;
    const int bitOffset = button % 8;
    ptr[byteIdx] |= 1 << bitOffset;
}

void SwitchControlLibrary::releaseButton(const ButtonType button) {
    std::lock_guard lock(reportMtx);

    if (button == BUTTON_NONE) {
        return;
    }

    auto* ptr = reinterpret_cast<uint8_t*>(&switchReport);
    const int byteIdx = button / 8;
    const int bitOffset = button % 8;
    ptr[byteIdx] &= ~(1 << bitOffset);
}

void SwitchControlLibrary::setIMU(const int16_t accX, const int16_t accY, const int16_t accZ, const int16_t gyroX, const int16_t gyroY, const int16_t gyroZ) {
    std::lock_guard lock(resetImuMtx);
    setIMUCore(accX, accY, accZ, gyroX, gyroY, gyroZ);
}

void SwitchControlLibrary::setIMU(const int index, const ImuData imu_data) {
    std::lock_guard lock(resetImuMtx);
    switchReport.imuData[index].accX = imu_data.accX;
    switchReport.imuData[index].accY = imu_data.accY;
    switchReport.imuData[index].accZ = imu_data.accZ;
    switchReport.imuData[index].gyroX = imu_data.gyroX;
    switchReport.imuData[index].gyroY = imu_data.gyroY;
    switchReport.imuData[index].gyroZ = imu_data.gyroZ;
}

void SwitchControlLibrary::setIMU(const ImuData *imu_datas) {
    std::lock_guard lock(resetImuMtx);
    for (int i = 0; i < 3; i++) {
        setIMU(i, imu_datas[i]);
    }
}

void SwitchControlLibrary::setIMUCore(const int16_t accX, const int16_t accY, const int16_t accZ, const int16_t gyroX, const int16_t gyroY, const int16_t gyroZ) {
    std::lock_guard lock(reportMtx);
    // IMU数据正常5ms采集一次，这里模拟下
    if (const long long currentTime = getCurrentTime(); currentTime > imuLastCollectTime + 5) {
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
    setIMU(gravitationImuData);
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

    while (!connected()) {
        std::cout << "未连接" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    const long long startTime = getCurrentTime();

    buffer[2] = 1;
    try {
        transport->send(buffer.data(), 49);
    } catch (const std::exception& e) {
        std::cout << "发送失败："<<e.what() << std::endl;
        transport->close();
    }
    const long long sendFinishedTime = getCurrentTime();

    const uint8_t delayTest[45] = {0};
    try {
        // 同步阻塞读取，直到填满 45 字节
        transport->send(delayTest, 45);
    } catch (const std::exception& e) {
         std::cout << "读取失败: " << e.what() << std::endl;
    }

    for (int i = 0; i < 45; i++) {
        if (delayTest[i] != buffer[i + 3]) {
            std::cout << "\n校验失败" << std::endl;
            return;
        }
    }
    const long long sendTime = getCurrentTime();
    std::cout << "消息处理耗时:" << sendFinishedTime - startTime << ",消息发送耗时" << (sendTime - startTime)/2 << std::endl;
}

void SwitchControlLibrary::serialRead() const {
    if (!connected()) return;
    transport->serialRead();
}

SwitchControlLibrary& SwitchControlLibrary::getInstance() {
    static SwitchControlLibrary instance;
    return instance;
}