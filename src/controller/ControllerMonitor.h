#ifndef SWITCH_AUTO_CORE_CONTROLLERMONITOR_H
#define SWITCH_AUTO_CORE_CONTROLLERMONITOR_H

#include <SDL2/SDL.h>
#include <iostream>
#include <thread>
#include <atomic>
#include <string>

class GamepadStatus {
public:
    std::map<ButtonType, bool> inputs = {};
    int leftStickX = 0;
    int leftStickY = 0;
    int rightStickX = 0;
    int rightStickY = 0;
    ImuData imuData = {};
    GamepadStatus() {
        reset();
    }
    void reset() {
        inputs.clear();
        leftStickX = 0;
        leftStickY = 0;
        rightStickY = 0;
        rightStickY = 0;
        imuData = {0, 0, -4096, 0, 0, 0};
    }

    void send() const {
        for (auto [fst, snd] : inputs) {
            const auto btn = fst;
            if (snd) {
                SwitchControlLibrary::getInstance().pressButton(btn);
            } else {
                SwitchControlLibrary::getInstance().releaseButton(btn);
            }
        }
        SwitchControlLibrary::getInstance().moveLeftAnalog(leftStickX, leftStickY);
        SwitchControlLibrary::getInstance().moveRightAnalog(rightStickX, rightStickY);
        SwitchControlLibrary::getInstance().setIMU(imuData.accX, imuData.accY, imuData.accZ, imuData.gyroX, imuData.gyroY, imuData.gyroZ);
        // printf("%d,%d,%d,%d,%d,%d\n", imuData.accX, imuData.accY, imuData.accZ, imuData.gyroX, imuData.gyroY, imuData.gyroZ);
    }
};

class ControllerMonitor {
public:
    static ControllerMonitor& getInstance() {
        static ControllerMonitor instance;
        return instance;
    }

    void start() {
        if (isRunning) return;

        std::cout << "=== Switch Pro Native Monitor ===" << std::endl;
        std::cout << "1. 摇杆映射: 笛卡尔 [-2047, 2047]" << std::endl;
        std::cout << "2. 支持运行中插拔手柄" << std::endl;
        std::cout << "3. 安全机制: 断连自动复位 & 状态自动同步" << std::endl; // 亮点说明
        std::cout << ">>> 正在后台等待设备..." << std::endl;

        isRunning = true;
        workerThread = std::thread(&ControllerMonitor::threadLoop, this);
    }

    void stop() {
        if (!isRunning) return;
        isRunning = false;
        if (workerThread.joinable()) {
            workerThread.join();
        }
        std::cout << "=== 监听线程已停止 ===" << std::endl;
    }

private:
    SDL_GameController* controller = nullptr;
    std::atomic<bool> isRunning{false};
    std::thread workerThread;
    GamepadStatus gamepad_status{};

    // --- 新增：按键状态追踪 (用于防丢信号) ---
    bool buttonStates[SDL_CONTROLLER_BUTTON_MAX] = {false};

    const float JOYSTICK_TARGET_MAX = 2047.0f;
    const float SDL_JOYSTICK_MAX = 32767.0f;

    // IMU 参数
    const float SWITCH_ACCEL_1G_RAW = 4096.0f;
    const float SWITCH_GYRO_MAX_DPS = 2000.0f;
    const float SWITCH_GYRO_MAX_VAL = 32767.0f;

