#include <FS.h>
#include <Arduino.h>
#include <IRsend.h>
#include "AcCommand.h"
// #include <DHT.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <WiFiManager.h>  // Thư viện để quản lý kết nối WiFi
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <IRrecv.h>
#include <assert.h>
#include "webConfigWifiAndMQTT.h"
#include <ArduinoJson.h>
#include <IRutils.h>

// #define DHTTYPE DHT11 // Định nghĩa loại cảm biến DHT là DHT11

// IR + dht + human sensor pin

const uint32_t kBaudRate = 115200;
const uint16_t kCaptureBufferSize = 1024;
const uint16_t kIrLed = 4;     // Pin GPIO cho LED IR (ESP8266 GPIO 4 - D2).
const uint16_t kRecvPin = 14;  // Pin GPIO cho IR Receiver (D5)
unsigned long lastReconnectAttempt = 0;
struct WifiSettings {
  char apName[20] = "ESP8266-config";
  char apPassword[20];
  char SSID[20];
  char oldSSID[20];
  char password[20];
  char oldPassword[20];
  char wm_old_wifi_ssid_identifier[9] = "old_ssid";
  char wm_old_wifi_password_identifier[18] = "old_wifi_password";
  char wm_wifi_ssid_identifier[5] = "ssid";
  char wm_wifi_password_identifier[14] = "wifi_password";
};

struct MQTTSettings {
  char clientId[20];
  char hostname[40];
  char port[6];
  char user[20];
  char password[20];
  char wm_mqtt_client_id_identifier[15] = "mqtt_client_id";
  char wm_mqtt_hostname_identifier[14] = "mqtt_hostname";
  char wm_mqtt_port_identifier[10] = "mqtt_port";
  char wm_mqtt_user_identifier[10] = "mqtt_user";
  char wm_mqtt_password_identifier[14] = "mqtt_password";
};

#if DECODE_AC
const uint8_t kTimeout = 50;
#else   // DECODE_AC
const uint8_t kTimeout = 15;
#endif  // DECODE_AC
const uint16_t kMinUnknownSize = 12;
const uint8_t kTolerancePercentage = kTolerance;  // kTolerance is normally 25%
IRrecv irrecv(kRecvPin, kCaptureBufferSize, kTimeout, true);
decode_results results;  // Somewhere to store the results

// const uint8_t DHTPin = 2;     // Pin GPIO cho cảm biến DHT (D4).
// const uint8_t ld2410_pin = 5; // Pin GPIO cho cảm biến chuyển động (D1).

// save config to file
bool shouldSaveConfig = false;
bool checkWiFiSaveInFile = false;

WifiSettings wifisettings;
MQTTSettings mqttsettings;
WiFiClient espClient;                 // Tạo một client WiFi.
PubSubClient mqtt_client(espClient);  // Tạo một client MQTT với client WiFi.

WiFiManager wifiManager;  // Tạo một instance của WiFiManager.

// Web Server
ESP8266WebServer webServer(80);  // Tạo một server web trên cổng 80.

// DHT dht(DHTPin, DHTTYPE); // Khởi tạo cảm biến DHT với pin và loại đã định nghĩa.
IRsend irsend(kIrLed);  // Khởi tạo IRsend với pin LED IR đã định nghĩa.

