#include "AcCommand.h"
#include <Arduino.h>
#define BIT_MARK 490      // Độ dài tín hiệu thấp
#define ONE_SPACE 3500    // Độ dài tín hiệu cao
#define ZERO_SPACE 1400   // Sai lệch cho phép
#define HEADER_MARK 6000  // Giá trị đầu
#define HEADER_SPACE 7500 // Giá trị tiếp theo
#define FOOTER1 460
#define FOOTER2 7500

const uint8_t kMitsubishiIndusHeader = 0b10110000;

const uint8_t kMitsubishiIndusTemp18 = 0b0100;
const uint8_t kMitsubishiIndusTemp19 = 0b1100;
const uint8_t kMitsubishiIndusTemp20 = 0b0010;
const uint8_t kMitsubishiIndusTemp21 = 0b1010;
const uint8_t kMitsubishiIndusTemp22 = 0b0110;
const uint8_t kMitsubishiIndusTemp23 = 0b1110;
const uint8_t kMitsubishiIndusTemp24 = 0b0001;
const uint8_t kMitsubishiIndusTemp25 = 0b1001;
const uint8_t kMitsubishiIndusTemp26 = 0b0101;
const uint8_t kMitsubishiIndusTemp27 = 0b1101;
const uint8_t kMitsubishiIndusTemp28 = 0b0011;
const uint8_t kMitsubishiIndusTemp29 = 0b1011;
const uint8_t kMitsubishiIndusTemp30 = 0b0111;

const uint8_t kMitsubishiIndusAuto = 0b000;
const uint8_t kMitsubishiIndusCool = 0b010;
const uint8_t kMitsubishiIndusHeat = 0b001;
const uint8_t kMitsubishiIndusDry = 0b100;
const uint8_t kMitsubishiIndusFanOnly = 0b110;

const uint8_t kMitsubishiIndusFanAuto = 0b001;
const uint8_t kMitsubishiIndusFanLow = 0b000;
const uint8_t kMitsubishiIndusFanMed = 0b100;
const uint8_t kMitsubishiIndusFanHigh = 0b010;
const uint8_t kMitsubishiIndusFanMax = 0b110;

const uint8_t kMitsubishiIndusSwingAuto = 0b1100;
const uint8_t kMitsubishiIndusSwingLow = 0b0000;
const uint8_t kMitsubishiIndusSwingMed = 0b1000;
const uint8_t kMitsubishiIndusSwingHigh = 0b0100;
const uint8_t kMitsubishiIndusSwingMax = 0b1100;

const uint8_t NoN1 = 0b10000000;
const uint8_t NoN2 = 0b00101000;
const uint8_t NoN3 = 0b00000000;
const uint8_t NoN4 = 0b01000000;

uint8_t encodeTemp(uint8_t temp)
{
    switch (temp)
    {
    case 18:
        return kMitsubishiIndusTemp18;
    case 19:
        return kMitsubishiIndusTemp19;
    case 20:
        return kMitsubishiIndusTemp20;
    case 21:
        return kMitsubishiIndusTemp21;
    case 22:
        return kMitsubishiIndusTemp22;
    case 23:
        return kMitsubishiIndusTemp23;
    case 24:
        return kMitsubishiIndusTemp24;
    case 25:
        return kMitsubishiIndusTemp25;
    case 26:
        return kMitsubishiIndusTemp26;
    case 27:
        return kMitsubishiIndusTemp27;
    case 28:
        return kMitsubishiIndusTemp28;
    case 29:
        return kMitsubishiIndusTemp29;
    case 30:
        return kMitsubishiIndusTemp30;
    default:
        return kMitsubishiIndusTemp28;
    }
}

uint8_t encodeMode(uint8_t mode)
{
    switch (mode)
    {
    case 1:
        return kMitsubishiIndusCool;
    case 2:
        return kMitsubishiIndusHeat;
    case 3:
        return kMitsubishiIndusDry;
    case 4:
        return kMitsubishiIndusFanOnly;
    default:
        return kMitsubishiIndusAuto;
    }
}

