#include <Arduino.h>
#include <IRsend.h>
#include "AcCommand.h"

const uint16_t kIrLed = 4; // ESP8266 GPIO pin to use. Recommended: 4 (D2).
IRsend irsend(kIrLed);     // Set the GPIO to be used to sending the message.
uint16_t raw_data;
uint8_t temp;
uint8_t mode;
uint8_t fan_mode;
uint8_t swing_mode;
bool swing_auto;
bool power;
uint16_t rawData[MAX_CODE_LENGTH];


void setup()
{
    irsend.begin();
#if ESP8266
    Serial.begin(115200, SERIAL_8N1, SERIAL_TX_ONLY);
#else  // ESP8266
    Serial.begin(115200, SERIAL_8N1);
#endif // ESP8266
    Serial.begin(115200);

    swing_auto = false;
    temp = 30;
    mode = 1;
    power = false;
    fan_mode = 3;
    swing_mode = 1;
    converToRawCode(encodeMitsubishiIR(swing_auto, temp, mode, power, fan_mode, swing_mode), rawData);
    for(int i = 0; i < MAX_CODE_LENGTH; i++){
        Serial.print(rawData[i]);
        if(i < MAX_CODE_LENGTH - 1){
            Serial.print(",");
        }
    }
}




String bitRaw44 = "1011000000000000001101010000000001001111111111111100101011111111100000001000000000101000000000000111111101111111110101111111111101000000101111110000000000000000";
uint16_t raw_data2[MAX_CODE_LENGTH] = {6016, 7510, 482, 3504, 484, 1508, 482, 3504, 484, 3504, 484, 1506, 484, 1506, 484, 1506, 484, 1506, 484, 1506, 484, 1506, 484, 1506, 484, 1508, 482, 1506, 484, 1506, 484, 1506, 484, 1506, 484, 1508, 484, 1506, 484, 3502, 484, 3502, 484, 1506, 484, 3504, 484, 1506, 484, 3504, 484, 1506, 484, 1506, 484, 1506, 482, 1508, 482, 1508, 484, 1506, 484, 1508, 482, 1508, 482, 1506, 484, 3502, 484, 1508, 484, 1506, 482, 3504, 482, 3506, 484, 3504, 482, 3504, 482, 3506, 482, 3504, 484, 3504, 482, 3504, 482, 3504, 484, 3504, 482, 3504, 482, 3504, 482, 3504, 482, 3504, 482, 1508, 482, 1506, 482, 3504, 482, 1508, 482, 3504, 484, 1506, 484, 3504, 482, 3504, 482, 3504, 482, 3504, 482, 3506, 482, 3504, 482, 3504, 482, 3506, 482, 3504, 482, 1508, 482, 1506, 482, 1508, 482, 1508, 482, 1508, 482, 1508, 482, 1508, 482, 3506, 482, 1508, 482, 1508, 482, 1508, 482, 1508, 482, 1508, 482, 1508, 482, 1508, 482, 1508, 482, 1508, 482, 3506, 482, 1508, 482, 3506, 482, 1508, 482, 1508, 482, 1508, 482, 1508, 482, 1508, 482, 1508, 482, 1508, 482, 1508, 482, 1508, 482, 1508, 482, 1508, 482, 1508, 482, 3504, 482, 3506, 482, 3506, 482, 3506, 482, 3506, 482, 3506, 480, 3506, 482, 1510, 480, 3506, 480, 3506, 482, 3506, 480, 3506, 482, 3506, 482, 3506, 482, 3506, 482, 3504, 480, 3508, 480, 1510, 480, 3506, 458, 1534, 480, 3506, 482, 3506, 458, 3528, 458, 3530, 458, 3530, 480, 3506, 458, 3530, 456, 3530, 458, 3528, 458, 3528, 458, 3530, 458, 1532, 458, 3530, 458, 1532, 458, 1532, 458, 1532, 458, 1532, 458, 1532, 458, 1532, 458, 3530, 458, 1532, 458, 3530, 458, 3530, 456, 3530, 458, 3530, 458, 3528, 458, 3530, 456, 1532, 458, 1534, 456, 1532, 458, 1532, 458, 1534, 456, 1534, 458, 1534, 456, 1532, 458, 1532, 458, 1532, 458, 1534, 456, 1534, 458, 1532, 456, 1534, 456, 1534, 458, 1532, 458, 7526, 456};
void loop()
{
    
    // Serial.println("Raw data send with temp 30 ");
    // irsend.sendRaw(result, MAX_CODE_LENGTH, 38); // Send a raw data capture at 38kHz.
    // delay(4000);

    // swing_auto = false;
    // temp = 29;
    // mode = 1;
    // power = false;
    // fan_mode = 3;
    // swing_mode = 1;
    // converToRawCode(encodeMitsubishiIR(swing_auto, temp, mode, power, fan_mode, swing_mode), rawData);
    // Serial.println("Raw data send with temp 29 ");
    // irsend.sendRaw(result, MAX_CODE_LENGTH, 38); // Send a raw data capture at 38kHz.
    // delay(4000);

    // swing_auto = false;
    // temp = 28;
    // mode = 1;
    // power = false;
    // fan_mode = 3;
    // swing_mode = 1;
    // converToRawCode(encodeMitsubishiIR(swing_auto, temp, mode, power, fan_mode, swing_mode), rawData);
    // Serial.println("Raw data send with temp 28 ");
    // irsend.sendRaw(result, MAX_CODE_LENGTH, 38); // Send a raw data capture at 38kHz.
    // delay(4000);

    // swing_auto = false;
    // temp = 27;
    // mode = 1;
    // power = false;
    // fan_mode = 3;
    // swing_mode = 1;
    // converToRawCode(encodeMitsubishiIR(swing_auto, temp, mode, power, fan_mode, swing_mode), rawData);
    // Serial.println("Raw data send with temp 27 ");
    // irsend.sendRaw(result, MAX_CODE_LENGTH, 38); // Send a raw data capture at 38kHz.
    // delay(4000);

    // swing_auto = false;
    // temp = 26;
    // mode = 1;
    // power = false;
    // fan_mode = 3;
    // swing_mode = 1;
    // converToRawCode(encodeMitsubishiIR(swing_auto, temp, mode, power, fan_mode, swing_mode), rawData);
    // Serial.println("Raw data send with temp 26 ");
    // irsend.sendRaw(result, MAX_CODE_LENGTH, 38); // Send a raw data capture at 38kHz.
    // delay(4000);

    // swing_auto = false;
    // temp = 25;
    // mode = 1;
    // power = false;
    // fan_mode = 3;
    // swing_mode = 1;
    // converToRawCode(encodeMitsubishiIR(swing_auto, temp, mode, power, fan_mode, swing_mode), rawData);
    // Serial.println("Raw data send with temp 25 ");
    // irsend.sendRaw(result, MAX_CODE_LENGTH, 38); // Send a raw data capture at 38kHz.
    // delay(4000);

    // converToRawCode(bitRaw44, rawData);
    // Serial.println("Raw data send with raw bit ");
    // irsend.sendRaw(result, MAX_CODE_LENGTH, 38); // Send a raw data capture at 38kHz.
    // delay(4000);

    // Serial.println("Raw data send with raw IR code ");
    // irsend.sendRaw(raw_data2, MAX_CODE_LENGTH, 38); // Send a raw data capture at 38kHz.
    // delay(4000);
}
