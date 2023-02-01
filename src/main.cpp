// Libraries
#include <WiFi.h>
#include <ArduinoWebsockets.h>
#include <ArduinoJson.h>
#include <Keypad.h>
#include <Arduino.h>
#include "KeyConfig.h"

using namespace websockets;

// Classes
WebsocketsClient client;
KeyConfig Touche1;
KeyConfig Touche2;
KeyConfig Touche3;

// Variables temporaires pour une trame JSON
char ReceivedKeyNumber;
char ReceivedKey1;
char ReceivedKey2;
char ReceivedKey3;
String IsMedia;
uint8_t ReceivedMedia[2];

// définition des pins
#define PinTouche1 13
#define PinTouche2 10
#define PinTouche3 9

// Tâches
TaskHandle_t Conf;
TaskHandle_t Ble;

// Id Wifi
const char *action;
const char *ssid = "wifirobot";                     //"SFR_43A0";   //Enter SSID
const char *password = "5cjWSgq7sefAnnJq";             //"16121998";   //Enter Password
const char *websockets_server_host = "192.168.10.58"; // Enter server adress
const uint16_t websockets_server_port = 8050;         // Enter server port

//-------------------------------------------------------------
// Réception des trames JSON
//-------------------------------------------------------------
void ConvertKey(const char *Touche1, const char *Touche2, const char *Touche3, const char *NumTouche)
{
  ReceivedKey1 = (char)strtol(Touche1, NULL, 0);
  ReceivedKey2 = (char)strtol(Touche2, NULL, 0);
  ReceivedKey3 = (char)strtol(Touche3, NULL, 0);
  ReceivedKeyNumber = (char)strtol(NumTouche, NULL, 0);
  Serial.println("ReceivedKey1 : " + String(ReceivedKey1));
}

void ConvertMedia(const char *ToucheMedia, uint8_t *media, const char *NumTouche)
{
  ReceivedKeyNumber = (char)strtol(NumTouche, NULL, 0);
  char *copy = strdup(ToucheMedia);
    char *p = strtok(copy, ",");
    int i = 0;
    while (p != NULL)
    {
      media[i++] = atoi(p);
      p = strtok(NULL, ",");
    }
    free(copy);
}

void ReceivedKeyConfiguration(WebsocketsMessage message)
{
  // Trame JSON reçue
  String JsonString = message.data();
  // création d'un document JSON pour stocker données reçues
  StaticJsonDocument<200> doc;
  // désérialisation de la trame JSON
  DeserializationError error = deserializeJson(doc, JsonString);
  if (error)
  {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  }
  JsonObject TOUCHE1 = doc["action"];
  // récupération des données reçues
  const char *TOUCHE1_KeyNumber = doc["keynumber"];
  const char *TOUCHE1_Key1 = doc["key1"];
  const char *TOUCHE1_Key2  = doc["key2"];
  const char *TOUCHE1_Key3 = doc["key3"];
  const char *TOUCHE1_IsMedia = doc["isMedia"];
  IsMedia = TOUCHE1_IsMedia;
  // conversion des données reçues
  if(IsMedia == "true")
  {
    Serial.println("Media " + String(TOUCHE1_Key1));
    ConvertMedia(TOUCHE1_Key1, ReceivedMedia, TOUCHE1_KeyNumber);
  }
  if(IsMedia == "false")
  {
    ConvertKey(TOUCHE1_Key1, TOUCHE1_Key2, TOUCHE1_Key3, TOUCHE1_KeyNumber);
  }
}

void DefineSelectedKey()
{
  if(ReceivedKeyNumber == 1)
  {
    Touche1.KeyInput1 = ReceivedKey1;
    Touche1.KeyInput2 = ReceivedKey2;
    Touche1.KeyInput3 = ReceivedKey3;
    Touche1.IsMedia = IsMedia;
    Touche1.MediaInput[0] = ReceivedMedia[0];
    Touche1.MediaInput[1] = ReceivedMedia[1];
    Serial.println("Touche1");
  }
  else if(ReceivedKeyNumber == 2)
  {
    Touche2.KeyInput1 = ReceivedKey1;
    Touche2.KeyInput2 = ReceivedKey2;
    Touche2.KeyInput3 = ReceivedKey3;
    Touche2.IsMedia = IsMedia;
    Touche2.MediaInput[0] = ReceivedMedia[0];
    Touche2.MediaInput[1] = ReceivedMedia[1];
    Serial.println("Touche2");
  }
  else if(ReceivedKeyNumber == 3)
  {
    Touche3.KeyInput1 = ReceivedKey1;
    Touche3.KeyInput2 = ReceivedKey2;
    Touche3.KeyInput3 = ReceivedKey3;
    Touche3.IsMedia = IsMedia;
    Touche3.MediaInput[0] = ReceivedMedia[0];
    Touche3.MediaInput[1] = ReceivedMedia[1];
    Serial.println("Touche3");
  }
}

//-------------------------------------------------------------
// Tâches exécutées sur les deux coeurs de l'Esp32
//-------------------------------------------------------------

// Tâche pour envoi des inputs bluetooth
void Bluetooth(void *parameter)
{
  for (;;)
  {
    Touche1.SendInput();
    Touche2.SendInput();
    Touche3.SendInput();
    delay(20);
  }
}

// Tâche pour connexion au serveur
void Configuration(void *parameter)
{
  for (;;)
  {
    client.poll();
    delay(20);
  }
}

//-------------------------------------------------------------
// Setup & Loop
//-------------------------------------------------------------

void setup()
{
  // Start serial
  Serial.begin(115200);
  
  // initialisation des pins et assignation aux classes
  Touche1.PinNumber = PinTouche1;
  Touche2.PinNumber = PinTouche2;
  Touche3.PinNumber = PinTouche3;
  pinMode(Touche1.PinNumber, INPUT_PULLUP);
  pinMode(Touche2.PinNumber , INPUT_PULLUP);
  pinMode(Touche3.PinNumber, INPUT_PULLUP);

  // Start Bluetooth
  bleKeyboard.begin();

  // Connect to wifi
  WiFi.begin(ssid, password);

  // Wait some time to connect to wifi
  for (int i = 0; i < 10 && WiFi.status() != WL_CONNECTED; i++)
  {
    Serial.print(".");
    delay(1000);
  }

  // Check if connected to wifi
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("No Wifi!");
    return;
  }

  Serial.println("Connected to Wifi, Connecting to server.");
  // try to connect to Websockets server
  bool connected = client.connect(websockets_server_host, websockets_server_port, "/");
  if (connected)
  {
    Serial.println("Connected!");
    client.send("Hello Server");
  }
  else
  {
    Serial.println("Not Connected !");
  }

  // run callback when messages are received
  client.onMessage([&](WebsocketsMessage message)
  { 
    Serial.println("Message received: " + message.data());
    ReceivedKeyConfiguration(message);
    DefineSelectedKey(); 
  });

  // Start tasks
  xTaskCreatePinnedToCore(Configuration, "task1", 10000, NULL, 1, &Conf, 1);
  xTaskCreatePinnedToCore(Bluetooth, "task2", 10000, NULL, 1, &Ble, 0);
}

void loop() {}
