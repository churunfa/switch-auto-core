#ifndef SWITCH_AUTO_CORE_CONTROLLERMONITOR_H
#define SWITCH_AUTO_CORE_CONTROLLERMONITOR_H

#include <SDL3/SDL.h>
#include <iostream>
#include <thread>
#include <atomic>
#include <string>
#include <map>


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
        rightStickX = 0;
        rightStickY = 0;
        imuData = {0, 0, 4096, 0, 0, 0};
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
        SwitchControlLibrary::getInstance().sendReport();
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
    // [SDL3] 类型变更为 SDL_Gamepad
    SDL_Gamepad* controller = nullptr;
    std::atomic<bool> isRunning{false};
    std::thread workerThread;
    GamepadStatus gamepad_status{};

    // [SDL3] 使用 COUNT 获取按键数量
    bool buttonStates[SDL_GAMEPAD_BUTTON_COUNT] = {false};

    const float JOYSTICK_TARGET_MAX = 2047.0f;
    const float SDL_JOYSTICK_MAX = 32767.0f;

    // IMU 参数
    const float SWITCH_ACCEL_1G_RAW = 4096.0f;
    const float SWITCH_GYRO_SCALE_CONST = 16.3835f;
    const float SWITCH_GYRO_MAX_DPS = 2000.0f;
    const float SWITCH_GYRO_MAX_VAL = 32767.0f;

    const unsigned int SAMPLE_RATE = 200; // Switch 手柄通常回报率为 200Hz

    ControllerMonitor() {
        if (!SDL_Init(SDL_INIT_GAMEPAD | SDL_INIT_EVENTS)) {
            std::cerr << "SDL Init Error: " << SDL_GetError() << std::endl;
        }

        SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");
        SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_SWITCH, "1");
        SDL_SetHint("SDL_GAMEPAD_USE_BUTTON_LABELS", "0");

    }

    ~ControllerMonitor() {
        stop();
        // [SDL3] CloseGamepad
        if (controller) SDL_CloseGamepad(controller);
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

            // 2. 周期性检查
            if (++checkTicks > 500) {
                checkTicks = 0;

                if (!controller) {
                    tryReconnect();
                }
                else {
                    syncButtonStates();
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    void syncButtonStates() {
        if (!controller) return;

        // [SDL3] 遍历使用 SDL_GAMEPAD_BUTTON_COUNT
        for (int i = 0; i < SDL_GAMEPAD_BUTTON_COUNT; ++i) {
            // [SDL3] GetGamepadButton

            if (const bool actualDown = SDL_GetGamepadButton(controller, static_cast<SDL_GamepadButton>(i)); buttonStates[i] != actualDown) {
                buttonStates[i] = actualDown;
                try {
                    ButtonType btn = getButtonType(static_cast<SDL_GamepadButton>(i));
                    if (btn == BUTTON_NONE) {
                        continue;
                    }
                    gamepad_status.inputs[btn] = actualDown;

                    // [SDL3] GetGamepadStringForButton
                    const char* btnName = SDL_GetGamepadStringForButton(static_cast<SDL_GamepadButton>(i));
                    std::cout << "[Sync] 状态自动修复: " << (btnName ? btnName : "Unknown") << (actualDown ? " (补按)" : " (补松)") << std::endl;
                } catch (...) {
                    // 忽略无效按键
                }
            }
        }
    }

    void tryReconnect() {
        // [SDL3] 仍然需要重新初始化子系统来强制刷新 HID 设备列表（针对 macOS 等）
        // GameController -> Gamepad
        SDL_QuitSubSystem(SDL_INIT_GAMEPAD);
        SDL_InitSubSystem(SDL_INIT_GAMEPAD);

        SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_SWITCH, "1");
        SDL_PumpEvents();

        // [SDL3] 变更：不再使用 NumJoysticks + Index，而是获取 ID 列表
        int count = 0;
        if (SDL_JoystickID* joysticks = SDL_GetJoysticks(&count)) {
            for (int i = 0; i < count; ++i) {
                if (const SDL_JoystickID instanceID = joysticks[i]; SDL_IsGamepad(instanceID)) {
                    openController(instanceID);
                    break;
                }
            }
            SDL_free(joysticks); // [SDL3] 必须手动释放列表
        }
    }

    static ButtonType getButtonType(const SDL_GamepadButton button) {
        switch (button) {
            // ... (保持原有的按键映射逻辑不变) ...
            case SDL_GAMEPAD_BUTTON_SOUTH: return BUTTON_B;
            case SDL_GAMEPAD_BUTTON_EAST: return BUTTON_A;
            case SDL_GAMEPAD_BUTTON_WEST: return BUTTON_Y;
            case SDL_GAMEPAD_BUTTON_NORTH: return BUTTON_X;
            case SDL_GAMEPAD_BUTTON_BACK: return BUTTON_MINUS;
            case SDL_GAMEPAD_BUTTON_START: return BUTTON_PLUS;
            case SDL_GAMEPAD_BUTTON_GUIDE: return BUTTON_HOME;
            case SDL_GAMEPAD_BUTTON_MISC1: return BUTTON_CAPTURE;
            case SDL_GAMEPAD_BUTTON_LEFT_STICK: return BUTTON_THUMB_L;
            case SDL_GAMEPAD_BUTTON_RIGHT_STICK: return BUTTON_THUMB_R;
            case SDL_GAMEPAD_BUTTON_LEFT_SHOULDER: return BUTTON_L;
            case SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER: return BUTTON_R;
            case SDL_GAMEPAD_BUTTON_DPAD_UP: return DPAD_UP;
            case SDL_GAMEPAD_BUTTON_DPAD_DOWN: return DPAD_DOWN;
            case SDL_GAMEPAD_BUTTON_DPAD_LEFT: return DPAD_LEFT;
            case SDL_GAMEPAD_BUTTON_DPAD_RIGHT: return DPAD_RIGHT;
            default: return BUTTON_NONE;
        }
    }

    void handleEvent(const SDL_Event& event) {
        switch (event.type) {
            case SDL_EVENT_QUIT: isRunning = false; break;
            case SDL_EVENT_GAMEPAD_ADDED:
                openController(event.gdevice.which);
                break;
            case SDL_EVENT_GAMEPAD_REMOVED:
                if (controller && event.gdevice.which == SDL_GetGamepadID(controller)) {
                    SDL_CloseGamepad(controller);
                    controller = nullptr;
                    gamepad_status.reset();
                    std::cout << ">>> 设备已断开，等待重新连接..." << std::endl;
                }
                break;
            case SDL_EVENT_GAMEPAD_BUTTON_UP:
                buttonStates[event.gbutton.button] = false;
                gamepad_status.inputs[getButtonType(static_cast<SDL_GamepadButton>(event.gbutton.button))] = false;
                break;
            case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
                buttonStates[event.gbutton.button] = true;
                gamepad_status.inputs[getButtonType(static_cast<SDL_GamepadButton>(event.gbutton.button))] = true;
                break;
            case SDL_EVENT_GAMEPAD_AXIS_MOTION:
                processJoystickAndTrigger(event.gaxis);
                break;
            case SDL_EVENT_GAMEPAD_SENSOR_UPDATE:
                processIMU(event.gsensor);
                break;
            default: ;
        }
    }

    // [SDL3] SDL_ControllerAxisEvent -> SDL_GamepadAxisEvent
    void processJoystickAndTrigger(const SDL_GamepadAxisEvent& axis) {
        if (axis.axis == SDL_GAMEPAD_AXIS_LEFT_TRIGGER) gamepad_status.inputs[BUTTON_ZL] = axis.value > 0;
        if (axis.axis == SDL_GAMEPAD_AXIS_RIGHT_TRIGGER) gamepad_status.inputs[BUTTON_ZR] = axis.value > 0;

        float ratio = static_cast<float>(axis.value) / SDL_JOYSTICK_MAX;
        if (ratio > 1.0f) ratio = 1.0f;
        if (ratio < -1.0f) ratio = -1.0f;

        switch (axis.axis) {
            case SDL_GAMEPAD_AXIS_LEFTX: gamepad_status.leftStickX = static_cast<int>(ratio * JOYSTICK_TARGET_MAX); break;
            case SDL_GAMEPAD_AXIS_RIGHTX: gamepad_status.rightStickX = static_cast<int>(ratio * JOYSTICK_TARGET_MAX); break;
            case SDL_GAMEPAD_AXIS_LEFTY: gamepad_status.leftStickY = -static_cast<int>(ratio * JOYSTICK_TARGET_MAX); break;
            case SDL_GAMEPAD_AXIS_RIGHTY: gamepad_status.rightStickY = -static_cast<int>(ratio * JOYSTICK_TARGET_MAX); break;
            default: ;
        }
    }

    static int16_t deadSpace(const int16_t x) {
        // return x;
        return (std::abs(x) < 50) ? 0 : x;
    }

    // [SDL3] SDL_ControllerSensorEvent -> SDL_GamepadSensorEvent
    void processIMU(const SDL_GamepadSensorEvent& sensor) {
        static std::atomic last_cnt = 0;
        static std::recursive_mutex imu_lock;
        std::lock_guard lock(imu_lock);

        if (sensor.sensor == SDL_SENSOR_ACCEL) {
            constexpr float GRAVITY = 9.80665f;
            const float scale = SWITCH_ACCEL_1G_RAW / GRAVITY;

            // 保持原有的坐标映射逻辑
            gamepad_status.imuData.accX = static_cast<int16_t>(-sensor.data[2] * scale);
            gamepad_status.imuData.accY = static_cast<int16_t>(-sensor.data[0] * scale);
            gamepad_status.imuData.accZ = static_cast<int16_t>(sensor.data[1] * scale);

        } else if (sensor.sensor == SDL_SENSOR_GYRO) {
            // 3. 转换回 Switch 的 Raw 单位
            // 系数 = (180 / PI) * (32767 / 2000)
            const float combinedScale = 57.29578f * (SWITCH_GYRO_MAX_VAL / SWITCH_GYRO_MAX_DPS);

            // 4. 应用转换和原有的坐标映射逻辑 (-X, Z, -Y)
            // 注意：gyro_si.axis.x/y/z 对应 SDL 的 0/1/2 索引
            const auto rawX = static_cast<int16_t>(-sensor.data[2] * combinedScale);
            const auto rawY = static_cast<int16_t>(-sensor.data[0] * combinedScale);
            const auto rawZ = static_cast<int16_t>(sensor.data[1] * combinedScale);

            gamepad_status.imuData.gyroX = deadSpace(rawX);
            gamepad_status.imuData.gyroY = deadSpace(rawY);
            gamepad_status.imuData.gyroZ = deadSpace(rawZ);
        }
    }

    // [SDL3] 参数改为 SDL_JoystickID
    void openController(const SDL_JoystickID instanceID) {
        if (controller) return;

        // [SDL3] OpenGamepad 接受 ID
        controller = SDL_OpenGamepad(instanceID);

        if (controller) {
            std::cout << ">>> 已连接: " << SDL_GetGamepadName(controller) << std::endl;
            // [SDL3] SetGamepadSensorEnabled
            SDL_SetGamepadSensorEnabled(controller, SDL_SENSOR_ACCEL, true);
            SDL_SetGamepadSensorEnabled(controller, SDL_SENSOR_GYRO, true);

            for(bool & buttonState : buttonStates) buttonState = false;
        }
    }
};

#endif