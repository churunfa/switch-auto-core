#ifndef SWITCH_AUTO_CORE_CONTROLLERMONITOR_H
#define SWITCH_AUTO_CORE_CONTROLLERMONITOR_H

#include <SDL3/SDL.h>
#include <iostream>
#include <thread>
#include <atomic>
#include <string>
#include <map>

#include "cache/ButtonBindingCache.h"
#include "repo/combination/CombinationGraph.h"

class GamepadStatus {
public:
    std::map<ButtonType, bool> inputs = {};
    int leftStickX = 0;
    int leftStickY = 0;
    bool sendLeftStick = false;
    int rightStickX = 0;
    int rightStickY = 0;
    bool sendRightStick = false;
    ImuData imuData = {};
    bool sendImuData = false;
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

    void clearInput() {
        if (const auto function_button = ButtonBindingCache::getInstance().getFunctionButton(); inputs[function_button]) {
            return;
        }
        inputs.clear();
    }

    void send() {
        bool func_run = false;
        const auto function_button = ButtonBindingCache::getInstance().getFunctionButton();
        if (inputs[function_button]) {
            func_run = true;
        }
        // 如果没有函数按键，则按功能按键
        if (!func_run) {
            for (auto [fst, snd] : inputs) {
                if (fst != function_button) {
                    const auto btn = fst;
                    if (snd) {
                        SwitchControlLibrary::getInstance().pressButton(btn);
                    } else {
                        SwitchControlLibrary::getInstance().releaseButton(btn);
                    }
                }
            }
        } else {
        bool original_func_press = false;
        int execGraphId = -1;
        for (auto [fst, snd] : inputs) {
            if (fst == function_button) {
                continue;
            }
            if (snd) {
                // 函数按键，未配置的按照功能按键原始功能处理
                const int graph_id = ButtonBindingCache::getInstance().getGraphId(fst);
                if (graph_id == -1) {
                    original_func_press = true;
                    continue;
                }
                if (CombinationGraphCache::getInstance().findCombinationGraphById(graph_id)) {
                    execGraphId = graph_id;
                }
            }
        }
        // 处理功能按键
        if (original_func_press) {
            SwitchControlLibrary::getInstance().pressButton(function_button);
        } else {
            SwitchControlLibrary::getInstance().releaseButton(function_button);
        }
        if (execGraphId != -1 && !TopoSession::async_running) {
            // 执行拓扑图
            try {
                TopoSession::asyncExec(execGraphId, 1);
            } catch (const std::exception& e) {
                std::cerr << "Error: " << e.what() << std::endl;
            }
        }
        }

        if (sendLeftStick) {
            SwitchControlLibrary::getInstance().moveLeftAnalog(leftStickX, leftStickY);
            sendLeftStick = false;
        }
        if (sendRightStick) {
            SwitchControlLibrary::getInstance().moveRightAnalog(rightStickX, rightStickY);
            sendRightStick = false;
        }
        if (sendImuData) {
            SwitchControlLibrary::getInstance().setIMU(imuData.accX, imuData.accY, imuData.accZ, imuData.gyroX, imuData.gyroY, imuData.gyroZ);
            sendImuData = false;
        }
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
            gamepad_status.clearInput();
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
        return ButtonBindingCache::getInstance().getButtonType(button);
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
        const int newStick = static_cast<int>(ratio * JOYSTICK_TARGET_MAX);
        switch (axis.axis) {
            case SDL_GAMEPAD_AXIS_LEFTX: {
                if (abs(newStick - gamepad_status.leftStickX) > 128) {
                    gamepad_status.leftStickX = newStick;
                    gamepad_status.sendLeftStick = true;
                }
                break;
            }
            case SDL_GAMEPAD_AXIS_RIGHTX: {
                if (abs(newStick - gamepad_status.rightStickX) > 128) {
                    gamepad_status.rightStickX = newStick;
                    gamepad_status.sendRightStick = true;
                }
                break;
            }
            case SDL_GAMEPAD_AXIS_LEFTY: {
                if (abs(newStick - gamepad_status.leftStickY) > 128) {
                    gamepad_status.leftStickY = -newStick;
                    gamepad_status.sendLeftStick = true;
                }
                break;
            }
            case SDL_GAMEPAD_AXIS_RIGHTY: {
                if (abs(newStick - gamepad_status.rightStickY) > 128) {
                    gamepad_status.rightStickY = -newStick;
                    gamepad_status.sendRightStick = true;
                }
                break;
            }
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

            const boost::endian::little_int16_t newAccX = static_cast<int16_t>(-sensor.data[2] * scale);
            const boost::endian::little_int16_t newAccY = static_cast<int16_t>(-sensor.data[0] * scale);
            const boost::endian::little_int16_t newAccZ = static_cast<int16_t>(sensor.data[1] * scale);

            if (abs(newAccX.value() - gamepad_status.imuData.accX.value()) > 50) {
                gamepad_status.imuData.accX = newAccX;
                gamepad_status.sendImuData = true;
            }
            if (abs(newAccY.value() - gamepad_status.imuData.accY.value()) > 50) {
                gamepad_status.imuData.accY = newAccY;
                gamepad_status.sendImuData = true;
            }
            if (abs(newAccZ.value() - gamepad_status.imuData.accZ.value()) > 50) {
                gamepad_status.imuData.accZ = newAccZ;
                gamepad_status.sendImuData = true;
            }

        } else if (sensor.sensor == SDL_SENSOR_GYRO) {
            // 3. 转换回 Switch 的 Raw 单位
            // 系数 = (180 / PI) * (32767 / 2000)
            const float combinedScale = 57.29578f * (SWITCH_GYRO_MAX_VAL / SWITCH_GYRO_MAX_DPS);

            const auto rawX = deadSpace(static_cast<int16_t>(-sensor.data[2] * combinedScale));
            const auto rawY = deadSpace(static_cast<int16_t>(-sensor.data[0] * combinedScale));
            const auto rawZ = deadSpace(static_cast<int16_t>(sensor.data[1] * combinedScale));

            if (abs(rawX - gamepad_status.imuData.gyroX.value()) > 50) {
                gamepad_status.imuData.gyroX = rawX;
                gamepad_status.sendImuData = true;
            }
            if (abs(rawY - gamepad_status.imuData.gyroY.value()) > 50) {
                gamepad_status.imuData.gyroY = rawY;
                gamepad_status.sendImuData = true;
            }
            if (abs(rawZ - gamepad_status.imuData.gyroZ.value()) > 50) {
                gamepad_status.imuData.gyroZ = rawZ;
                gamepad_status.sendImuData = true;
            }
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