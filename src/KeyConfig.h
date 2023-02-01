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
    int IsMedia;
    uint8_t MediaInput[2];
    int PinNumber;

    void SendInput()
    {
        Serial.printf("KeyNumber = %d, isMedia = %d\n", KeyNumber, IsMedia);
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
        Serial.printf("KeyInput1 = %d\n", KeyInput1);
        Serial.printf("KeyInput2 = %d\n", KeyInput2);
        Serial.printf("KeyInput3 = %d\n", KeyInput3);
        delay(20);
        bleKeyboard.releaseAll();
        delay(20);
    }

    void ApplyMediaInput()
    {
        bleKeyboard.write(MediaInput);
        delay(20);
    }
};