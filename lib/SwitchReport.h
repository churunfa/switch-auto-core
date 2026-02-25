//
// Created by churunfa on 2026/1/18.
//

#ifndef CONTROLLER_SWITCH_REPORT_H
#define CONTROLLER_SWITCH_REPORT_H
#include <cstdint>
#include <boost/endian/arithmetic.hpp>

// 跨平台结构体内存对齐宏定义
#ifdef _MSC_VER
    #define PACKED_STRUCT_BEGIN __pragma(pack(push, 1))
    #define PACKED_STRUCT_END   __pragma(pack(pop))
    #define PACKED 
#else
    #define PACKED_STRUCT_BEGIN 
    #define PACKED_STRUCT_END   
    #define PACKED __attribute__((packed, aligned(1)))
#endif

PACKED_STRUCT_BEGIN
typedef struct PACKED {
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
PACKED_STRUCT_END

PACKED_STRUCT_BEGIN
typedef struct PACKED {
    uint8_t data[3];
} SwitchAnalog;
PACKED_STRUCT_END


PACKED_STRUCT_BEGIN
typedef struct PACKED {
    boost::endian::little_int16_t accX;
    boost::endian::little_int16_t accY;
    boost::endian::little_int16_t accZ;
    boost::endian::little_int16_t gyroX;
    boost::endian::little_int16_t gyroY;
    boost::endian::little_int16_t gyroZ;
} ImuData;
PACKED_STRUCT_END

PACKED_STRUCT_BEGIN
typedef struct PACKED {
    SwitchInputReport inputs;
    SwitchAnalog leftStick;
    SwitchAnalog rightStick;
    ImuData imuData[3];
} SwitchProReport;
PACKED_STRUCT_END

// 清理宏定义
#undef PACKED_STRUCT_BEGIN
#undef PACKED_STRUCT_END
#undef PACKED


#endif //CONTROLLER_SWITCH_REPORT_H