    ControllerMonitor() {
        // 全局初始化一次
        if (SDL_Init(SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS) < 0) {
            std::cerr << "SDL Init Error: " << SDL_GetError() << std::endl;
        }

        SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");
        SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_SWITCH, "1");
        SDL_SetHint(SDL_HINT_ACCELEROMETER_AS_JOYSTICK, "0");
    }

    ~ControllerMonitor() {
        stop();
        if (controller) SDL_GameControllerClose(controller);
        SDL_Quit();
    }

    void threadLoop() {
        SDL_Event event;
        int checkTicks = 0;

        while (isRunning) {
            // 1. 处理事件
            gamepad_status.inputs.clear();
            while (SDL_PollEvent(&event)) {
                handleEvent(event);
            }
            gamepad_status.send();

            // 2. 周期性检查 (每隔约 1 秒)
            if (++checkTicks > 500) {
                checkTicks = 0;

                // 情况A: 没有连接手柄，尝试强制重连 (原有逻辑)
                if (!controller) {
                    tryReconnect();
                }
                else {
                    syncButtonStates();
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    }

    void syncButtonStates() {
        if (!controller) return;

        for (int i = 0; i < SDL_CONTROLLER_BUTTON_MAX; ++i) {
            bool actualDown = SDL_GameControllerGetButton(controller, static_cast<SDL_GameControllerButton>(i));

            // 如果记录的状态与硬件实际状态不符
            if (buttonStates[i] != actualDown) {
                // 1. 更新内部记录
                buttonStates[i] = actualDown;

                // 2. [新增] 将修正后的状态加入待发送队列
                // 这样下一次 send() 调用时，Switch 库才会收到 Press/Release 指令
                try {
                    ButtonType btn = getButtonType(static_cast<SDL_GameControllerButton>(i));
                    gamepad_status.inputs[btn] = actualDown;

                    const char* btnName = SDL_GameControllerGetStringForButton(static_cast<SDL_GameControllerButton>(i));
                    std::cout << "[Sync] 状态自动修复: " << btnName << (actualDown ? " (补按)" : " (补松)") << std::endl;
                } catch (...) {
                    // 忽略无效按键
                }
            }
        }
    }

    // --- 强制重扫 USB ---
    void tryReconnect() {
        // [关键步骤]
        // 在 macOS 后台线程中，普通的 SDL_PumpEvents 无法检测到硬件变化。
        // 我们必须暂时关闭 Joystick 子系统再重新打开，强制 SDL 重新扫描 USB 总线。

        SDL_QuitSubSystem(SDL_INIT_GAMECONTROLLER);
        SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER);

        // 重新启用 Sensor (因为重置子系统后配置会丢失)
        SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_SWITCH, "1");

        // 刷新事件泵
        SDL_PumpEvents();

        int numJoysticks = SDL_NumJoysticks();
        if (numJoysticks > 0) {
            // std::cout << "发现设备: " << numJoysticks << " 个，尝试连接..." << std::endl;
            for (int i = 0; i < numJoysticks; ++i) {
                if (SDL_IsGameController(i)) {
                    openController(i);
                    break;
                }
            }
        }
    }

    static ButtonType getButtonType(const SDL_GameControllerButton button) {
    switch (button) {
        case SDL_CONTROLLER_BUTTON_Y:
            return BUTTON_Y;
        case SDL_CONTROLLER_BUTTON_X:
            return BUTTON_X;
        case SDL_CONTROLLER_BUTTON_B:
            return BUTTON_B;
        case SDL_CONTROLLER_BUTTON_A:
            return BUTTON_A;

        // --- 系统功能键 ---
        case SDL_CONTROLLER_BUTTON_BACK:
            return BUTTON_MINUS;    // Switch 的 "-" 键通常映射为 Back
        case SDL_CONTROLLER_BUTTON_START:
            return BUTTON_PLUS;     // Switch 的 "+" 键通常映射为 Start
        case SDL_CONTROLLER_BUTTON_GUIDE:
            return BUTTON_HOME;     // Home 键
        case SDL_CONTROLLER_BUTTON_MISC1:
            return BUTTON_CAPTURE;  // SDL中 MISC1 对应 Switch 的截图键

        // --- 摇杆按压 (Stick Click) ---
        case SDL_CONTROLLER_BUTTON_LEFTSTICK:
            return BUTTON_THUMB_L;  // 左摇杆按下
        case SDL_CONTROLLER_BUTTON_RIGHTSTICK:
            return BUTTON_THUMB_R;  // 右摇杆按下

        // --- 肩键 (Bumpers) ---
        case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
            return BUTTON_L;        // L键
        case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
            return BUTTON_R;        // R键

        // --- 十字键 (D-Pad) ---
        case SDL_CONTROLLER_BUTTON_DPAD_UP:
            return DPAD_UP;
        case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
            return DPAD_DOWN;
        case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
            return DPAD_LEFT;
        case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
            return DPAD_RIGHT;

        // --- 未映射 / 无效键 ---
        case SDL_CONTROLLER_BUTTON_INVALID:
        default:
            throw std::out_of_range("无效按键");;
    }
}
    void handleEvent(const SDL_Event& event) {
        switch (event.type) {
            case SDL_QUIT: isRunning = false; break;

            case SDL_CONTROLLERDEVICEADDED:
                openController(event.cdevice.which);
                break;

            case SDL_CONTROLLERDEVICEREMOVED:
                if (controller) {
                    SDL_GameControllerClose(controller);
                    controller = nullptr;
                    gamepad_status.reset();
                    std::cout << ">>> 设备已断开，等待重新连接..." << std::endl;
                }
                break;
            case SDL_CONTROLLERBUTTONUP:
                // [新增] 更新状态记录
                buttonStates[event.cbutton.button] = false;
                gamepad_status.inputs[getButtonType(static_cast<SDL_GameControllerButton>(event.cbutton.button))] = false;
                break;
            case SDL_CONTROLLERBUTTONDOWN:
                // [新增] 更新状态记录
                buttonStates[event.cbutton.button] = true;
                gamepad_status.inputs[getButtonType(static_cast<SDL_GameControllerButton>(event.cbutton.button))] = true;
                break;

            case SDL_CONTROLLERAXISMOTION:
                processJoystickAndTrigger(event.caxis);
                break;

            case SDL_CONTROLLERSENSORUPDATE:
                processIMU(event.csensor);
                break;
            default: ;
        }
    }

    void processJoystickAndTrigger(const SDL_ControllerAxisEvent& axis) {
        // ZL / ZR
        if (axis.axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT) {
            gamepad_status.inputs[BUTTON_ZL] = axis.value > 0;
        }
        if (axis.axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT) {
            gamepad_status.inputs[BUTTON_ZR] = axis.value > 0;
        }

        float ratio = static_cast<float>(axis.value) / SDL_JOYSTICK_MAX;
        if (ratio > 1.0f) ratio = 1.0f;
        if (ratio < -1.0f) ratio = -1.0f;

        switch (axis.axis) {
            case SDL_CONTROLLER_AXIS_LEFTX:
                gamepad_status.leftStickX = ratio * JOYSTICK_TARGET_MAX;
                break;
            case SDL_CONTROLLER_AXIS_RIGHTX:
                gamepad_status.rightStickX = ratio * JOYSTICK_TARGET_MAX;
                break;
            // Y轴取反：SDL向下为正(+)，笛卡尔向下为负(-)
            case SDL_CONTROLLER_AXIS_LEFTY:
                gamepad_status.leftStickY = -ratio * JOYSTICK_TARGET_MAX;
                break;
            case SDL_CONTROLLER_AXIS_RIGHTY:
                gamepad_status.rightStickY = -ratio * JOYSTICK_TARGET_MAX;
                break;
            default:
        }
    }

    void processIMU(const SDL_ControllerSensorEvent& sensor) {
        // SDL_STANDARD_GRAVITY 宏在不同平台可能定义不同，直接用 9.80665f 更稳
        if (sensor.sensor == SDL_SENSOR_ACCEL) {
            constexpr float GRAVITY = 9.80665f;
            const float scale = SWITCH_ACCEL_1G_RAW / GRAVITY;
            const int16_t accX = static_cast<int16_t>(-sensor.data[0] * scale);
            const int16_t accY = static_cast<int16_t>(sensor.data[2] * scale);
            const int16_t accZ = static_cast<int16_t>(-sensor.data[1] * scale);
            gamepad_status.imuData.accX = accX;
            gamepad_status.imuData.accY = accY;
            gamepad_status.imuData.accZ = accZ;

        }
        else if (sensor.sensor == SDL_SENSOR_GYRO) {
            constexpr float radToDeg = 180.0f / M_PI;
            const float degToRaw = SWITCH_GYRO_MAX_VAL / SWITCH_GYRO_MAX_DPS;
            const float combinedScale = radToDeg * degToRaw;
            const int16_t gyroX = static_cast<int16_t>(-sensor.data[0] * combinedScale);
            const int16_t gyroY = static_cast<int16_t>(sensor.data[2] * combinedScale);
            const int16_t gyroZ = static_cast<int16_t>(-sensor.data[1] * combinedScale);
            gamepad_status.imuData.gyroX = gyroX;
            gamepad_status.imuData.gyroY = gyroY;
            gamepad_status.imuData.gyroZ = gyroZ;
        }
    }

    void openController(const int index) {
        if (controller) return;

        controller = SDL_GameControllerOpen(index);

        if (controller) {
            std::cout << ">>> 已连接: " << SDL_GameControllerName(controller) << std::endl;
            // 启用传感器
            SDL_GameControllerSetSensorEnabled(controller, SDL_SENSOR_ACCEL, SDL_TRUE);
            SDL_GameControllerSetSensorEnabled(controller, SDL_SENSOR_GYRO, SDL_TRUE);

            // [新增] 连接时重置状态
            for(int i=0; i<SDL_CONTROLLER_BUTTON_MAX; ++i) buttonStates[i] = false;
        }
    }
};

#endif