uint8_t encodeFanMode(uint8_t fan_mode)
{
    switch (fan_mode)
    {
    case 1:
        return kMitsubishiIndusFanLow;
    case 2:
        return kMitsubishiIndusFanMed;
    case 3:
        return kMitsubishiIndusFanHigh;
    case 4:
        return kMitsubishiIndusFanMax;
    default:
        return kMitsubishiIndusFanAuto;
    }
}

uint8_t encodeSwingMode(uint8_t swing_mode)
{
    switch (swing_mode)
    {
    case 1:
        return kMitsubishiIndusSwingLow;
    case 2:
        return kMitsubishiIndusSwingMed;
    case 3:
        return kMitsubishiIndusSwingHigh;
    case 4:
        return kMitsubishiIndusSwingMax;
    default:
        return kMitsubishiIndusSwingAuto;
    }
}

uint8_t encodePower(bool power)
{
    return power ? 1 : 0;
}

uint8_t encodeSwingAuto(bool swing_auto)
{
    return swing_auto ? 0b00000010 : 0b00000000;
}

uint8_t inverseBit8(uint8_t value)
{
    return ~value;
}
uint32_t inverseBit32(uint32_t value)
{
    return ~value;
}

// encodeMitsubishiIR(data, 30, 1, 2, 1, 2, 0);  0111                             001         1             100                 1100
void encodeMitsubishiIR(uint32_t *result, uint8_t temp, bool swing_auto, uint8_t mode, bool power, uint8_t fan_mode, uint8_t swing_mode)
{
    uint8_t eTemp = encodeTemp(temp);
    uint8_t eSwing_auto = encodeSwingAuto(swing_auto);
    uint8_t eMode = encodeMode(mode);
    uint8_t ePower = encodePower(power);
    uint8_t eFan_mode = encodeFanMode(fan_mode);
    uint8_t eSwing_mode = encodeSwingMode(swing_mode);

    uint8_t rTemp = inverseBit8(eTemp);
    uint8_t rSwing_auto = inverseBit8(eSwing_auto);
    uint8_t rMode = inverseBit8(eMode);
    uint8_t rFan_mode = inverseBit8(eFan_mode);
    uint8_t rSwing_mode = inverseBit8(eSwing_mode);
    Serial.print("eTemp: ");
    Serial.print(String(eTemp, BIN));
    Serial.print(" | rTemp: ");
    Serial.print(String(rTemp, BIN));
    Serial.println();

    Serial.print("eSwing_auto: ");
    Serial.print(String(eSwing_auto, BIN));
    Serial.print(" | rSwing_auto: ");
    Serial.print(String(rSwing_auto, BIN));
    Serial.println();

    Serial.print("eMode: ");
    Serial.print(String(eMode, BIN));
    Serial.print(" | rMode: ");
    Serial.print(String(rMode, BIN));
    Serial.println();

    Serial.print("eFan_mode: ");
    Serial.print(String(eFan_mode, BIN));
    Serial.print(" | rFan_mode: ");
    Serial.print(String(rFan_mode, BIN));
    Serial.println();

    Serial.print("eSwing_mode: ");
    Serial.print(String(eSwing_mode, BIN));
    Serial.print(" | rSwing_mode: ");
    Serial.print(String(rSwing_mode, BIN));
    Serial.println();

    result[0] = ((uint32_t)kMitsubishiIndusHeader << 24) 
    | ((uint32_t)eSwing_auto << 16) // 00000010
    | ((uint32_t)eTemp << 12)                                                // 0111
    | ((uint32_t)eMode << 9)                                                 // 011
    | ((uint32_t)ePower << 8)                                                // 1
    | (0b0000 << 4) 
    | (eSwing_mode);                                         // 1100
    Serial.print("result[0]: ");
    Serial.print(String(result[0], BIN));
    Serial.println();

    result[1] = inverseBit32(result[0]);
    Serial.print("result[1]: ");
    Serial.print(String(result[1], BIN));
    Serial.println();

    result[2] = ((uint32_t)eFan_mode << 29) // 100 -> 001
                | (0b00000 << 24) 
                | ((uint32_t)NoN1 << 16) 
                | (NoN2 << 8) 
                | (NoN3);
    Serial.print("result[2]: ");
    Serial.print(String(result[2], BIN));
    Serial.println();

    result[3] = inverseBit32(result[2]);
    Serial.print("result[3]: ");
    Serial.print(String(result[3], BIN));
    Serial.println();

    result[4] = ((uint32_t)NoN4 << 24) 
    | ((uint32_t)inverseBit8(NoN4) << 16) 
    | (0b00000000 << 8) 
    | (0b00000000);
    Serial.print("result[4]: ");
    Serial.print(String(result[4], BIN));
    Serial.println();
}
void convertToRawCode(uint32_t *irBit, uint16_t *result)
{
    int n = 325;
    result[0] = HEADER_MARK;
    result[1] = HEADER_SPACE;
    result[324] = FOOTER1;
    result[323] = FOOTER2;
    result[322] = FOOTER1;
    int index = 0;
    for (int i = 0; i < 5; i++)
    {
        for (int j = 31; j >= 0; --j)
        {
            if (((irBit[i] >> j) & 1) == 1)
            {
                result[2 + index * 2] = BIT_MARK;
                result[2 + index * 2 + 1] = ONE_SPACE;
            }
            else
            {
                result[2 + index * 2] = BIT_MARK;
                result[2 + index * 2 + 1] = ZERO_SPACE;
            }
            index++;
        }
    }
    Serial.println();
    for (int i = 0; i < n; i++)
    {
        Serial.print(result[i]);
        if (i < n - 1)
        {
            Serial.print(", ");
        }
    }
}

