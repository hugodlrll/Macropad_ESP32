// #include <BleKeyboard.h>

// BleKeyboard bleKeyboard;
// int button = 34;

// void setup() {
//   Serial.begin(115200);
//   pinMode(button, INPUT_PULLUP);
//   bleKeyboard.begin();
// }

// void loop() {
//   if (digitalRead(button) == LOW) 
//   {
//     Serial.println("Button is pressed");
//     bleKeyboard.press(KEY_LEFT_CTRL);
//     bleKeyboard.press(KEY_LEFT_ALT);
//     bleKeyboard.press(KEY_DELETE);
//     bleKeyboard.releaseAll();
//     while(digitalRead(button) == LOW);
//   }
//   delay(20);
// }

#include <WiFi.h>
#include <blekeyboard.h>
#include <ArduinoWebsockets.h>
#include <ArduinoJson.h>

const char* ssid = "wifirobot"; //Enter SSID
const char* password = "5cjWSgq7sefAnnJq"; //Enter Password
const char* websockets_server_host = "192.168.10.58"; //Enter server adress
const uint16_t websockets_server_port = 8050; // Enter server port

using namespace websockets;

WebsocketsClient client;

void ConfigReveived(WebsocketsMessage message) 
{
  
  String data = message.data(); 
  //data.replace("\"", "\"");
  // Serial.println(data);
  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, data);
  if (error) 
  {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }
  JsonObject TOUCHE1 = doc["action"];
  //if(!TOUCHE1.isNull()){
    const char* TOUCHE1_action = doc["action"];
    const char* TOUCHE1_key1 = doc["key1"];
    const char* TOUCHE1_key2 = doc["key2"]; 
    const char* TOUCHE1_key3 = doc["key3"];
    Serial.println(TOUCHE1_action);
    Serial.println(TOUCHE1_key1);
    Serial.println(TOUCHE1_key2);
    Serial.println(TOUCHE1_key3);
  //}
}

void setup() 
{
    Serial.begin(115200);
    // Connect to wifi
    WiFi.begin(ssid, password);

    // Wait some time to connect to wifi
    for(int i = 0; i < 10 && WiFi.status() != WL_CONNECTED; i++) {
        Serial.print(".");
        delay(1000);
    }

    // Check if connected to wifi
    if(WiFi.status() != WL_CONNECTED) {
        Serial.println("No Wifi!");
        return;
    }

    Serial.println("Connected to Wifi, Connecting to server.");
    // try to connect to Websockets server
    bool connected = client.connect(websockets_server_host, websockets_server_port, "/");
    if(connected) {
        Serial.println("Connected!");
        client.send("Hello Server");
    } else {
        Serial.println("Not Connected!");
    }
    
    // run callback when messages are received
    client.onMessage([&](WebsocketsMessage message)
    {
      //Serial.println("Got Message : " + message.data());
      ConfigReveived(message);
    } );
}

void loop() 
{
  client.poll();
}