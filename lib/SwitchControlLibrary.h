//
// Created by churunfa on 2026/1/18.
//

#ifndef CONTROLLER_SWITCH_CONTROL_LIBRARY_H
#define CONTROLLER_SWITCH_CONTROL_LIBRARY_H
#include <atomic>
#include <mutex>
#include <thread>

#include "SwitchReport.h"
#include "SerialPort.h"

typedef enum {
    BUTTON_Y = 0,
    BUTTON_X,
    BUTTON_B,
    BUTTON_A,
    BUTTON_SR,
    BUTTON_SL,
    BUTTON_R,
    BUTTON_ZR,

    BUTTON_MINUS, // 减号键
    BUTTON_PLUS, // 加号键
    BUTTON_THUMB_R, // 按下R键
    BUTTON_THUMB_L, // 按下L键
    BUTTON_HOME, // Home键
    BUTTON_CAPTURE, // 截图
    DUMMY, // 震动
    CHARGING_GRIP, //充电握把

    DPAD_DOWN, // 下
    DPAD_UP, // 上
    DPAD_RIGHT, // 右
    DPAD_LEFT, // 左
    BUTTON_LEFT_SL, // joy-con的SL键
    BUTTON_LEFT_SR, // joy-con的SR键
    BUTTON_L,
    BUTTON_ZL,
} ButtonType;

class SwitchControlLibrary {
public:
    SwitchControlLibrary();
    ~SwitchControlLibrary();
    void pressButton(ButtonType button);
    void releaseButton(ButtonType button);
    // 左摇杆，使用笛卡尔坐标系，x,y -> -2047~2047; 居中：0
    void moveLeftAnalog(int x, int y);
    void resetLeftAnalog();
    // 右摇杆，使用笛卡尔坐标系，x,y -> -2047~2047; 居中：0
    void moveRightAnalog(int x, int y);
    void resetRightAnalog();
    // 体感角速度和加速度
    void setIMU(int16_t accX, int16_t accY, int16_t accZ, int16_t gyroX, int16_t gyroY, int16_t gyroZ);
    void resetIMU();
    void resetAll();
    void sendReport();
    static SwitchControlLibrary& getInstance();

    // 禁止拷贝
    SwitchControlLibrary(const SwitchControlLibrary&) = delete;
    SwitchControlLibrary& operator=(const SwitchControlLibrary&) = delete;
private:
    SwitchProReport switchReport;
    SwitchProReport lastSwitchReport;
    uint8_t reportSize;
    SerialPort serial;
    std::string port_name;
    std::thread worker;
    std::recursive_mutex reportMtx;
    std::atomic<bool> running{false};
    std::atomic<bool> resetImuStatus{false};
    std::recursive_mutex resetImuMtx;
    uint8_t header[2]={0xAA, 0x55};
    long long imuLastCollectTime;

    void loop();
    void setIMUCore(int16_t accX, int16_t accY, int16_t accZ, int16_t gyroX, int16_t gyroY, int16_t gyroZ);
    static void setAnalogX(SwitchAnalog& stick, uint16_t x);
    static void setAnalogY(SwitchAnalog& stick, uint16_t y);
};


#endif //CONTROLLER_SWITCH_CONTROL_LIBRARY_H