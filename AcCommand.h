#ifndef ACCOMMAND_H
#define ACCOMMAND_H
#include <Arduino.h>
#define MAX_CODE_LENGTH 325


String encodeTemp(uint8_t temp);
String encodeMode(uint8_t mode);
String encodeFanMode(uint8_t fan_mode);
String encodeSwingMode(uint8_t swing_mode);
String encodeSwingAuto(bool swing_auto);
String encodePower(bool swing_auto);
String reverseBit(String value);
String encodeMitsubishiIR(bool swing_auto, uint8_t temp, uint8_t mode, bool power, uint8_t fan_mode, uint8_t swing_mode);
void converToRawCode(String str, uint16_t *result);

#endif // ACCOMMAND_H