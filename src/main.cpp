// #include "BluetoothSerial.h"
// #include <Arduino.h>
// #include <BleKeyboard.h>
// #include <Keypad.h>

// BluetoothSerial SerialBT;
// BleKeyboard bleKeyboard;
// int button = 34;

// void setup() {
//   Serial.begin(115200);
//   pinMode(button, INPUT_PULLUP);
//   bleKeyboard.begin();
//   Serial.println("Starting BLE work!");
//   SerialBT.begin("ESP32test"); //Bluetooth device name
//   Serial.println("The device started, now you can pair it with bluetooth!");
// }

// void loop() {
//   if (Serial.available()) 
//   {
//     SerialBT.write(Serial.read());
//   }
//   if (SerialBT.available())
//   {
//     Serial.write(SerialBT.read());
//   }
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
#include <WebSocketClient.h>


const char* ssid     = "wifirobot";
const char* password = "5cjWSgq7sefAnnJq";
char path[] = "/";
char host[] = "localhost";

WebSocketClient webSocketClient;

// Use WiFiClient class to create TCP connections
WiFiClient client;

void setup() {
  Serial.begin(115200);
  delay(10);

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println("Wifi-local");
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  delay(5000);
  

  // Connect to the websocket server
  if (client.connect("192.168.10.58", 8050)) {
    Serial.println("Connected");
  } else {
    Serial.println("Connection failed.");
    while(1) {
      // Hang on failure
    }
  }

  // Handshake with the server
  webSocketClient.path = path;
  webSocketClient.host = host;
  if (webSocketClient.handshake(client)) {
    Serial.println("Handshake successful");
  } else {
    Serial.println("Handshake failed.");
    while(1) {
      // Hang on failure
    }  
  }

}


void loop() {
  String data;
      	
  if (client.connected()) {
    
		webSocketClient.sendData( String(  ESP.getFreeHeap()  ).c_str() ); 		
 	  Serial.println(ESP.getFreeHeap());

 
  } else {
  	Serial.println("Client disconnected.");
    while (1) {
      // Hang on disconnect.
 
    }
  }
 
  delay(500);
  
}