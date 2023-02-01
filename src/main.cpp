// Libraries
#include <WiFi.h>
#include <ArduinoWebsockets.h>
#include <ArduinoJson.h>
#include <Keypad.h>
#include <Arduino.h>
#include "KeyConfig.h"
#include <MatrixKeypad.h>
// #include <EEPROM.h>

// #define EEPROM_SIZE 512

using namespace websockets;

// Classes
WebsocketsClient client;
KeyConfig Touche[9];

MatrixKeypad_t *keypad;

// Tâches
TaskHandle_t Conf;
TaskHandle_t Ble;

// Id Wifi
const char *action;
const char *ssid = /*"wifirobot";*/ "SFR_43A0";            // Enter SSID
const char *password = /*"5cjWSgq7sefAnnJq";*/ "16121998"; // Enter Password
const char *websockets_server_host = "192.168.1.83";       // Enter server adress
const uint16_t websockets_server_port = 8050;              // Enter server port

// Variables pour le clavier matricielle
const byte ROWS = 3;               // 3 rows
const byte COLS = 3;               // 3 columns
byte rowPins[ROWS] = {9, 10, 13};  // connect to the row pinouts of the keypad
byte colPins[COLS] = {25, 26, 27}; // connect to the column pinouts of the keypad
char keymap[ROWS][COLS] = {
    {'0', '1', '2'},
    {'3', '4', '5'},
    {'6', '7', '8'}}; // keymap for the keypad
char Touchepress;

//-------------------------------------------------------------
// Réception des trames JSON
//-------------------------------------------------------------

void ReceivedKeyConfiguration(WebsocketsMessage message)
{
  // Trame JSON reçue
  String JsonString = message.data();
  // création d'un document JSON pour stocker données reçues
  StaticJsonDocument<256> doc;
  // désérialisation de la trame JSON
  DeserializationError error = deserializeJson(doc, JsonString);
  if (error)
  {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  }
  JsonObject TOUCHE = doc["action"];
  // récupération des données reçues
  int TOUCHE_KeyNumber = doc["keynumber"];
  int TOUCHE_IsMedia = doc["isMedia"];
  int TOUCHE_Key1 = doc["key1"];
  int TOUCHE_Key2 = doc["key2"];
  int TOUCHE_Key3 = doc["key3"];
  // Serial.println("KeyNumber : " + String(TOUCHE_KeyNumber));
  // Serial.printf("ReceivedKey1 = %d\n", TOUCHE_Key1);
  // Serial.printf("ReceivedKey2 = %d\n", TOUCHE_Key2);
  // Serial.printf("ReceivedKey3 = %d\n", TOUCHE_Key3);
  // Serial.printf("ReceivedIsMedia = %d\n", TOUCHE_IsMedia);
  Touche[TOUCHE_KeyNumber].IsMedia = TOUCHE_IsMedia;
  Touche[TOUCHE_KeyNumber].KeyNumber = TOUCHE_KeyNumber;

  if (TOUCHE_IsMedia)
  {
    Serial.printf("MediaInput");
    Touche[TOUCHE_KeyNumber].MediaInput[0] = TOUCHE_Key1;
    Touche[TOUCHE_KeyNumber].MediaInput[1] = TOUCHE_Key2;
  }
  else
  {
    Serial.printf("KeyInput");
    Touche[TOUCHE_KeyNumber].KeyInput1 = TOUCHE_Key1;
    Touche[TOUCHE_KeyNumber].KeyInput2 = TOUCHE_Key2;
    Touche[TOUCHE_KeyNumber].KeyInput3 = TOUCHE_Key3;
  }
}

//

// Détecte l'appui d'une touche du clavier matricielle
void ToucheAppuyee()
{
  MatrixKeypad_scan(keypad);       // scans for a key press event
  if (MatrixKeypad_hasKey(keypad)) // if a key was pressed
  {
    Touchepress = MatrixKeypad_getKey(keypad); // get the key that was pressed
    // char to int touchpress
    int TouchepressInt = Touchepress - '0';
    if (TouchepressInt >= 0 && TouchepressInt <= 8)
    {
      Touche[TouchepressInt].SendInput();
    }
    else
    {
      return; // error
    }
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
    ToucheAppuyee();
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

  // EEPROM.begin(EEPROM_SIZE);

  // Initialisation du clavier
  keypad = MatrixKeypad_create((char *)keymap, rowPins, colPins, ROWS, COLS); // creates the keypad object

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
    ReceivedKeyConfiguration(message); });

  // Start tasks
  xTaskCreatePinnedToCore(Configuration, "task1", 10000, NULL, 1, &Conf, 1);
  xTaskCreatePinnedToCore(Bluetooth, "task2", 10000, NULL, 1, &Ble, 0);
}

void loop() {}