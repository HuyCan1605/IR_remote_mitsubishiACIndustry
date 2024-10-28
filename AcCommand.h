#ifndef ACCOMMAND_H
#define ACCOMMAND_H
#include <Arduino.h>
#include <stdint.h>

#define MAX_CODE_LENGTH 325
#define MAX_CODE_BIT_LENGTH 160

uint8_t encodeTemp(uint8_t temp);
uint8_t encodeMode(uint8_t mode);
uint8_t encodeFanMode(uint8_t fan_mode);
uint8_t encodeSwingMode(uint8_t swing_mode);
uint8_t encodeSwingAuto(bool swing_auto);
uint8_t encodePower(bool swing_auto);
uint8_t inverseBit8(uint8_t value);
uint32_t inverseBit32(uint32_t value);
const char* returnTempForMQTT(String resultData);
const char* returnModeForMQTT(String resultData);
const char* returnSwingModeForMQTT(String resultData);
const char* returnPowerForMQTT(String resultData);
const char* returnFanModeForMQTT(String resultData);
const char* returnSwingAutoForMQTT(String resultData);
String turnRawSignalToBinary(String rawData);
void encodeMitsubishiIR(uint32_t *result, uint8_t temp, bool swing_auto, uint8_t mode, bool power, uint8_t fan_mode, uint8_t swing_mode);
void convertToRawCode(uint32_t *irBit, uint16_t *result);

#endif // ACCOMMAND_H