#include "ButtonHandler.h"

const unsigned long debounceDelay = 50;
const unsigned long holdThreshold = 500; // Time before hold repeat starts (ms)
const unsigned long initialRepeatDelay = 200; // Initial delay between repeats (ms)
const unsigned long minRepeatDelay = 50; // Minimum delay between repeats (ms)
const unsigned long accelerationTime = 2000; // Time to reach max speed (ms)

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
        // Button just pressed
        buttonPressTime = millis();
        buttonIsHeld = true;
        lastRepeatTime = millis();
        m_callback(); // Trigger initial press
      } else {
        // Button released
        buttonIsHeld = false;
      }
    } else if (buttonState == LOW && buttonIsHeld) {
      // Button is being held
      unsigned long holdDuration = millis() - buttonPressTime;
      
      if (holdDuration >= holdThreshold) {
        // Calculate repeat delay with progressive acceleration
        unsigned long progressTime = holdDuration - holdThreshold;
        unsigned long repeatDelay;
        
        if (progressTime >= accelerationTime) {
          repeatDelay = minRepeatDelay;
        } else {
          // Linear interpolation from initialRepeatDelay to minRepeatDelay
          repeatDelay = initialRepeatDelay - 
                       ((initialRepeatDelay - minRepeatDelay) * progressTime / accelerationTime);
        }
        
        if (millis() - lastRepeatTime >= repeatDelay) {
          lastRepeatTime = millis();
          m_callback(); // Trigger repeat action
        }
      }
    }
  }

  lastButtonState = reading;
}

bool ButtonHandler::isHeld() {
  return buttonIsHeld;
}

unsigned long ButtonHandler::getHoldDuration() {
  if (buttonIsHeld) {
    return millis() - buttonPressTime;
  }
  return 0;
}