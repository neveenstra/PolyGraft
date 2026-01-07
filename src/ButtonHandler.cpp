#include "ButtonHandler.h"

const unsigned long debounceDelay = 50;

ButtonHandler::ButtonHandler(int pin, void (*callback)()):m_pin(pin), m_callback(callback){
    pinMode(pin, INPUT_PULLUP);
}

void ButtonHandler::update() {
  int reading = digitalRead(m_pin);

  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;

      if (buttonState == LOW) {
        buttonState = LOW;
        m_callback();
      }
    }
  }

  lastButtonState = reading;
}