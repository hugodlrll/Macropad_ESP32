#include <WiFi.h>
#include <blekeyboard.h>
#include <ArduinoWebsockets.h>
#include <ArduinoJson.h>
#include <Keypad.h>
#include <Arduino.h>

TaskHandle_t task1;
TaskHandle_t task2;
const char* action;
String isMedia;
char key1;
char key2; 
char key3;
uint8_t media[2] = {};

const char * ssid = "SFR_1E88"; //"SFR_43A0";   //Enter SSID
const char * password = "16121998"; //"16121998";   //Enter Password
const char * websockets_server_host = "192.168.1.39"; //Enter server adress
const uint16_t websockets_server_port = 8050; // Enter server port

using namespace websockets;

WebsocketsClient client;
BleKeyboard bleKeyboard;
int button = 13;

void ConfigReveived(WebsocketsMessage message) 
{
  String data = message.data(); 
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, data);
  if (error) 
  {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }
  JsonObject TOUCHE1 = doc["action"];
  const char* TOUCHE1_action = doc["action"];
  const char* TOUCHE1_isMedia = doc["isMedia"];
  const char* TOUCHE1_key1 = doc["key1"];
  const char* TOUCHE1_key2 = doc["key2"]; 
  const char* TOUCHE1_key3 = doc["key3"];
  Serial.println(TOUCHE1_action);
  Serial.println(TOUCHE1_isMedia);
  Serial.println(TOUCHE1_key1);
  Serial.println(TOUCHE1_key2);
 // Serial.println(TOUCHE1_key3);
  isMedia = TOUCHE1_isMedia;
  Serial.println(isMedia);
  if(isMedia == "false")
  {
    key1 = (char)strtol(TOUCHE1_key1, NULL, 0);
    key2 = (char)strtol(TOUCHE1_key2, NULL, 0);
    key3 = (char)strtol(TOUCHE1_key3, NULL, 0);
  }
  if(isMedia == "true")
  {
    char *copy = strdup(TOUCHE1_key1);
    char *p = strtok(copy, ",");
    int i = 0;
    while (p != NULL) {
    media[i++] = atoi(p);
    p = strtok(NULL, ",");
}
free(copy);
    //Serial.println(media[0]);
    //Serial.println(media[1]);
  }
  // Serial.println(key1);
  // Serial.println(key2);
  // Serial.println(key3);
}

void KeyPressed(char key1, char key2, char key3)
{
  int buttonPressTime;
  if(isMedia == "false")
  {
    bleKeyboard.press(key1);
    bleKeyboard.press(key2);
    bleKeyboard.press(key3);
    bleKeyboard.releaseAll();
    delay(20);
  }
  if(isMedia == "true")
  {
    bleKeyboard.write(media);
    buttonPressTime = millis();
    while(digitalRead(button) == LOW){
      if(millis() - buttonPressTime > 600)
      {
        bleKeyboard.press(media);
        delay (20);
      }
    bleKeyboard.releaseAll();
    }
    
  }
}

void Bluetooth(void * parameter)
{
  for(;;)
  {
    if(digitalRead(button) == LOW)
    {
      Serial.println("Button is pressed");
      KeyPressed(key1, key2, key3);
      while(digitalRead(button) == LOW);
    }
    delay(20);
  }
}

void Configuration(void * parameter)
{
  for(;;)
  {
    client.poll();
  }
  delay(20);
}

void setup() 
{
  Serial.begin(115200);
  pinMode(button, INPUT_PULLUP);
  bleKeyboard.begin();
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
    ConfigReveived(message);
  } );

  xTaskCreatePinnedToCore(Configuration, "task1", 10000, NULL, 1, &task1, 1);
  xTaskCreatePinnedToCore(Bluetooth, "task2", 10000, NULL, 1, &task2, 0);
}

void loop() 
{
  //client.poll();
}