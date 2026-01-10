#ifndef BUTTON_HANDLER_H
#define BUTTON_HANDLER_H

#include <Arduino.h>
//#include <functional>

const int upButtonPin = 4;
const int downButtonPin = 2; 
const int leftButtonPin = 3; 
const int rightButtonPin = 5;

class ButtonHandler {

  public:
    ButtonHandler(int pin, void (*callback)());
    void update();
    bool getState();
    bool isHeld(); // Check if button is currently held
    unsigned long getHoldDuration(); // Get how long button has been held

  private:
    int m_pin;

    int buttonState = LOW;
    int lastButtonState = LOW;
    unsigned long lastDebounceTime = 0;
    unsigned long buttonPressTime = 0; // Time when button was first pressed
    bool buttonIsHeld = false;
    unsigned long lastRepeatTime = 0; // Time of last repeat action

    void (*m_callback)();
};

#endif