void readSettingsFromConfig() {
  //clean FS for testing
  //  SPIFFS.format();
  // Đọc cấu hình từ SPIFFS
  Serial.println("Mounting FS...");

  if (SPIFFS.begin()) {
    Serial.println("Mounted file system");
    if (SPIFFS.exists("/config.json")) {
      Serial.println("Reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("Opened config file");
        size_t size = configFile.size();

        if (size > 0)  // Kiểm tra file không rỗng
        {
          std::unique_ptr<char[]> buf(new char[size + 1]);  // +1 cho ký tự null terminator
          configFile.readBytes(buf.get(), size);
          buf[size] = '\0';  // Đảm bảo chuỗi kết thúc với '\0'

          StaticJsonDocument<1024> doc;
          DeserializationError error = deserializeJson(doc, buf.get());
          if (error) {
            Serial.println(F("Failed to load JSON config"));
          } else {
            Serial.println("\nParsed JSON successfully");

            // Đọc thông tin từ JSON và sao chép vào cấu trúc
            strncpy(mqttsettings.clientId, doc[mqttsettings.wm_mqtt_client_id_identifier], sizeof(mqttsettings.clientId));
            strncpy(mqttsettings.hostname, doc[mqttsettings.wm_mqtt_hostname_identifier], sizeof(mqttsettings.hostname));
            strncpy(mqttsettings.port, doc[mqttsettings.wm_mqtt_port_identifier], sizeof(mqttsettings.port));
            strncpy(mqttsettings.user, doc[mqttsettings.wm_mqtt_user_identifier], sizeof(mqttsettings.user));
            strncpy(mqttsettings.password, doc[mqttsettings.wm_mqtt_password_identifier], sizeof(mqttsettings.password));
            strcpy(wifisettings.SSID, doc[wifisettings.wm_wifi_ssid_identifier]);
            strcpy(wifisettings.password, doc[wifisettings.wm_wifi_password_identifier]);
            strcpy(wifisettings.oldSSID, doc[wifisettings.wm_old_wifi_ssid_identifier]);
            strcpy(wifisettings.oldPassword, doc[wifisettings.wm_old_wifi_password_identifier]);
            Serial.print("Do dai cua ssid trong doc: ");
            Serial.println(strlen(wifisettings.SSID));
            Serial.print("Do dai cua wifi passs trong doc: ");
            Serial.println(strlen(wifisettings.SSID));
            if (strlen(wifisettings.SSID) != 0 && strlen(wifisettings.SSID) != 0) {
              checkWiFiSaveInFile = true;
            }
          }
        } else {
          Serial.println("Config file is empty");
        }

        configFile.close();  // Đóng file
      } else {
        Serial.println("Failed to open config file");
      }
    } else {
      Serial.println("Config file does not exist");
    }
  } else {
    Serial.println("Failed to mount file system");
  }
}
void mainpage() {
  String htmlWebSite = MainPage;
  webServer.send(200, "text/html", htmlWebSite);
}

void mqttCallBack(char *topic, byte *payload, unsigned int length) {
  // Hàm gọi lại khi nhận được tin nhắn từ MQTT.
  String reveiceMessage = "";
  Serial.print("Message received on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");

  // Chuyển đổi payload thành chuỗi để sử dụng.
  for (int i = 0; i < length; i++) {
    reveiceMessage += (char)payload[i];
  }

  Serial.println();
  Serial.print(reveiceMessage);
  Serial.println();
  Serial.println("------------------------");
  getTopicAndMessage(topic, reveiceMessage);  // Gọi hàm xử lý tin nhắn.
}
void initializeMqttClient() {
  Serial.println("Local IP:");
  Serial.println(WiFi.localIP());

  mqtt_client.setServer(mqttsettings.hostname, atoi(mqttsettings.port));
  mqtt_client.setCallback(mqttCallBack);
}