/**
 * Extracts the Temperature bits from the received IR data and converts them to human-readable format.
 * @param resultData: The string representing the raw bit data received from the IR signal.
 * @return uint8_t: The parsed temperature value in binary.
 */
uint8_t getTemp(String resultData)
{
    String result = resultData.substring(16, 20);
    return (uint8_t)strtol(result.c_str(), NULL, 2);
}
// Hàm getMode trích xuất đoạn bit dài của IRremote và chuyển đổi thông tin từ chuỗi bit liên quan đến Mode
uint8_t getMode(String resultData)
{
    String result = resultData.substring(20, 23);
    return (uint8_t)strtol(result.c_str(), NULL, 2);
}

// Hàm getPower trích xuất đoạn bit dài của IRremote và chuyển đổi thông tin từ chuỗi bit liên quan đến bật/tắt
uint8_t getPower(String resultData)
{
    String result = resultData.substring(23, 24);
    return (uint8_t)strtol(result.c_str(), NULL, 2);
}
// Hàm getSwingMode trích xuất đoạn bit dài của IRremote và chuyển đổi thông tin từ chuỗi bit liên quan đến độ mở cách gió
uint8_t getSwingMode(String resultData)
{
    String result = resultData.substring(28, 32);
    return (uint8_t)strtol(result.c_str(), NULL, 2);
}

// Hàm getFanMode trích xuất đoạn bit dài của IRremote và chuyển đổi thông tin từ chuỗi bit liên quan đến độ mạnh của quạt
uint8_t getFanMode(String resultData)
{
    String result = resultData.substring(64, 67);
    return (uint8_t)strtol(result.c_str(), NULL, 2);
}
uint8_t getSwingAuto(String resultData)
{
    String result = resultData.substring(14, 15);
    return (uint8_t)strtol(result.c_str(), NULL, 2);
}

int abs(int val)
{
    return (val < 0) ? -val : val;
}

char decode(int val1, int val2)
{
    if (val1 != HEADER_MARK && val2 != HEADER_SPACE)
    {
        if (abs(val1 - BIT_MARK) < ZERO_SPACE && abs(val2 - BIT_MARK) < ZERO_SPACE)
        {
            return '0';
        }
        if (abs(val1 - BIT_MARK) < ZERO_SPACE && abs(val2 - ONE_SPACE) < ZERO_SPACE)
        {
            return '1';
        }
    }
    return '\0';
}

