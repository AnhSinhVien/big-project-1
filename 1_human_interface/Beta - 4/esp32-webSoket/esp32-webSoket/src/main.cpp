#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WebSocketsServer.h>
#include <HardwareSerial.h>



const char* ssid = "wifiLamDAKoCho";
const char* password = "vocungnhe";

// Cấu hình IP tĩnh
// wifi nhà
// IPAddress local_IP(192, 168, 0, 200);    // IP cố định mong muốn
// IPAddress gateway(192, 168, 0, 1);       // Gateway của mạng
// IPAddress subnet(255, 255, 255, 0);      // Subnet mask
// IPAddress primaryDNS(192, 168, 0, 1);        // DNS router
// IPAddress secondaryDNS(0, 0, 0, 0);      // DNS router

// wifi điện thoại
IPAddress local_IP(192, 168, 146, 200);           // IP cố định mong muốn
IPAddress gateway(192, 168, 146, 21);            // Gateway của mạng
IPAddress subnet(255, 255, 255, 0);              // Subnet mask
IPAddress primaryDNS(192, 168, 146, 21);        // DNS router
IPAddress secondaryDNS(0, 0, 0, 0);              // DNS router


AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
HardwareSerial SerialPort(2);  //if using UART2


void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  if (type == WS_EVT_DATA) {
    data[len] = '\0';  // Đảm bảo dữ liệu nhận được là một chuỗi ký tự hợp lệ
    SerialPort.println((char*)data);
  }
}

void setup() {
  Serial.begin(9600); // Khởi tạo Serial cho giám sát
  SerialPort.begin(9600, SERIAL_8N1, 16, 17); // UART2 với RX2 = GPIO 16, TX2 = GPIO 17

  // Cấu hình IP tĩnh trước khi kết nối WiFi
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("Cấu hình IP tĩnh thất bại.");
  }
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Đang kết nối tới WiFi...");
  }
  Serial.println("Đã kết nối với WiFi!");
  Serial.print("Địa chỉ IP tĩnh: ");
  Serial.println(WiFi.localIP());

  // xem địa chỉ ip
  // Serial.print("SSID: "); Serial.println(WiFi.SSID());
  // Serial.print("Địa chỉ IP: "); Serial.println(WiFi.localIP());
  // Serial.print("RSSI: "); Serial.println(WiFi.RSSI());
  // Serial.print("MAC: "); Serial.println(WiFi.macAddress());
  // Serial.print("Gateway: "); Serial.println(WiFi.gatewayIP());
  // Serial.print("Subnet Mask: "); Serial.println(WiFi.subnetMask());

  ws.onEvent(onWebSocketEvent);
  server.addHandler(&ws);
  server.begin();
}

void loop() {
  ws.cleanupClients();
  if(SerialPort.available()){
    String phanHoi = SerialPort.readStringUntil('\n');
    String response = "OFF ban so " + phanHoi;
    ws.textAll(response); // Gửi tín hiệu tắt về tất cả clients
    Serial.println(response);
  }
  
}