void connectToMQTTBroker() {
  // Hàm kết nối đến MQTT Broker.
  if (!mqtt_client.connected()) {
    if (millis() - lastReconnectAttempt > 5000) {
      lastReconnectAttempt = millis();
      String client_id = "esp8266-client-" + String(WiFi.macAddress());
      Serial.printf("Connecting to MQTT Broker as %s...... \n", client_id.c_str());
      if (mqtt_client.connect(client_id.c_str(), mqttsettings.user, mqttsettings.password)) {
        Serial.println("Connected to MQTT Broker");
        mqtt_client.subscribe("study/ac/mode");
        mqtt_client.subscribe("study/ac/temperature");
        mqtt_client.subscribe("study/ac/fan");
        mqtt_client.subscribe("study/ac/swing");
        lastReconnectAttempt = 0;
      } else {
        Serial.println("Connection failed");
        Serial.println(mqtt_client.state());
        Serial.println(" try again in 5 seconds");
        delay(5000);
      }
    }
  } else {
    mqtt_client.loop();
  }
}
void createAccessPoint() {
  // set timeout until configuration
  wifiManager.setConfigPortalTimeout(360);

  // set minium quality of signal
  //  wifiManager.setMinimumSignalQuality();
  if (!wifiManager.autoConnect(wifisettings.apName)) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    // reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  } else {
    strcpy(wifisettings.SSID, wifiManager.getWiFiSSID().c_str());
    strcpy(wifisettings.password, wifiManager.getWiFiPass().c_str());
  }
  Serial.println("WiFi connected!");
}
void initializeWiFiManager() {
  WiFiManagerParameter custom_mqtt_client("client_id", "MQTT Client ID", mqttsettings.clientId, 20);
  WiFiManagerParameter custom_mqtt_server("server", "MQTT Server", mqttsettings.hostname, 40);
  WiFiManagerParameter custom_mqtt_port("port", "MQTT Port", mqttsettings.port, 6);
  WiFiManagerParameter custom_mqtt_user("user", "MQTT User", mqttsettings.user, 20);
  WiFiManagerParameter custom_mqtt_pass("pass", "MQTT Password", mqttsettings.password, 20);

  // Reset wifi setting for testing
  //  wifiManager.resetSettings();

  wifiManager.setSaveConfigCallback(saveConfigCallBack);

  // add all your parameters here
  wifiManager.addParameter(&custom_mqtt_client);
  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_port);
  wifiManager.addParameter(&custom_mqtt_user);
  wifiManager.addParameter(&custom_mqtt_pass);

  //start auto create an Access point in the first time programmed
  createAccessPoint();

  strcpy(mqttsettings.clientId, custom_mqtt_client.getValue());
  strcpy(mqttsettings.hostname, custom_mqtt_server.getValue());
  strcpy(mqttsettings.port, custom_mqtt_port.getValue());
  strcpy(mqttsettings.user, custom_mqtt_user.getValue());
  strcpy(mqttsettings.password, custom_mqtt_pass.getValue());
}
void saveConfigCallBack() {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}
void saveConfig() {
  Serial.println("\n----------------------------------------");
  Serial.println("Saving config");
  StaticJsonDocument<1024> doc;
  doc[mqttsettings.wm_mqtt_client_id_identifier] = mqttsettings.clientId;
  doc[mqttsettings.wm_mqtt_hostname_identifier] = mqttsettings.hostname;
  doc[mqttsettings.wm_mqtt_port_identifier] = mqttsettings.port;
  doc[mqttsettings.wm_mqtt_user_identifier] = mqttsettings.user;
  doc[mqttsettings.wm_mqtt_password_identifier] = mqttsettings.password;
  doc[wifisettings.wm_wifi_ssid_identifier] = wifisettings.SSID;
  doc[wifisettings.wm_wifi_password_identifier] = wifisettings.password;
  doc[wifisettings.wm_old_wifi_ssid_identifier] = wifisettings.oldSSID;
  doc[wifisettings.wm_old_wifi_password_identifier] = wifisettings.oldPassword;

  Serial.print("Doc client id: ");
  Serial.println(doc[mqttsettings.wm_mqtt_client_id_identifier].as<String>());
  Serial.print("DOC hostname: ");
  Serial.println(doc[mqttsettings.wm_mqtt_hostname_identifier].as<String>());
  Serial.print("DOC mqtt user: ");
  Serial.println(doc[mqttsettings.wm_mqtt_user_identifier].as<String>());
  Serial.print("DOC mqtt password: ");
  Serial.println(doc[mqttsettings.wm_mqtt_password_identifier].as<String>());
  Serial.print("DOC ssid user: ");
  Serial.println(doc[wifisettings.wm_wifi_ssid_identifier].as<String>());
  Serial.print("DOC wifi password: ");
  Serial.println(doc[wifisettings.wm_wifi_password_identifier].as<String>());
  Serial.print("DOC old ssid user: ");
  Serial.println(doc[wifisettings.wm_old_wifi_ssid_identifier].as<String>());
  Serial.print("DOC old wifi password: ");
  Serial.println(doc[wifisettings.wm_old_wifi_password_identifier].as<String>());

  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile) {
    Serial.println("Failed to open config file for writing");
    return;  // Thoát khỏi hàm nếu không mở được file
  } else {
    Serial.println("Success to open config file for writing");
  }

  // Ghi dữ liệu JSON vào file
  if (serializeJson(doc, configFile) == 0) {
    Serial.println("Failed to write to config file");
  } else {
    Serial.println("Config file saved successfully");
  }
  configFile.close();
}


