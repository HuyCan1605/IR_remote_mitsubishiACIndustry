#include "AcCommand.h"
#include <Arduino.h>
#define BIT_MARK 490      // Độ dài tín hiệu thấp
#define ONE_SPACE 3500    // Độ dài tín hiệu cao
#define ZERO_SPACE 1400   // Sai lệch cho phép
#define HEADER_MARK 6000  // Giá trị đầu
#define HEADER_SPACE 7500 // Giá trị tiếp theo
#define FOOTER1 460
#define FOOTER2 7500

String kMitsubishiIndusHeader = "10110000";
String kMitsubishiIndusTurnSwingAuto = "00000010";
String kMitsubishiIndusTemp18 = "1100";
String kMitsubishiIndusTemp19 = "0100";
String kMitsubishiIndusTemp20 = "0010";
String kMitsubishiIndusTemp21 = "1010";
String kMitsubishiIndusTemp22 = "0110";
String kMitsubishiIndusTemp23 = "1110";
String kMitsubishiIndusTemp24 = "0001";
String kMitsubishiIndusTemp25 = "1001";
String kMitsubishiIndusTemp26 = "0101";
String kMitsubishiIndusTemp27 = "1101";
String kMitsubishiIndusTemp28 = "0011";
String kMitsubishiIndusTemp29 = "1011";
String kMitsubishiIndusTemp30 = "0111";

String kMitsubishiIndusAuto = "000";
String kMitsubishiIndusCool = "010";
String kMitsubishiIndusHeat = "001";
String kMitsubishiIndusDry = "100";
String kMitsubishiIndusFanOnly = "110";

String kMitsubishiIndusFanAuto = "001";
String kMitsubishiIndusFanLow = "000";
String kMitsubishiIndusFanMed = "100";
String kMitsubishiIndusFanHigh = "010";
String kMitsubishiIndusFanMax = "110";

String kMitsubishiIndusSwingMax = "0000";
String kMitsubishiIndusSwingHigh = "1000";
String kMitsubishiIndusSwingMed = "0100";
String kMitsubishiIndusSwingLow = "1100";
String kMitsubishiIndusSwingAuto = "1100";


String encodeTemp(uint8_t temp)
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

String encodeMode(uint8_t mode)
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

String encodeFanMode(uint8_t fan_mode)
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

String encodeSwingMode(uint8_t swing_mode)
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

String encodeSwingAuto(bool swing_auto)
{
    return swing_auto ? "00000010" : "00000000";
}

String encodePower(bool swing_auto)
{
    return swing_auto ? "1" : "0";
}

String reverseBit(String value)
{
    String result = ""; // Chuỗi kết quả khởi tạo rỗng
    // Duyệt từ cuối về đầu chuỗi và thêm từng ký tự vào chuỗi kết quả
    for (int i = 0; i < value.length(); ++i)
    {
        if (value[i] == '1')
        {
            result += '0'; // Nếu là '1' thì thêm '0' vào chuỗi kết quả
        }
        else
        {
            result += '1'; // Nếu là '0' thì thêm '1' vào chuỗi kết quả
        }
    }

    return result; // Trả về chuỗi kết quả đã đảo ngược và thay đổi
}

String encodeMitsubishiIR(bool swing_auto, uint8_t temp, uint8_t mode, bool power, uint8_t fan_mode, uint8_t swing_mode)
{
    String result = "";
    // mã hoá cho từng tham số
    result += kMitsubishiIndusHeader;      // 8
    result += encodeSwingAuto(swing_auto); // 8
    result += encodeTemp(temp);            // 4
    result += encodeMode(mode);            // 3
    result += encodePower(power);          // 1
    result += "0000";
    result += encodeSwingMode(swing_mode);
    result += reverseBit(kMitsubishiIndusHeader);
    result += reverseBit(encodeSwingAuto(swing_auto));
    result += reverseBit(encodeTemp(temp));
    result += reverseBit(encodeMode(mode));
    result += reverseBit(encodePower(power));
    result += "1111";
    result += reverseBit(encodeSwingMode(swing_mode));
    result += encodeFanMode(fan_mode);
    result += "00000";
    result += "10000000";
    result += "00101000";
    result += "00000000";
    result += reverseBit(encodeFanMode(fan_mode));
    result += "11111";
    result += "01111111";
    result += "11010111";
    result += "11111111";
    result += "01000000";
    result += "10111111";
    result += "00000000";
    result += "00000000";
    return result;
}

void converToRawCode(String str, uint16_t *result)
{
    result[0] = HEADER_MARK;
    result[1] = HEADER_SPACE;
    result[324] = FOOTER1;
    result[323] = FOOTER2;
    result[322] = FOOTER1;
    int i;
    for (i = 0; i < str.length(); ++i)
    {
        if (str[i] == '1')
        {
            result[2 + i * 2] = BIT_MARK;
            result[2 + i * 2 + 1] = ONE_SPACE;
        }
        else if (str[i] == '0')
        {
            result[2 + i * 2] = BIT_MARK;
            result[2 + i * 2 + 1] = ZERO_SPACE;
        }
    }
}