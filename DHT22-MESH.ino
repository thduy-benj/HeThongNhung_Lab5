//Libraries
#include <ArduinoJson.h>
#include <DHT22.h>
#include <painlessMesh.h>

// Constants
#define DHTPIN D6     // Chân dữ liệu của DHT22 được kết nối với chân D6 trên bo mạch Arduino
// #define DHTTYPE DHT22   // DHT 22  (AM2302)

#define   MESH_PREFIX     "NT131.O11.1"
#define   MESH_PASSWORD   "Nhom01Mesh"
#define   MESH_PORT       5555

DHT22 dht(DHTPIN); // Khởi tạo cảm biến DHT
Scheduler userScheduler; // Lập lịch để điều khiển tác vụ của bạn
painlessMesh mesh;

// Nguyên mẫu hàm
void sendMessage(); 

Task taskSendMessage(TASK_SECOND * 1, TASK_FOREVER, &sendMessage);

void sendMessage() {
  int nodeId = 1;
  // Đọc dữ liệu từ cảm biến DHT22
  float temp = dht.getTemperature();
  float hum = dht.getHumidity();

  // Tạo một tài liệu JSON
  StaticJsonDocument<256> jsonDocument;
  JsonObject root = jsonDocument.to<JsonObject>();
  root["Node"] = nodeId;
  JsonObject data = root.createNestedObject("Data");
  data["Temp"] = temp;
  data["Hum"] = hum;

  // Chuyển đổi JSON thành chuỗi
  String message;
  serializeJson(jsonDocument, message);

  // Gửi thông điệp broadcast
  mesh.sendBroadcast(message);

  // Đặt khoảng thời gian cho thông điệp tiếp theo
  taskSendMessage.setInterval(random(TASK_SECOND * 1, TASK_SECOND * 5));
}

  //Cần thiết cho thư viện painless
  void receivedCallback(uint32_t from, String &msg) {
    // Serial.printf("%s\n", msg.c_str());
  }

void newConnectionCallback(uint32_t nodeId) {
  Serial.printf("New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback() {
  Serial.println("Changed connections");
}

void nodeTimeAdjustedCallback(int32_t offset) {
  Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(), offset);
}

void setup() {
  Serial.begin(115200);

  mesh.setDebugMsgTypes(ERROR | STARTUP);
  mesh.init(MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT);
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  userScheduler.addTask(taskSendMessage);
  taskSendMessage.enable();
}

void loop() {
  mesh.update();
}