uint8_t temp = 19;        // Biến để lưu trữ giá trị nhiệt độ.
uint8_t mode = 1;         // Biến để lưu trữ chế độ hoạt động.
uint8_t fan_mode = 1;     // Biến để lưu trữ chế độ quạt.
uint8_t swing_mode = 1;   // Biến để lưu trữ chế độ đảo gió.
bool swing_auto = false;  // Biến để xác định chế độ đảo gió tự động.
bool power = false;       // Biến để xác định trạng thái nguồn (ON/OFF).

void getTopicAndMessage(char *topic, String reveice) {
  int count = 0;           // Biến đếm số lần xuất hiện ký tự '/' trong topic.
  String main_topic = "";  // Chuỗi lưu trữ phần chủ đề chính.

  // topic from HA: study/ac/temperature
  for (int i = 0; i < strlen(topic); i++) {
    if (topic[i] == '/') {
      count++;  // Tăng biến đếm mỗi khi gặp '/'.
    }
    // Khi đã gặp đủ 2 lần '/' và ký tự tiếp theo không phải là '\0'
    if (count == 2 && topic[i + 1] != '\0' && i + 1 < strlen(topic)) {
      main_topic += topic[i + 1];  // Lấy ký tự tiếp theo sau '/' thứ hai.
    }
  }

  // Xác định hành động dựa trên chủ đề chính.
  if (main_topic.equals("temperature")) {
    temp = reveice.toInt();  // Chuyển đổi dữ liệu nhận được thành số nguyên cho nhiệt độ.
  } else if (main_topic.equals("mode")) {
    mode = reveice.toInt();  // Chuyển đổi dữ liệu nhận được thành số nguyên cho chế độ.
  } else if (main_topic.equals("fan")) {
    fan_mode = reveice.toInt();  // Chuyển đổi dữ liệu nhận được thành số nguyên cho chế độ quạt.
  } else if (main_topic.equals("swing")) {
    swing_mode = reveice.toInt();  // Chuyển đổi dữ liệu nhận được thành số nguyên cho chế độ đảo gió.
  }

  // Xác định trạng thái nguồn và chế độ đảo gió tự động.
  power = (mode == 5) ? false : true;             // Nếu mode = 5 thì tắt nguồn (power = false).
  swing_auto = (swing_mode == 0) ? true : false;  // Nếu swing_mode = 0 thì chế độ đảo gió tự động (swing_auto = true).

  uint16_t rawData[MAX_CODE_LENGTH];  // Mảng lưu trữ dữ liệu thô từ cảm biến IR.
  Serial.println();
  Serial.print("Receive temp from MQTT:");
  Serial.println(temp);
  Serial.print("Receive mode from MQTT:");
  Serial.println(mode);
  Serial.print("Receive fan_mode from MQTT:");
  Serial.println(fan_mode);
  Serial.print("Receive swing_mode from MQTT:");
  Serial.println(swing_mode);
  Serial.print("Receive power from MQTT:");
  Serial.println(power);
  Serial.print("Receive swing_auto from MQTT:");
  Serial.println(swing_auto);
  encodeData(rawData);                           // Gọi hàm để mã hóa dữ liệu IR.
  irsend.sendRaw(rawData, MAX_CODE_LENGTH, 38);  // Gửi dữ liệu IR đã mã hóa.
}
void handleRoot() {
  String s = MainPage;
  webServer.send(200, "text/html", s);
}

