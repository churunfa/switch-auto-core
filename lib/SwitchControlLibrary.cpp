//
// Created by churunfa on 2026/1/18.
//
//

#include "SwitchControlLibrary.h"
#include <boost/asio.hpp>
#include <iostream>
#include <cstring>
#ifdef __APPLE__
#include <termios.h>
#include <sys/ioctl.h>
#include <IOKit/serial/ioss.h>
#endif

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

void SwitchControlLibrary::initSerial() {
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

void SwitchControlLibrary::cleanup() {
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

    // 3. 重置状态标记
    {
        std::lock_guard lock(reportMtx);
        switchReport = {};
        lastSwitchReport = {};
    }
}

void SwitchControlLibrary::loop() {
    while (running) {
        if (port_name.empty()) {
            port_name = SerialPort::AutoDetectPort();
            if (!port_name.empty()) {
                initSerial();
            }
        }
        if (port_name.empty() || !port || !port->is_open()) {
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
    if (!port || !port->is_open()) return;

    try {
        boost::asio::write(*port, boost::asio::buffer(wakeUpData, 5));
    } catch (const boost::system::system_error&) {
        std::cout << "发送失败" << std::endl;
    }
}

void SwitchControlLibrary::sendReport() {
    std::lock_guard lock(reportMtx);
    if (!port || !port->is_open()) {
        return;
    }
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
        boost::asio::write(*port, boost::asio::buffer(buffer.data(), 2 + 1 + reportSize + 1));
        // 更新lastSwitchReport
        memcpy(&lastSwitchReport, &switchReport, sizeof(SwitchProReport));
    } catch (const boost::system::system_error&) {
        std::cout << "发送失败" << std::endl;
    }
}

void SwitchControlLibrary::sendBytes(const void *buf, const size_t count, const unsigned int timeout_ms) {
    std::lock_guard lock(reportMtx);
    if (!port || !port->is_open()) return;

    try {
        // Boost.Asio 原生的同步 write 并不直接支持 timeout。
        // 但串口发送通常会立即完成。如需严格超时，需用 async_write + 定时器处理。
        boost::asio::write(*port, boost::asio::buffer(buf, count));
    } catch (const boost::system::system_error&) {
        std::cout << "发送失败" << std::endl;
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

    while (port_name.empty() || !port || !port->is_open()) {
        std::cout << "未连接" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    const long long startTime = getCurrentTime();

    buffer[2] = 1;
    try {
        boost::asio::write(*port, boost::asio::buffer(buffer.data(), 49));
    } catch (const boost::system::system_error&) {
        std::cout << "发送失败" << std::endl;
    }
    const long long sendFinishedTime = getCurrentTime();

    uint8_t delayTest[45] = {0};
    try {
        // 同步阻塞读取，直到填满 45 字节
        boost::asio::read(*port, boost::asio::buffer(delayTest, 45));
    } catch (const boost::system::system_error& e) {
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

char temp_buf[1008611];
std::string line_buffer;

void SwitchControlLibrary::serialRead() { // 注意：去掉了 const，因为要操作非 const 的 io_context
    if (!port || !port->is_open()) return;

    boost::system::error_code read_ec;
    size_t bytes_read = 0;

    // 结合 Boost.Asio 定时器和异步读取，实现带有 3ms 超时的读取
    port->async_read_some(boost::asio::buffer(temp_buf, sizeof(temp_buf)),
        [&](const boost::system::error_code& ec, std::size_t bytes) {
            read_ec = ec;
            bytes_read = bytes;
        });

    io_context.restart();
    // 运行 3ms。如果在这 3ms 内读到了数据，run_for 也会提前返回或继续执行回调。
    io_context.run_for(std::chrono::milliseconds(3));

    // 如果 3ms 后事件循环还没停止（代表还没有读到数据），取消读取操作
    if (!io_context.stopped()) {
        port->cancel();
        io_context.run(); // 消费掉被取消的回调事件
    }

    if (bytes_read > 0 && !read_ec) {
        line_buffer.append(temp_buf, bytes_read);

        size_t pos = 0;
        while ((pos = line_buffer.find('\n')) != std::string::npos) {
            std::string distinct_line = line_buffer.substr(0, pos + 1);
            std::cout << "接收到数据:" << distinct_line;
            line_buffer.erase(0, pos + 1);
        }
    }
}

SwitchControlLibrary& SwitchControlLibrary::getInstance() {
    static SwitchControlLibrary instance;
    return instance;
}