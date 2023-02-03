#include <Arduino.h>
#include <BleKeyboard.h>
#include <MatrixKeypad.h>

BleKeyboard bleKeyboard;

class KeyConfig
{
public:
    int KeyNumber;
    int KeyInput1;
    int KeyInput2;
    int KeyInput3;
    int KeyInput4;
    int IsMedia;
    uint8_t MediaInput1[2];
    uint8_t MediaInput2[2];
    int ENCP;
    int ENCM;

    void SendInput()
    {
        if (IsMedia)
        {
            ApplyMediaInput();
        }
        else
        {
            ApplyKeyInput();
        }
    }

    void ApplyKeyInput()
    {
        bleKeyboard.press(KeyInput1);
        bleKeyboard.press(KeyInput2);
        bleKeyboard.press(KeyInput3);
        delay(20);
        bleKeyboard.releaseAll();
        delay(20);
    }

    void ApplyMediaInput()
    {
        bleKeyboard.write(MediaInput1);
        delay(20);
    }

    void EncInput()
    {
        if(digitalRead(ENCP)==LOW)
        {
            bleKeyboard.write(MediaInput1);
            delay(60);
        }
        else if(digitalRead(ENCM)==LOW)
        {
            bleKeyboard.write(MediaInput2);
            delay(60);
        }
    }
};