String turnRawSignalToBinary(String rawData)
{
    char rawDataArray[rawData.length()]; // Thêm ký tự null cuối
    const char* delimiters = ", ";
    int firstValue, secondValue;

    strcpy(rawDataArray, rawData.c_str());
    char *token = strtok(rawDataArray, delimiters);
    String binaryResult = "";  // Sử dụng `String` thay cho mảng ký tự
    int tokenIndex = 0;

    while (token != NULL)
    {
        if (tokenIndex & 1)
        {
            secondValue = atoi(token);
            char decodedChar = decode(firstValue, secondValue);
            if (decodedChar != '\0') {
                binaryResult += decodedChar;  // Nối thẳng vào `String`
            }
        }
        else
        {
            firstValue = atoi(token);
        }
        token = strtok(NULL, delimiters);
        tokenIndex++;
    }
    Serial.println();
    Serial.print(binaryResult);
    return binaryResult;  // Trả về kết quả chuỗi nhị phân
}

/**
 * Publishes the parsed Auto Swing and Temperature data to Home Assistant via MQTT.
 */
const char* returnTempForMQTT(String resultData)
{
  // Serial.print("Temperature: "); Serial.println(getTemp(resultData), BIN);
    switch (getTemp(resultData))
    {
    case kMitsubishiIndusTemp18:
        return "18";
    case kMitsubishiIndusTemp19:
        return "19";
    case kMitsubishiIndusTemp20:
        return "20";
    case kMitsubishiIndusTemp21:
        return "21";
    case kMitsubishiIndusTemp22:
        return "22";
    case kMitsubishiIndusTemp23:
        return "23";
    case kMitsubishiIndusTemp24:
        return "24";
    case kMitsubishiIndusTemp25:
        return "25";
    case kMitsubishiIndusTemp26:
        return "26";
    case kMitsubishiIndusTemp27:
        return "27";
    case kMitsubishiIndusTemp28:
        return "28";
    case kMitsubishiIndusTemp29:
        return "29";
    case kMitsubishiIndusTemp30:
        return "30";
    default:
        return "";
    }
}

const char* returnModeForMQTT(String resultData)
{
  // Serial.print("Mode: "); Serial.println(getMode(resultData), BIN);
    switch (getMode(resultData))
    {
    case kMitsubishiIndusAuto:
        return "auto";
    case kMitsubishiIndusCool:
        return "cool";
    case kMitsubishiIndusHeat:
        return "heat";
    case kMitsubishiIndusDry:
        return "dry";
    case kMitsubishiIndusFanOnly:
        return "fan_only";
    default:
        return "";
    }
}
const char* returnSwingModeForMQTT(String resultData)
{
  // Serial.print("Swing: "); Serial.println(getSwingMode(resultData), BIN);
    switch (getSwingMode(resultData))
    {
    case kMitsubishiIndusSwingHigh:
        return "High";
    case kMitsubishiIndusSwingMed:
        return "Medium";
    case kMitsubishiIndusSwingMax:
        return "Max";
    case kMitsubishiIndusSwingLow:
        return "Low";
    default:
        return "";
    }
}
const char* returnPowerForMQTT(String resultData)
{
  // Serial.print("Power: "); Serial.println(getPower(resultData), BIN);
    switch (getPower(resultData))
    {
    case 0:
        return "off";
    default:
        return "";
    }
}

const char* returnFanModeForMQTT(String resultData) 
{
  // Serial.print("Fan: "); Serial.println(getFanMode(resultData), BIN);
    switch (getFanMode(resultData))
    {
    case kMitsubishiIndusFanAuto:
        return "auto";
    case kMitsubishiIndusFanMax:
        return "Max";
    case kMitsubishiIndusFanHigh:
        return "high";
    case kMitsubishiIndusFanMed:
        return "medium";
    case kMitsubishiIndusFanLow:
        return "low";
    default:
        return "";
    }
}

const char* returnSwingAutoForMQTT(String resultData)
{
    switch (getSwingAuto(resultData))
    {
    case 1:
        return "Auto";
    default:
        return "";
    }
}