void changeWiFiParameters() {
  String new_ssid = webServer.arg("ssid");
  String new_password = webServer.arg("password");

  Serial.print("SSID lấy từ web: ");
  Serial.println(new_ssid);
  Serial.print("Pass lấy từ web: ");
  Serial.println(new_password);

  strcpy(wifisettings.oldSSID, wifisettings.SSID);
  strcpy(wifisettings.oldPassword, wifisettings.password);
  new_ssid.toCharArray(wifisettings.SSID, sizeof(wifisettings.SSID));
  new_password.toCharArray(wifisettings.password, sizeof(wifisettings.password));

  Serial.print("SSID copy vào wifisettings: ");
  Serial.println(wifisettings.SSID);
  Serial.print("Pass copy vào wifisettings: ");
  Serial.println(wifisettings.password);
  Serial.print("SSID copy vào old_SSID: ");
  Serial.println(wifisettings.oldSSID);
  Serial.print("Pass copy vào old_password: ");
  Serial.println(wifisettings.oldPassword);
  // Đặt cờ lưu cấu hình và lưu nếu cần
  saveConfig();
  // Tạo phản hồi và gửi về client
  String response = FPSTR(wifiSuccessfulConnectionWeb);  // Load HTML từ PROGMEM
  webServer.send(200, "text/html", response);

  // Khởi động lại ESP8266 để áp dụng cài đặt WiFi mới
  delay(2000);
  ESP.restart();
}

void changeMQTTParameters() {
  String new_mqtt_hostname = webServer.arg("server");
  String new_mqtt_client_ID = webServer.arg("clientID");
  String new_mqtt_port = webServer.arg("port");
  String new_mqtt_user = webServer.arg("user");
  String new_mqtt_password = webServer.arg("pass");

  new_mqtt_client_ID.toCharArray(mqttsettings.clientId, sizeof(mqttsettings.clientId));
  new_mqtt_hostname.toCharArray(mqttsettings.hostname, sizeof(mqttsettings.hostname));
  new_mqtt_port.toCharArray(mqttsettings.port, sizeof(mqttsettings.port));
  new_mqtt_user.toCharArray(mqttsettings.user, sizeof(mqttsettings.user));
  new_mqtt_password.toCharArray(mqttsettings.password, sizeof(mqttsettings.password));

  // Đặt cờ lưu cấu hình và lưu nếu cần
  saveConfig();

  String response = FPSTR(mqttSuccessfulConnectionWeb);  // Load HTML từ PROGMEM nếu cần
  response += "<div style='display: flex; margin: 20px;'><a style='color: darkmagenta;' href='http://";
  response += WiFi.localIP().toString();
  response += "'>Go to Device Page</a></div>";
  webServer.send(200, "text/html", response);
  delay(2000);

  ESP.restart();
}

