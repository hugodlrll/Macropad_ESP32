#include <Arduino.h>
#include <BleKeyboard.h>

BleKeyboard bleKeyboard;

class KeyConfig 
{
    public:
        char KeyNumber;
        char KeyInput1;
        char KeyInput2;
        char KeyInput3;
        String IsMedia;
        uint8_t MediaInput[2];
        int PinNumber;

    void SendInput()
    {
        if(KeyIsPressed() == true)
        {
            InputType();
        }
        while(KeyIsPressed() == true);
    }

    void InputType()
    {
        if(IsMedia == "false")
        {
            ApplyKeyInput();
        }
        if(IsMedia == "true")
        {
            ApplyMediaInput();
        }
    }

    void ApplyKeyInput ()
    {
        bleKeyboard.press(KeyInput1);
        bleKeyboard.press(KeyInput2);
        bleKeyboard.press(KeyInput3);
        delay(20);
        bleKeyboard.releaseAll();
        delay(20);
    }

    void ApplyMediaInput ()
    {
        Serial.println("Media Input : " + String(MediaInput[0]) + " " + String(MediaInput[1]));
        bleKeyboard.write(MediaInput);
        delay(20);
    }

    int KeyIsPressed ()
    {
        if(digitalRead(PinNumber) == LOW)
        {
            return true;
            Serial.println("Key is pressed");
        }
        else
        {
            return false;
        }
    }
    
};