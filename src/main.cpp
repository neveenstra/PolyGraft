/*
TODO:
threaded insert for bleed port
angle display?
add ribs to bosses
*/

#include <MIDI.h>
#include <Arduino.h>
#include "SensorCache.h"
#include "DisplayHandler.h"
#include "UserSettings.h"
#include "ButtonHandler.h"

MIDI_CREATE_INSTANCE(HardwareSerial, Serial5, hwMIDI);

SensorCache sensors;
UserSettings settings;
DisplayHandler display(sensors, settings);
ButtonHandler buttonUp(upButtonPin, [](){display.pressUp();});
ButtonHandler buttonDown(downButtonPin, [](){display.pressDown();});
ButtonHandler buttonEnter(enterButtonPin, [](){display.pressEnter();});
ButtonHandler buttonBack(backButtonPin, [](){display.pressBack();});


void setup() {
  Serial.begin(115200);
  sensors.begin();
  display.begin();
  settings.begin();
  
  hwMIDI.begin(MIDI_CHANNEL_OMNI);
}

void handle(){

  //display.write("poop");

}

void sendMidi(){
  sensors.update();
  
  float breath = constrain(sensors.getBreathNormalized() * settings.getCalBreath(), 0, 1);
  float expression = constrain(sensors.getExpressionNormalized() * settings.getCalExp(), 0, 1);
  float pinch = constrain(sensors.getPinchNormalized() * settings.getCalPinch(), 0, 1);
  float tilt = constrain(sensors.getGyroX() * settings.getCalTilt(), 0, 1);
  float nod = constrain(sensors.getGyroY() * settings.getCalNod(), 0, 1);
    
  uint8_t breathCC = breath * 127;
  uint8_t expressionCC = expression * 127;
  uint8_t pinchCC = pinch * 127;
  uint8_t tiltCC = tilt * 127;
  uint8_t nodCC = nod * 127;

  if (settings.getUsbMidiEnabled()) {
    usbMIDI.sendControlChange(settings.getBreathCC(), breathCC, settings.getMidiChannel());
    usbMIDI.sendControlChange(settings.getExpCC(), expressionCC, settings.getMidiChannel());
    usbMIDI.sendControlChange(settings.getPinchCC(), pinchCC, settings.getMidiChannel());
    usbMIDI.sendControlChange(settings.getTiltCC(), tiltCC, settings.getMidiChannel());
    usbMIDI.sendControlChange(settings.getNodCC(), nodCC, settings.getMidiChannel());
  }

  if (settings.getHwMidiEnabled()) {
    hwMIDI.sendControlChange(settings.getBreathCC(), breathCC, settings.getMidiChannel());
    hwMIDI.sendControlChange(settings.getExpCC(), expressionCC, settings.getMidiChannel());
    hwMIDI.sendControlChange(settings.getPinchCC(), pinchCC, settings.getMidiChannel());
    hwMIDI.sendControlChange(settings.getTiltCC(), tiltCC, settings.getMidiChannel());
    hwMIDI.sendControlChange(settings.getNodCC(), nodCC, settings.getMidiChannel());
  }
}

void loop() {
  
  sendMidi();

  buttonUp.update();
  buttonDown.update();
  buttonEnter.update();
  buttonBack.update();

  display.update();

  delay(10);
}
