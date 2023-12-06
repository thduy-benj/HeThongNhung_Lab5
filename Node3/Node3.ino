///Khai báo mấy cái thư viện///////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <WiFiClient.h>
#include <Arduino_JSON.h>
#include "painlessMesh.h"
#include <Arduino.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
///Điều khiển đèn///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AsyncWebServer server(80);

const char webpage[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
  <meta http-equiv='Content-Type' content='text/html; charset=utf-8'>
  <title>Điều khiển thiết bị</title>
  <meta name='viewport' content='width=device-width, initial-scale=1'>
  <style>
    .ON { width: 100px; height: 40px; font-size: 21px; color: #FFF; background-color:#4caf50; border-radius: 10px; }
    .OFF { width: 100px; height: 40px; font-size: 21px; color: #FFF; background-color:#f44336; border-radius: 10px; }
    .container { width: 330px; height: auto; margin: 0 auto; margin-top: 70px; }
  </style>
</head>
<body>
  <div class='container'>
    <h1 align='center'>HỆ THỐNG NHÚNG MẠNG KHÔNG DÂY LAB5</h1>
    <table align='center'>
      <tr>
        <td><button class="ON" onclick="send(1)">ON</button></td>
        <td><button class="OFF" onclick="send(0)">OFF</button></td>
      </tr>
    </table>
  </div>
  <br>
  <div class='container'>
    <h2>
      Nhiệt độ(C): <span id="temperature">%TEMPERATURE%</span><br>
      Độ ẩm(%): <span id="humidity">%HUMIDITY%</span><br>
      Ánh sáng(lx): <span id="lux">%LUX%</span><br>
    </h2>
  </div>

  <script>
    function send(led_sts) 
    {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() 
      {
        if (this.readyState == 4 && this.status == 200) {
          document.getElementById("state").innerHTML = this.responseText;
        }
      };
      xhttp.open("GET", "/led_set?state=" + led_sts, true);
      xhttp.send();
    }
    setInterval(function ( ) {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          document.getElementById("lux").innerHTML = this.responseText;
        }
      };
      xhttp.open("GET", "/lux", true);
      xhttp.send();
    }, 1000 );
    setInterval(function ( ) {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          document.getElementById("temperature").innerHTML = this.responseText;
        }
      };
      xhttp.open("GET", "/temperature", true);
      xhttp.send();
    }, 1000 );
    setInterval(function ( ) {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          document.getElementById("humidity").innerHTML = this.responseText;
        }
      };
      xhttp.open("GET", "/humidity", true);
      xhttp.send();
    }, 1000 );
  </script>
</body>
</html>
)=====";

int LED = D5;
int ledState = LOW;

void led_control(AsyncWebServerRequest *request) {
  String act_state = request->arg("state");
  if (act_state == "1") {
    digitalWrite(LED, HIGH);
  } else {
    digitalWrite(LED, LOW);
  }
  request->send(200, "text/plain", act_state);
}

///Nhận dữ liệu từ 2 node/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define   MESH_PREFIX     "NT131.O11.1"
#define   MESH_PASSWORD   "Nhom01Mesh"
#define   MESH_PORT       5555

#define STATION_SSID "UiTiOt-E3.1"
#define STATION_PASSWORD "UiTiOtAP"

Scheduler userScheduler; // to control your personal task
painlessMesh  mesh;

// User stub
void sendMessage() ; // Prototype so PlatformIO doesn't complain

Task taskSendMessage( TASK_SECOND * 1 , TASK_FOREVER, &sendMessage );

void sendMessage() {
  String msg = "Hello from root node";
  // msg += mesh.getNodeId();
  mesh.sendBroadcast( msg );
  taskSendMessage.setInterval( random( TASK_SECOND * 1, TASK_SECOND * 5 ));
}

// Lấy thông tin từ 2 node
double lux = 0; 
double hum = 0;
double temp = 0;
void receivedCallback(uint32_t from, String &msg) {
  Serial.printf("Received from %u msg=%s\n", from, msg.c_str());
  JSONVar myObject = JSON.parse(msg.c_str());
  int node = myObject["Node"];
  if (node == 2 ) lux = myObject["Data"]["Light"];
  else if (node == 1) {
    temp = myObject["Data"]["Temp"];
    hum = myObject["Data"]["Hum"];
  }
  // Serial.print("Node: ");
  // Serial.println(node);
  // Serial.print("Temperature: ");
  // Serial.print(temp);
  // Serial.println(" C");
  // Serial.print("Humidity: ");
  // Serial.print(hum);
  // Serial.println(" %");
  // Serial.print("Light: ");
  // Serial.print(light);
  // Serial.println(" %");
}

void newConnectionCallback(uint32_t nodeId) {
  Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback() {
  Serial.printf("Changed connections\n");
}

void nodeTimeAdjustedCallback(int32_t offset) {
    Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);
}

String processor(const String& var){
  //Serial.println(var);
  if(var == "TEMPERATURE"){
    return String(temp);
  } else if(var == "HUMIDITY"){
    return String(hum);
  } else if (var == "LUX"){
    return String(lux);
  }
  return String();
}


void setup() {
  Serial.begin(115200);

//mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages

  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  userScheduler.addTask( taskSendMessage );
  taskSendMessage.enable();

  pinMode(LED, OUTPUT);

  Serial.println();
  Serial.print("Connecting to ");
  WiFi.begin("UiTiOt-E3.1", "UiTiOtAP");

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
  //   request->send(200, "text/html", webpage);
  // });
  server.on("/led_set", HTTP_POST, [](AsyncWebServerRequest *request){
    led_control(request);
  });
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", webpage, processor);
  });
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(temp).c_str());
  });
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(hum).c_str());
  });
  server.on("/lux", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(lux).c_str());
  });

  server.begin();
}

void loop() {
  // it will run the user scheduler as well
  mesh.update();
  // server.handleClient();
}