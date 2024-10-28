#include <Arduino.h>
#include <IRsend.h>
#include "AcCommand.h"
#include <DHT.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <WiFiManager.h> // Thư viện để quản lý kết nối WiFi
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <IRrecv.h>
#include <assert.h>
#include <IRutils.h>

#define DHTTYPE DHT11 // Định nghĩa loại cảm biến DHT là DHT11

// IR + dht + human sensor pin

const uint32_t kBaudRate = 115200;
const uint16_t kCaptureBufferSize = 1024;
const uint16_t kIrLed = 4;    // Pin GPIO cho LED IR (ESP8266 GPIO 4 - D2).
const uint16_t kRecvPin = 14; // Pin GPIO cho IR Receiver (D5)

#if DECODE_AC
const uint8_t kTimeout = 50;
#else  // DECODE_AC
const uint8_t kTimeout = 15;
#endif // DECODE_AC
const uint16_t kMinUnknownSize = 12;
const uint8_t kTolerancePercentage = kTolerance; // kTolerance is normally 25%
IRrecv irrecv(kRecvPin, kCaptureBufferSize, kTimeout, true);
decode_results results; // Somewhere to store the results

const uint8_t DHTPin = 2;     // Pin GPIO cho cảm biến DHT (D4).
const uint8_t ld2410_pin = 5; // Pin GPIO cho cảm biến chuyển động (D1).

// MQTT Config
char mqtt_server[40] = "10.8.102.2"; // Địa chỉ IP của MQTT Broker.
char mqtt_port[6] = "1883";          // Cổng MQTT.
char mqtt_user[20] = "usermqtt";     // Tên người dùng MQTT.
char mqtt_pass[20] = "huycan1605";   // Mật khẩu MQTT.

// Custom parameters for MQTT
// Khởi tạo các tham số cấu hình MQTT cho WiFiManager.
WiFiManagerParameter custom_mqtt_server("server", "MQTT Server", mqtt_server, 40);
WiFiManagerParameter custom_mqtt_port("port", "MQTT Port", mqtt_port, 6);
WiFiManagerParameter custom_mqtt_user("user", "MQTT User", mqtt_user, 20);
WiFiManagerParameter custom_mqtt_pass("pass", "MQTT Password", mqtt_pass, 20);

WiFiClient espClient;                // Tạo một client WiFi.
PubSubClient mqtt_client(espClient); // Tạo một client MQTT với client WiFi.

WiFiManager wifiManager; // Tạo một instance của WiFiManager.

// Web Server
ESP8266WebServer server(80); // Tạo một server web trên cổng 80.

DHT dht(DHTPin, DHTTYPE); // Khởi tạo cảm biến DHT với pin và loại đã định nghĩa.
IRsend irsend(kIrLed);    // Khởi tạo IRsend với pin LED IR đã định nghĩa.

void connectToMQTTBroker()
{
  // Hàm kết nối đến MQTT Broker.
  while (!mqtt_client.connected())
  {                                                                   // Vòng lặp cho đến khi kết nối thành công.
    String client_id = "esp8266-client-" + String(WiFi.macAddress()); // Tạo ID cho client dựa trên địa chỉ MAC.
    Serial.printf("Connecting to MQTT Broker as %s..... \n", client_id.c_str());

    // Thử kết nối đến MQTT broker với thông tin xác thực.
    if (mqtt_client.connect(client_id.c_str(), mqtt_user, mqtt_pass))
    {
      Serial.println("Connected to MQTT broker"); // Thông báo kết nối thành công.
      // Đăng ký các chủ đề để nhận tin nhắn.
      mqtt_client.subscribe("study/ac/mode");
      mqtt_client.subscribe("study/ac/temperature");
      mqtt_client.subscribe("study/ac/fan");
      mqtt_client.subscribe("study/ac/swing");
    }
    else
    {
      Serial.println("Connection failed, rc = "); // Thông báo kết nối thất bại.
      Serial.println(mqtt_client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000); // Chờ 5 giây trước khi thử lại.
    }
  }
}

void mqttCallback(char *topic, byte *payload, unsigned int length)
{
  // Hàm gọi lại khi nhận được tin nhắn từ MQTT.
  String reveiceMessage = "";
  Serial.print("Message received on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");

  // Chuyển đổi payload thành chuỗi để sử dụng.
  for (int i = 0; i < length; i++)
  {
    reveiceMessage += (char)payload[i];
  }
  
  Serial.println();
  Serial.print(reveiceMessage);
  Serial.println();
  Serial.println("------------------------");
  getTopicAndMessage(topic, reveiceMessage); // Gọi hàm xử lý tin nhắn.
}

