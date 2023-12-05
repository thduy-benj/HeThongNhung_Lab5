//Libraries
#include <ArduinoJson.h>
#include <Wire.h>
#include <BH1750.h>
#include <painlessMesh.h>

// Constants
BH1750 lightMeter;

#define   MESH_PREFIX     "NT131.O11.1"
#define   MESH_PASSWORD   "Nhom01Mesh"
#define   MESH_PORT       5555

Scheduler userScheduler; // Lập lịch để điều khiển tác vụ của bạn
painlessMesh mesh;

// Nguyên mẫu hàm
void sendMessage(); 

Task taskSendMessage(TASK_SECOND * 1, TASK_FOREVER, &sendMessage);

void sendMessage() {
  int nodeId = 2;
  // Đọc dữ liệu từ cảm biến BH1750
  float lig = lightMeter.readLightLevel();

  // Tạo một tài liệu JSON
  StaticJsonDocument<256> jsonDocument;
  JsonObject root = jsonDocument.to<JsonObject>();
  root["Node"] = nodeId;
  JsonObject data = root.createNestedObject("Data");
  data["Light"] = lig;

  // Chuyển đổi JSON thành chuỗi
  String message;
  serializeJson(jsonDocument, message);

  // Gửi thông điệp broadcast
  mesh.sendBroadcast(message);

  // Đặt khoảng thời gian cho thông điệp tiếp theo
  taskSendMessage.setInterval(random(TASK_SECOND * 1, TASK_SECOND * 5));
}

// Cần thiết cho thư viện painless
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

  Wire.begin();
  lightMeter.begin();
}

void loop() {
  mesh.update();
}