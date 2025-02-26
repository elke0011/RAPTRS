/*
Copyright (c) 2016 - 2019 Regents of the University of Minnesota and Bolder Flight Systems Inc.
MIT License; See LICENSE.md for complete details
Author: Brian Taylor
*/

#pragma once

#include "i2c_t3.h"
#include "Arduino.h"

// FMU Software Version
const String SoftwareVersion = "0.11.0";

// Debug port
const uint32_t kDebugBaud = 115200;                           // Baudrate for status and debug messages

// SOC port
static HardwareSerial &kSocUart = Serial1;                    // Serial port used for communicating with SOC
const uint32_t kSocBaud = 1500000;                            // Baudrate for communicating with SOC

// BFS bus
static i2c_t3 &kBfsPort = Wire;                               // I2C port
const i2c_pins kBfsPins = I2C_PINS_18_19;                     // I2C pins
const uint32_t kBfsRate = 6000000;                            // I2C rate
const uint8_t kBfsInt1Pin = 20;
const uint8_t kBfsInt2Pin = 17;

// Buffers
const size_t kEepromMaxSize = 4096;                           // Max EEPROM size on board
const size_t kUartBufferMaxSize = 4096;                       // Max size for UART buffers

// Sensors
const uint8_t kMpu9250CsPin = 24;                             // FMU integrated MPU9250 CS pin
const uint8_t kMpu9250IntPin = 27;                            // FMU integrated MPU9250 interrupt pin
const float kMpu9250Orientation[3][3] = {{0.0f,1.0f,0.0f},    // FMU integrated MPU9250 orientation relative to FMU
                                        {-1.0f,0.0f,0.0f},
                                        {0.0f,0.0f,1.0f}};
const uint8_t kBme280CsPin = 26;                              // FMU integrated BME280 CS pin
static HardwareSerial &kSbusUart = Serial2;                   // Serial port used for SBUS receive and servo command
const uint8_t kAnalogReadResolution = 16;                     // Resolution used for reading analog
const uint8_t kAnalogPins[2] = {14,16};                       // Array of pins for analog measurement
const uint8_t kInputVoltagePin = 15;                          // Pin used for measuring system input voltage
const uint8_t kRegulatedVoltagePin = A22;                     // Pin used for measuring system regulated voltage
const uint8_t kSbusVoltagePin = A21;                          // Pin used for measuring SBUS servo voltage
const uint8_t kPwmVoltagePin = 39;                            // Pin used for measuring PWM servo voltage
const float kInputVoltageScale = 11.0f;                       // Scale factor for converting measured to input input voltage
const float kRegulatedVoltageScale = 2.0f;                    // Scale factor for converting measured to input regulated voltage
const float kEffectorVoltageScale = 3.0f;                     // Scale factor for converting measured to input servo voltage

// Effectors
const uint8_t kPwmPins[8] = {21,22,23,2,3,4,5,6};             // Array of pins for PWM output
const float kPwmFrequency = 50;                               // PWM frequency, Hz
const float kPwmResolution = 16;                              // PWM resolution, bits