void connectToWiFi() {
  WiFi.begin(wifisettings.SSID, wifisettings.password);
  Serial.print("Đang kết nối với WiFi...");

  // Thời gian bắt đầu kết nối
  unsigned long startAttemptTime = millis();

  // Thử kết nối với WiFi mới trong 20 giây
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 20000) {
    delay(500);
    Serial.print(".");
  }

  // Nếu không kết nối được trong 20 giây, chuyển sang WiFi cũ
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\nKhông kết nối được với WiFi mới, thử lại với WiFi cũ...");

    WiFi.begin(wifisettings.oldSSID, wifisettings.oldPassword);
    startAttemptTime = millis();

    // Thử kết nối với WiFi cũ
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 20000) {
      delay(500);
      Serial.print(".");
    }

    // Kiểm tra kết nối lại
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nĐã kết nối với WiFi cũ thành công!");
    } else {
      Serial.println("\nKhông kết nối được với WiFi cũ.");
    }
  } else {
    Serial.println("\nKết nối với WiFi mới thành công!");
    Serial.print("Địa chỉ IP: ");
    Serial.println(WiFi.localIP());
  }
  createAccessPoint();
}

void encodeData(uint16_t *rawData) {
  uint32_t dataBit[5];
  // Mã hóa dữ liệu dựa trên các tham số điều khiển và chuyển đổi thành mã thô.
  encodeMitsubishiIR(dataBit, temp, swing_auto, mode, power, fan_mode, swing_mode);
  Serial.println();
  Serial.print("Binary bit receive from MQTT: ");
  Serial.print(String(dataBit[0], BIN));
  Serial.print(String(dataBit[1], BIN));
  Serial.print(String(dataBit[2], BIN));
  Serial.print(String(dataBit[3], BIN));
  Serial.print(String(dataBit[4], BIN));
  Serial.println();
  convertToRawCode(dataBit, rawData);
}

void setup() {
  Serial.begin(115200);  // Khởi động giao tiếp Serial với baud rate 115200.

  readSettingsFromConfig();  // Đọc cấu hình từ file
  if (checkWiFiSaveInFile) {
    connectToWiFi();
  } else {
    initializeWiFiManager();  // Khởi tạo WiFiManager
  }
  if (shouldSaveConfig) {
    saveConfig();  // Lưu cấu hình nếu có thay đổi
  }
  initializeMqttClient();  // Thiết lập MQTT


  // Khởi tạo web server cho giao diện người dùng.
  webServer.on("/", handleRoot);
  webServer.on("/savewifi", HTTP_POST, changeWiFiParameters);
  webServer.on("/savemqtt", HTTP_POST, changeMQTTParameters);

  webServer.begin();
  Serial.println("HTTP server started");

  irsend.begin();  // Khởi động chức năng gửi IR.
  // dht.begin();    // Khởi động cảm biến DHT.

#if ESP8266
  Serial.begin(kBaudRate, SERIAL_8N1, SERIAL_TX_ONLY);  // Khởi động Serial cho ESP8266.
#else                                                   // ESP8266
  Serial.begin(kBaudRate, SERIAL_8N1);  // Khởi động Serial cho các loại khác (nếu có).
#endif                                                  // ESP8266
  while (!Serial)                                       // Wait for the serial connection to be establised.
    delay(50);
  assert(irutils::lowLevelSanityCheck() == 0);

#if DECODE_HASH
  irrecv.setUnknownThreshold(kMinUnknownSize);
#endif                                        // DECODE_HASH
  irrecv.setTolerance(kTolerancePercentage);  // Override the default tolerance.
  irrecv.enableIRIn();                        // Start the receiver

  // pinMode(ld2410_pin, INPUT); // Thiết lập pin cho cảm biến LD2410 là đầu vào.

}




// unsigned long lastTimeRecord = 0;
// void detectHumanAndReadTempAndHumidity()
// {
//   // Chỉ thực hiện kiểm tra mỗi 500ms
//   if (millis() - lastTimeRecord >= 2000)
//   {
//     lastTimeRecord = millis();                  // Cập nhật thời gian hiện tại.
//     int human_detect = digitalRead(ld2410_pin); // Đọc trạng thái của cảm biến LD2410.

//     // Gửi thông báo qua MQTT về trạng thái phát hiện con người.
//     if (human_detect == 1)
//     {
//       Serial.println("Human detected");
//       mqtt_client.publish("sensor/ld2410/state", "detected");
//     }
//     else
//     {
//       Serial.println("Human not detected");
//       mqtt_client.publish("sensor/ld2410/state", "clear");
//     }