uint8_t temp = 19;       // Biến để lưu trữ giá trị nhiệt độ.
uint8_t mode = 1;       // Biến để lưu trữ chế độ hoạt động.
uint8_t fan_mode = 1;   // Biến để lưu trữ chế độ quạt.
uint8_t swing_mode = 1; // Biến để lưu trữ chế độ đảo gió.
bool swing_auto= false;    // Biến để xác định chế độ đảo gió tự động.
bool power = false;         // Biến để xác định trạng thái nguồn (ON/OFF).

void getTopicAndMessage(char *topic, String reveice)
{
  int count = 0;          // Biến đếm số lần xuất hiện ký tự '/' trong topic.
  String main_topic = ""; // Chuỗi lưu trữ phần chủ đề chính.

  // topic from HA: study/ac/temperature
  for (int i = 0; i < strlen(topic); i++)
  {
    if (topic[i] == '/')
    {
      count++; // Tăng biến đếm mỗi khi gặp '/'.
    }
    // Khi đã gặp đủ 2 lần '/' và ký tự tiếp theo không phải là '\0'
    if (count == 2 && topic[i + 1] != '\0' && i + 1 < strlen(topic))
    {
      main_topic += topic[i + 1]; // Lấy ký tự tiếp theo sau '/' thứ hai.
    }
  }

  // Xác định hành động dựa trên chủ đề chính.
  if (main_topic.equals("temperature"))
  {
    temp = reveice.toInt(); // Chuyển đổi dữ liệu nhận được thành số nguyên cho nhiệt độ.
  }
  else if (main_topic.equals("mode"))
  {
    mode = reveice.toInt(); // Chuyển đổi dữ liệu nhận được thành số nguyên cho chế độ.
  }
  else if (main_topic.equals("fan"))
  {
    fan_mode = reveice.toInt(); // Chuyển đổi dữ liệu nhận được thành số nguyên cho chế độ quạt.
  }
  else if (main_topic.equals("swing"))
  {
    swing_mode = reveice.toInt(); // Chuyển đổi dữ liệu nhận được thành số nguyên cho chế độ đảo gió.
  }

  // Xác định trạng thái nguồn và chế độ đảo gió tự động.
  power = (mode == 5) ? false : true;            // Nếu mode = 5 thì tắt nguồn (power = false).
  swing_auto = (swing_mode == 0) ? true : false; // Nếu swing_mode = 0 thì chế độ đảo gió tự động (swing_auto = true).

  uint16_t rawData[MAX_CODE_LENGTH];            // Mảng lưu trữ dữ liệu thô từ cảm biến IR.
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
  encodeData(rawData);                          // Gọi hàm để mã hóa dữ liệu IR.
  irsend.sendRaw(rawData, MAX_CODE_LENGTH, 38); // Gửi dữ liệu IR đã mã hóa.
}

