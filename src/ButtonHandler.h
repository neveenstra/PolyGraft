#ifndef BUTTON_HANDLER_H
#define BUTTON_HANDLER_H

#include <Arduino.h>
//#include <functional>

const int upButtonPin = 2;
const int downButtonPin = 3; 
const int enterButtonPin = 4; 
const int backButtonPin = 5;

class ButtonHandler {

  public:
    ButtonHandler(int pin, void (*callback)());
    void update();
    bool getState();

  private:
    int m_pin;

    int buttonState = LOW;
    int lastButtonState = LOW;
    unsigned long lastDebounceTime = 0;

    void (*m_callback)();
};

#endif