//     // Đọc nhiệt độ và độ ẩm từ cảm biến DHT.
//     float humidity = dht.readHumidity();
//     float temperature = dht.readTemperature();

//     // Kiểm tra xem đọc giá trị có hợp lệ hay không.
//     if (isnan(humidity) || isnan(temperature))
//     {
//       Serial.println("Failed to read from DHT sensor!");
//     }
//     else
//     {
//       Serial.print("Temperature: ");
//       Serial.print(temperature);
//       mqtt_client.publish("sensor/dht11/temperature", String(temperature).c_str()); // Gửi nhiệt độ qua MQTT.
//       Serial.print(" C\tHumidity: ");
//       Serial.print(humidity);
//       mqtt_client.publish("sensor/dht11/humidity", String(humidity).c_str()); // Gửi độ ẩm qua MQTT.
//       Serial.println("% ");
//     }
//   }
// }

/**
 * Publishes the parsed Auto Swing and Temperature data to Home Assistant via MQTT.
 */
void processBinaryIRSignalAndSendToMQTT(String resultData) {
  // Serial.print("\nLength of binary: ");
  //   Serial.println(resultData.length());
  if (resultData.length() == MAX_CODE_BIT_LENGTH) {
    Serial.print("\nTemperature: ");
    Serial.print(returnTempForMQTT(resultData));
    Serial.print("\nMode: ");
    Serial.print(returnModeForMQTT(resultData));
    Serial.print("\nSwing: ");
    Serial.print(returnSwingModeForMQTT(resultData));
    Serial.print("\nFan: ");
    Serial.print(returnFanModeForMQTT(resultData));
    Serial.print("\nPower: ");
    Serial.print(returnPowerForMQTT(resultData));
    mqtt_client.publish("mitsu/temperature/set", returnTempForMQTT(resultData));
    mqtt_client.publish("mitsu/mode/set", returnModeForMQTT(resultData));
    mqtt_client.publish("mitsu/swing/set", returnSwingModeForMQTT(resultData));
    mqtt_client.publish("mitsu/fan/set", returnFanModeForMQTT(resultData));
    mqtt_client.publish("mitsu/mode/set", returnPowerForMQTT(resultData));
    mqtt_client.publish("mitsu/swing/set", returnSwingAutoForMQTT(resultData));
  }
}

String getRawValueFromIR(const decode_results *const results) {
  String output = "";
  const uint16_t length = getCorrectedRawLength(results);

  // Loop through the raw data and append each value to output
  for (uint16_t i = 1; i < results->rawlen; i++) {
    uint32_t usecs;
    for (usecs = results->rawbuf[i] * kRawTick; usecs > UINT16_MAX;
         usecs -= UINT16_MAX) {
      output += uint64ToString(UINT16_MAX);
      output += ", ";
    }
    output += uint64ToString(usecs, 10);
    if (i < results->rawlen - 1)
      output += ", ";  // ',' not needed on the last one
  }

  return output;
}

String excuteIRsignal() {
  String binaryData = "";
  // Check if the IR code has been received.
  if (irrecv.decode(&results)) {
    String rawData = getRawValueFromIR(&results);
    yield();  // Feed the WDT (again)
    binaryData = turnRawSignalToBinary(rawData);
    // Serial.print("Length of binary: ");
    // Serial.println(binaryData.length());
    processBinaryIRSignalAndSendToMQTT(binaryData);
  }
  return binaryData;
}
void loop() {
  // detectHumanAndReadTempAndHumidity(); // Kiểm tra và đọc nhiệt độ, độ ẩm.
  connectToMQTTBroker();
  processBinaryIRSignalAndSendToMQTT(excuteIRsignal());
  // Xử lý các yêu cầu từ web server.
  webServer.handleClient();
}