void encodeData(uint16_t *rawData)
{
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

void setup()
{
  Serial.begin(115200); // Khởi động giao tiếp Serial với baud rate 115200.

  // Thêm các tham số MQTT tùy chỉnh vào WiFiManager.
  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_port);
  wifiManager.addParameter(&custom_mqtt_user);
  wifiManager.addParameter(&custom_mqtt_pass);

  // Uncomment to reset saved WiFi credentials
  // wifiManager.resetSettings(); // Đặt lại cài đặt WiFi đã lưu.

  Serial.println("Config WiFi and MQTT");
  // Kết nối WiFi và thiết lập thông tin MQTT.
  if (!wifiManager.autoConnect("ESP8266_Config"))
  {
    Serial.println("Failed to connect to WiFi and hit timeout"); // Thông báo nếu kết nối không thành công.
    ESP.reset();                                                 // Reset ESP8266.
    delay(1000);                                                 // Chờ 1 giây.
  }

  Serial.println("WiFi connected!"); // Thông báo kết nối WiFi thành công.

  // Lưu thông tin cấu hình MQTT từ WiFiManager.
  strcpy(mqtt_server, custom_mqtt_server.getValue());
  strcpy(mqtt_port, custom_mqtt_port.getValue());
  strcpy(mqtt_user, custom_mqtt_user.getValue());
  strcpy(mqtt_pass, custom_mqtt_pass.getValue());

  // Khởi tạo web server cho giao diện người dùng.
  server.on("/", handleRoot);         // Xử lý trang chính.
  server.on("/config", handleConfig); // Xử lý trang cấu hình.
  server.begin();                     // Bắt đầu server.
  Serial.println("HTTP server started");

  irsend.begin(); // Khởi động chức năng gửi IR.
  dht.begin();    // Khởi động cảm biến DHT.

#if ESP8266
  Serial.begin(kBaudRate, SERIAL_8N1, SERIAL_TX_ONLY); // Khởi động Serial cho ESP8266.
#else                                                  // ESP8266
  Serial.begin(kBaudRate, SERIAL_8N1); // Khởi động Serial cho các loại khác (nếu có).
#endif                                                 // ESP8266
  while (!Serial)                                      // Wait for the serial connection to be establised.
    delay(50);
  assert(irutils::lowLevelSanityCheck() == 0);

#if DECODE_HASH
  irrecv.setUnknownThreshold(kMinUnknownSize);
#endif                                       // DECODE_HASH
  irrecv.setTolerance(kTolerancePercentage); // Override the default tolerance.
  irrecv.enableIRIn();                       // Start the receiver

  pinMode(ld2410_pin, INPUT); // Thiết lập pin cho cảm biến LD2410 là đầu vào.

  // Kết nối đến MQTT Broker.
  mqtt_client.setServer(mqtt_server, atoi(mqtt_port)); // Thiết lập máy chủ MQTT.
  mqtt_client.setCallback(mqttCallback);               // Thiết lập hàm callback cho MQTT.
  connectToMQTTBroker();                               // Gọi hàm để kết nối đến MQTT broker.
}

void handleRoot()
{
  String html = "<html><body>";
  html += "<h1>ESP8266 Configuration</h1>";
  html += "<form action='/config'>"; // Form gửi đến hàm handleConfig khi người dùng nhấn nút Save.
  html += "MQTT Server: <input type='text' name='server' value='" + String(mqtt_server) + "'><br>";
  html += "MQTT Port: <input type='text' name='port' value='" + String(mqtt_port) + "'><br>";
  html += "MQTT User: <input type='text' name='user' value='" + String(mqtt_user) + "'><br>";
  html += "MQTT Password: <input type='password' name='pass' value='" + String(mqtt_pass) + "'><br>";
  html += "<input type='submit' value='Save'>"; // Nút gửi thông tin cấu hình.
  html += "</form></body></html>";
  server.send(200, "text/html", html); // Gửi nội dung HTML về client.
}

// Handle saving configuration
void handleConfig()
{
  // Lấy thông tin cấu hình mới từ request.
  String new_mqtt_server = server.arg("server");
  String new_mqtt_port = server.arg("port");
  String new_mqtt_user = server.arg("user");
  String new_mqtt_pass = server.arg("pass");

  // Chuyển đổi chuỗi thành mảng ký tự để lưu trữ.
  new_mqtt_server.toCharArray(mqtt_server, 40);
  new_mqtt_port.toCharArray(mqtt_port, 6);
  new_mqtt_user.toCharArray(mqtt_user, 20);
  new_mqtt_pass.toCharArray(mqtt_pass, 20);

  // Tạo phản hồi để xác nhận cấu hình đã lưu.
  String response = "Config Saved!<br>";
  response += "MQTT Server: " + new_mqtt_server + "<br>";
  response += "MQTT Port: " + new_mqtt_port + "<br>";
  response += "MQTT User: " + new_mqtt_user + "<br>";
  server.send(200, "text/html", response); // Gửi phản hồi về client.

  // Có thể lưu cấu hình vào EEPROM hoặc SPIFFS để bảo toàn sau khi khởi động lại.
}

void reconnectMQTT()
{
  // Thử kết nối đến MQTT broker.
  while (!mqtt_client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Kết nối với MQTT broker bằng thông tin đăng nhập.
    if (mqtt_client.connect("ESP8266Client", mqtt_user, mqtt_pass))
    {
      Serial.println("connected");
      mqtt_client.subscribe("study/ac/mode");
      mqtt_client.subscribe("study/ac/temperature");
      mqtt_client.subscribe("study/ac/fan");
      mqtt_client.subscribe("study/ac/swing");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(mqtt_client.state()); // In ra mã lỗi nếu không kết nối thành công.
      Serial.println(" try again in 5 seconds");
      delay(5000); // Chờ 5 giây trước khi thử lại.
    }
  }
}

unsigned long lastTimeRecord = 0;

