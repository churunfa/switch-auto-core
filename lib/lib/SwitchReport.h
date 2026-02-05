//
// Created by churunfa on 2026/1/18.
//

#ifndef CONTROLLER_SWITCH_REPORT_H
#define CONTROLLER_SWITCH_REPORT_H
#include <cstdint>

typedef struct __attribute((packed, aligned(1)))
{
    // byte 00
    uint8_t buttonY : 1;
    uint8_t buttonX : 1;
    uint8_t buttonB : 1;
    uint8_t buttonA : 1;
    uint8_t buttonRightSR : 1;
    uint8_t buttonRightSL : 1;
    uint8_t buttonR : 1;
    uint8_t buttonZR : 1;

    // byte 01
    uint8_t buttonMinus : 1;
    uint8_t buttonPlus : 1;
    uint8_t buttonThumbR : 1;
    uint8_t buttonThumbL : 1;
    uint8_t buttonHome : 1;
    uint8_t buttonCapture : 1;
    uint8_t dummy : 1;
    uint8_t chargingGrip : 1;

    // byte 02
    uint8_t dpadDown : 1;
    uint8_t dpadUp : 1;
    uint8_t dpadRight : 1;
    uint8_t dpadLeft : 1;
    uint8_t buttonLeftSL : 1;
    uint8_t buttonLeftSR : 1;
    uint8_t buttonL : 1;
    uint8_t buttonZL : 1;
} SwitchInputReport;

typedef struct __attribute((packed, aligned(1))){
    uint8_t data[3];
} SwitchAnalog;


typedef struct __attribute((packed, aligned(1))){
    int16_t accX;
    int16_t accY;
    int16_t accZ;
    int16_t gyroX;
    int16_t gyroY;
    int16_t gyroZ;
} ImuData;

typedef struct __attribute((packed, aligned(1))) {
    SwitchInputReport inputs;
    SwitchAnalog leftStick;
    SwitchAnalog rightStick;
    ImuData imuData[3];
} SwitchProReport;


#endif //CONTROLLER_SWITCH_REPORT_H