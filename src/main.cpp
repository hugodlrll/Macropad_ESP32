// Libraries
#include <WiFi.h>
#include <ArduinoWebsockets.h>
#include <ArduinoJson.h>
#include <Keypad.h>
#include <Arduino.h>
#include "KeyConfig.h"
#include <MatrixKeypad.h>
#include <EEPROM.h>

#define EEPROM_SIZE 512
#define ENC1P 5
#define ENC1M 2
#define ENC2P 34
#define ENC2M 35
#define ENC3P 18
#define ENC3M 23

using namespace websockets;

// Classes
WebsocketsClient client;
KeyConfig Touche[12];

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

void ReceivedKeyConfiguration(WebsocketsMessage message, int* TOUCHE_KeyNumber)
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
  *TOUCHE_KeyNumber = doc["keynumber"];
  int TOUCHE_IsMedia = doc["isMedia"];
  int TOUCHE_Key1 = doc["key1"];
  int TOUCHE_Key2 = doc["key2"];
  int TOUCHE_Key3 = doc["key3"];
  int TOUCHE_Key4 = doc["key4"];
  Touche[*TOUCHE_KeyNumber].IsMedia = TOUCHE_IsMedia;
  Touche[*TOUCHE_KeyNumber].KeyNumber = *TOUCHE_KeyNumber;

  if (TOUCHE_IsMedia == 1)
  {
    //Serial.printf("MediaInput");
    Touche[*TOUCHE_KeyNumber].MediaInput1[0] = TOUCHE_Key1;
    Touche[*TOUCHE_KeyNumber].MediaInput1[1] = TOUCHE_Key2;
  }
  else if (TOUCHE_IsMedia == 2)
  {
    Touche[*TOUCHE_KeyNumber].MediaInput1[0] = TOUCHE_Key1;
    Touche[*TOUCHE_KeyNumber].MediaInput1[1] = TOUCHE_Key2;
    Touche[*TOUCHE_KeyNumber].MediaInput2[0] = TOUCHE_Key3;
    Touche[*TOUCHE_KeyNumber].MediaInput2[1] = TOUCHE_Key4;
  }
  else
  {
    //Serial.printf("KeyInput");
    Touche[*TOUCHE_KeyNumber].KeyInput1 = TOUCHE_Key1;
    Touche[*TOUCHE_KeyNumber].KeyInput2 = TOUCHE_Key2;
    Touche[*TOUCHE_KeyNumber].KeyInput3 = TOUCHE_Key3;
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
    Touche[9].EncInput();
    Touche[10].EncInput();
    Touche[11].EncInput();
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
// Save & Load Configuration to EEPROM
//-------------------------------------------------------------

void EEPROMSave(int ToucheId, int Touche1, int Touche2, int Touche3, int ToucheIsMedia, uint8_t Media1[], uint8_t Media2[])
{
  int EEPROMAddress = ToucheId * 5;
  if(ToucheIsMedia == 1)
  {
    EEPROM.write(EEPROMAddress, Media1[0]);
    EEPROM.write(EEPROMAddress + 1, Media1[1]);
    EEPROM.write(EEPROMAddress + 4, ToucheIsMedia);
  }
  else if (ToucheIsMedia == 2)
  {
    Serial.println("Media2");
    EEPROM.write(EEPROMAddress, Media1[0]);
    EEPROM.write(EEPROMAddress + 1, Media1[1]);
    EEPROM.write(EEPROMAddress + 2, Media2[0]);
    EEPROM.write(EEPROMAddress + 3, Media2[1]);
    EEPROM.write(EEPROMAddress + 4, ToucheIsMedia);
  }
  else{
    EEPROM.write(EEPROMAddress, Touche1);
    EEPROM.write(EEPROMAddress + 1, Touche2);
    EEPROM.write(EEPROMAddress + 2, Touche3);
    EEPROM.write(EEPROMAddress + 4, ToucheIsMedia);
  }
  EEPROM.commit();
}

void EEPROMLoad()
{
  for (int i = 0; i < 12; i++)
  {
    int EEPROMAddress = i * 5;
    if(EEPROM.read(EEPROMAddress + 4) == 1)
    {
      Touche[i].MediaInput1[0] = EEPROM.read(EEPROMAddress);
      Touche[i].MediaInput1[1] = EEPROM.read(EEPROMAddress + 1);
      Touche[i].IsMedia = EEPROM.read(EEPROMAddress + 4);
    }
    else if(EEPROM.read(EEPROMAddress + 4) == 2)
    {
      Touche[i].MediaInput1[0] = EEPROM.read(EEPROMAddress);
      Touche[i].MediaInput1[1] = EEPROM.read(EEPROMAddress + 1);
      Touche[i].MediaInput2[0] = EEPROM.read(EEPROMAddress + 2);
      Touche[i].MediaInput2[1] = EEPROM.read(EEPROMAddress + 3);
      Touche[i].IsMedia = EEPROM.read(EEPROMAddress + 4);
    }
    else
    {
      Touche[i].KeyInput1 = EEPROM.read(EEPROMAddress);
      Touche[i].KeyInput2 = EEPROM.read(EEPROMAddress + 1);
      Touche[i].KeyInput3 = EEPROM.read(EEPROMAddress + 2);
      Touche[i].IsMedia = EEPROM.read(EEPROMAddress + 4);
      Serial.printf("1) EEPROMAddress = %d\n EEPROMVAlue = %d\n", EEPROMAddress, Touche[i].KeyInput1);
      Serial.printf("2) EEPROMAddress = %d\n EEPROMVAlue = %d\n", EEPROMAddress + 1, Touche[i].KeyInput2);
      Serial.printf("3) EEPROMAddress = %d\n EEPROMVAlue = %d\n", EEPROMAddress + 2, Touche[i].KeyInput3);
      Serial.printf("Media EEPROMAddress = %d\n EEPROMVAlue = %d\n", EEPROMAddress + 3, Touche[i].IsMedia);
    }
  }
}

//-------------------------------------------------------------
// Setup & Loop
//-------------------------------------------------------------

void setup()
{
  // Start serial
  Serial.begin(115200);

  // Initialisation des encodeurs
  pinMode(ENC1P, INPUT_PULLUP);
  pinMode(ENC1M, INPUT_PULLUP);
  pinMode(ENC2P, INPUT_PULLUP);
  pinMode(ENC2M, INPUT_PULLUP);
  pinMode(ENC3P, INPUT_PULLUP);
  pinMode(ENC3M, INPUT_PULLUP);
  Touche[9].ENCP = ENC1P;
  Touche[9].ENCM = ENC1M;
  Touche[10].ENCP = ENC2P;
  Touche[10].ENCM = ENC2M;
  Touche[11].ENCP = ENC3P;
  Touche[11].ENCM = ENC3M;

  EEPROM.begin(EEPROM_SIZE);
  EEPROMLoad();

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
      int TOUCHE_KeyNumber;
      Serial.println("Message received: " + message.data());
      ReceivedKeyConfiguration(message, &TOUCHE_KeyNumber);
      EEPROMSave(Touche[TOUCHE_KeyNumber].KeyNumber, Touche[TOUCHE_KeyNumber].KeyInput1, Touche[TOUCHE_KeyNumber].KeyInput2, Touche[TOUCHE_KeyNumber].KeyInput3, Touche[TOUCHE_KeyNumber].IsMedia, Touche[TOUCHE_KeyNumber].MediaInput1, Touche[TOUCHE_KeyNumber].MediaInput2); 
    });

  // Start tasks
  xTaskCreatePinnedToCore(Configuration, "task1", 10000, NULL, 1, &Conf, 1);
  xTaskCreatePinnedToCore(Bluetooth, "task2", 10000, NULL, 1, &Ble, 0);
}

void loop() {}