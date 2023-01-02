#include <WiFi.h>
#include <ESPmDNS.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include <IRsend.h>

const char* ssid     = ";)";
const char* password = "NiceTry";




char webpage[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<body>
<script>
var connection = new WebSocket('ws://'+location.hostname+':81/');
var commandButton =0;
function powerButton()
{
  commandButton = 1
  console.log("Power");
  var full_data = '{"Command" :1}';
  connection.send(full_data);
}
function inputButton()
{
commandButton = 2
console.log("Input");
  var full_data = '{"Command" :2}';
  connection.send(full_data);
}
function VolumeUpButton()
{
  commandButton = 3
  console.log("Volume Up");
  var full_data = '{"Command" :3}';
  connection.send(full_data);
}
function VolumeDownButton()
{
  commandButton = 4
console.log("Volume Down");
  var full_data = '{"Command" :4}';
  connection.send(full_data);
}

</script>
<center>
<h1>Living Room TV</h1>
<button style="border-radius: 12px; font-size: 100px; padding: 32px 16px;" onclick= "powerButton()" >Power</button><br/>
<button style="border-radius: 12px; font-size: 100px; padding: 32px 16px;" onclick="inputButton()" >Input</button><br/>
<button style="border-radius: 12px; font-size: 100px; padding: 32px 16px;" onclick="VolumeUpButton()">Volume Up</button><br/>
<button style="border-radius: 12px; font-size: 100px; padding: 32px 16px;" onclick="VolumeDownButton()">Volume Down </button>
</center>
</body>
</html>
)=====";

// ipaddress/led1/on
//ipaddress/led1/off

// ipaddress/led2/on
//ipaddress/led2/off
#include <ESPAsyncWebServer.h>

AsyncWebServer server(80); // server port 80
WebSocketsServer websockets(81); 
IRsend irsend(14);


void notFound(AsyncWebServerRequest *request)
{
  request->send(404, "text/plain", "Page Not found");
}


void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

  switch (type) 
  {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED: {
        IPAddress ip = websockets.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

        // send message to client
        websockets.sendTXT(num, "Connected from server");
      }
      break;
    case WStype_TEXT:
      Serial.printf("[%u] get Text: %s\n", num, payload);
      String message = String((char*)( payload));
      Serial.println(message);

      
     DynamicJsonDocument doc(200);
    // deserialize the data
    DeserializationError error = deserializeJson(doc, message);
    // parse the parameters we expect to receive (TO-DO: error handling)
      // Test if parsing succeeds.
  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }
 int irCommand = doc["Command"];
  switch(irCommand){
    case 1:
      irsend.sendRC5(0x0C);
      break;
    case 2:
      irsend.sendRC5(0x38);
      break;
    case 3:
      irsend.sendRC5(0x10);
      break;
    case 4:
      irsend.sendRC5(0x11);
      break;
  }

  }
}

void setup(void)
{

  Serial.begin(115200);
  irsend.begin();
  Serial.println("I have turned on.");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }


  if (MDNS.begin("LivingRoomTV")) { //esp.local/
    Serial.println("MDNS responder started");
  }



  server.on("/", [](AsyncWebServerRequest * request)
  { 
   
  request->send_P(200, "text/html", webpage);
  });

//   server.on("/led1/on", HTTP_GET, [](AsyncWebServerRequest * request)
//  { 
//    digitalWrite(LED1,HIGH);
//  request->send_P(200, "text/html", webpage);
//  });

  server.onNotFound(notFound);

  server.begin();  // it will start webserver
  websockets.begin();
  websockets.onEvent(webSocketEvent);

}


void loop(void)
{
 websockets.loop();
}