void detectHumanAndReadTempAndHumidity()
{
  // Chỉ thực hiện kiểm tra mỗi 500ms
  if (millis() - lastTimeRecord >= 2000)
  {
    lastTimeRecord = millis();                  // Cập nhật thời gian hiện tại.
    int human_detect = digitalRead(ld2410_pin); // Đọc trạng thái của cảm biến LD2410.

    // Gửi thông báo qua MQTT về trạng thái phát hiện con người.
    if (human_detect == 1)
    {
      Serial.println("Human detected");
      mqtt_client.publish("sensor/ld2410/state", "detected");
    }
    else
    {
      Serial.println("Human not detected");
      mqtt_client.publish("sensor/ld2410/state", "clear");
    }

    // Đọc nhiệt độ và độ ẩm từ cảm biến DHT.
    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();

    // Kiểm tra xem đọc giá trị có hợp lệ hay không.
    if (isnan(humidity) || isnan(temperature))
    {
      Serial.println("Failed to read from DHT sensor!");
    }
    else
    {
      Serial.print("Temperature: ");
      Serial.print(temperature);
      mqtt_client.publish("sensor/dht11/temperature", String(temperature).c_str()); // Gửi nhiệt độ qua MQTT.
      Serial.print(" C\tHumidity: ");
      Serial.print(humidity);
      mqtt_client.publish("sensor/dht11/humidity", String(humidity).c_str()); // Gửi độ ẩm qua MQTT.
      Serial.println("% ");
    }
  }
}

/**
 * Publishes the parsed Auto Swing and Temperature data to Home Assistant via MQTT.
 */
void processBinaryIRSignalAndSendToMQTT(String resultData)
{
  // Serial.print("\nLength of binary: ");
  //   Serial.println(resultData.length());
  if(resultData.length() == MAX_CODE_BIT_LENGTH){
    Serial.print("\nTemperature: "); Serial.print(returnTempForMQTT(resultData));
    Serial.print("\nMode: "); Serial.print(returnModeForMQTT(resultData));
    Serial.print("\nSwing: "); Serial.print(returnSwingModeForMQTT(resultData));
    Serial.print("\nFan: "); Serial.print(returnFanModeForMQTT(resultData));
    Serial.print("\nPower: "); Serial.print(returnPowerForMQTT(resultData));
    mqtt_client.publish("mitsu/temperature/set", returnTempForMQTT(resultData));
      mqtt_client.publish("mitsu/mode/set", returnModeForMQTT(resultData));
      mqtt_client.publish("mitsu/swing/set", returnSwingModeForMQTT(resultData));
      mqtt_client.publish("mitsu/fan/set", returnFanModeForMQTT(resultData));
      mqtt_client.publish("mitsu/mode/set", returnPowerForMQTT(resultData));
      mqtt_client.publish("mitsu/swing/set", returnSwingAutoForMQTT(resultData));
  }  
}

String getRawValueFromIR(const decode_results *const results)
{
  String output = "";
  const uint16_t length = getCorrectedRawLength(results);

  // Loop through the raw data and append each value to output
  for (uint16_t i = 1; i < results->rawlen; i++)
  {
    uint32_t usecs;
    for (usecs = results->rawbuf[i] * kRawTick; usecs > UINT16_MAX;
         usecs -= UINT16_MAX)
    {
      output += uint64ToString(UINT16_MAX);
      output += ", ";
    }
    output += uint64ToString(usecs, 10);
    if (i < results->rawlen - 1)
      output += ", "; // ',' not needed on the last one
  }

  return output;
}

String excuteIRsignal()
{
  String binaryData = "";
  // Check if the IR code has been received.
  if (irrecv.decode(&results))
  {
    String rawData = getRawValueFromIR(&results);
    yield();          // Feed the WDT (again)
    binaryData = turnRawSignalToBinary(rawData);
    // Serial.print("Length of binary: ");
    // Serial.println(binaryData.length());
    processBinaryIRSignalAndSendToMQTT(binaryData);
    
  }
  return binaryData;
}
void loop()
{
  // detectHumanAndReadTempAndHumidity(); // Kiểm tra và đọc nhiệt độ, độ ẩm.
  if (!mqtt_client.connected())
  {
    reconnectMQTT(); // Nếu không kết nối, thử kết nối lại.
  }
  mqtt_client.loop(); // Cập nhật trạng thái MQTT.
  processBinaryIRSignalAndSendToMQTT(excuteIRsignal());
  // Xử lý các yêu cầu từ web server.
  // server.handleClient();
}
