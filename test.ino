#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

MDNSResponder mdns;
ESP8266WebServer server(80);

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
      Nhiệt độ(C): <span id="temp">0</span><br>
      Độ ẩm(%): <span id="hum">0</span><br>
      Ánh sáng(lx): <span id="lux">0</span><br>
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
  </script>
</body>
</html>
)=====";

int LED = D5;
int ledState = LOW;

void TrangChu()
{
  server.send(200, "text/html", webpage);
}

void led_control()
{
  String act_state = server.arg("state");
  if (act_state == "1")
  {
    digitalWrite(LED, HIGH);
  }
  else
  {
    digitalWrite(LED, LOW);
  }
  server.send(200, "text/plane", act_state);
}

void setup()
{
  pinMode(LED, OUTPUT);

  Serial.begin(9600);
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
  if (mdns.begin("esp8266", WiFi.localIP()))
    Serial.println("MDNS responder started");

  // server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
  //   request->send(200, "text/html", webpage);
  // });
  server.on("/", TrangChu);
  server.on("/led_set", HTTP_GET, led_control);

  server.begin();
}

void loop()
{
  
  server.handleClient();
}