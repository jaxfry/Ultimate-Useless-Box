#pragma once

#include <Arduino.h>

// Inputs
constexpr uint8_t PIN_MAIN_SWITCH = 1;
constexpr uint8_t PIN_VBATT_SENSE = 5;

// I2C bus shared by OLED and ToF.
constexpr uint8_t PIN_I2C_SDA = 2;
constexpr uint8_t PIN_I2C_SCL = 12;

// ToF control lines.
constexpr uint8_t PIN_TOF_SHUT = 10;

// Servo outputs.
constexpr uint8_t PIN_SERVO_1 = 4;
constexpr uint8_t PIN_SERVO_2 = 8;

// DRV8833 control.
constexpr uint8_t PIN_MOT_NSLEEP = 17;

constexpr uint8_t PIN_MOT_AIN1 = 6;
constexpr uint8_t PIN_MOT_AIN2 = 7;

constexpr uint8_t PIN_MOT_BIN1 = 15;
constexpr uint8_t PIN_MOT_BIN2 = 16;

// Audio output.
constexpr uint8_t PIN_BUZZER = 48;
