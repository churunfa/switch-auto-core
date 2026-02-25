//
// Created by churunfa on 2026/1/18.
//

#ifndef CONTROLLER_SWITCH_REPORT_H
#define CONTROLLER_SWITCH_REPORT_H
#include <cstdint>
#include <boost/endian/arithmetic.hpp>

#ifdef _MSC_VER
#pragma pack(push, 1)
#endif

typedef struct 
#ifndef _MSC_VER
__attribute__((packed, aligned(1)))
#endif
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

#ifdef _MSC_VER
#pragma pack(pop)
#endif

#ifdef _MSC_VER
#pragma pack(push, 1)
#endif

typedef struct 
#ifndef _MSC_VER
__attribute__((packed, aligned(1)))
#endif
{
    uint8_t data[3];
} SwitchAnalog;

#ifdef _MSC_VER
#pragma pack(pop)
#endif


#ifdef _MSC_VER
#pragma pack(push, 1)
#endif

typedef struct 
#ifndef _MSC_VER
__attribute__((packed, aligned(1)))
#endif
{
    boost::endian::little_int16_t accX;
    boost::endian::little_int16_t accY;
    boost::endian::little_int16_t accZ;
    boost::endian::little_int16_t gyroX;
    boost::endian::little_int16_t gyroY;
    boost::endian::little_int16_t gyroZ;
} ImuData;

#ifdef _MSC_VER
#pragma pack(pop)
#endif

#ifdef _MSC_VER
#pragma pack(push, 1)
#endif

typedef struct 
#ifndef _MSC_VER
__attribute__((packed, aligned(1)))
#endif
{
    SwitchInputReport inputs;
    SwitchAnalog leftStick;
    SwitchAnalog rightStick;
    ImuData imuData[3];
} SwitchProReport;

#ifdef _MSC_VER
#pragma pack(pop)
#endif


#endif //CONTROLLER_SWITCH_REPORT_H