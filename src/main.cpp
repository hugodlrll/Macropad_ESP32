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

BleKeyboard bleKeyboard;
int button = 27;

const char* ssid = "wifirobot"; //Enter SSID
const char* password = "16121998"; //Enter Password
const char* websockets_server_host = "192.168.1.37"; //Enter server adress
const uint16_t websockets_server_port = 8050; // Enter server port

using namespace websockets;

void onMessageCallback(WebsocketsMessage message) {
    Serial.print("Got Message: ");
    Serial.println(message.data());
}

void onEventsCallback(WebsocketsEvent event, String data) {
    if(event == WebsocketsEvent::ConnectionOpened) {
        Serial.println("Connnection Opened");
    } else if(event == WebsocketsEvent::ConnectionClosed) {
        Serial.println("Connnection Closed");
    } else if(event == WebsocketsEvent::GotPing) {
        Serial.println("Got a Ping!");
    } else if(event == WebsocketsEvent::GotPong) {
        Serial.println("Got a Pong!");
    }
}

WebsocketsClient client;
void setup() {
    Serial.begin(115200);
    // Connect to wifi
    WiFi.begin(ssid, password);

    // Wait some time to connect to wifi
    for(int i = 0; i < 10 && WiFi.status() != WL_CONNECTED; i++) {
        Serial.print(".");
        delay(1000);
    }

    // run callback when messages are received
    client.onMessage(onMessageCallback);
    
    // run callback when events are occuring
    client.onEvent(onEventsCallback);

    // Connect to server
    client.connect(websockets_server_host, websockets_server_port, "/");

    // Send a message
    client.send("Hello Server");

    // Send a ping
    client.ping();

    Serial.begin(115200);
    pinMode(button, INPUT_PULLUP);
    bleKeyboard.begin();
}

void loop() {
  client.poll();
  if (digitalRead(button) == LOW) 
  {
    Serial.println("Button is pressed");
    bleKeyboard.press(KEY_LEFT_CTRL);
    bleKeyboard.press(KEY_LEFT_ALT);
    bleKeyboard.press(KEY_DELETE);
    delay(20);
    bleKeyboard.releaseAll();
    while(digitalRead(button) == LOW);
  }

}