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

MIDI_CREATE_INSTANCE(HardwareSerial, Serial3, hwMIDI);

SensorCache sensors;
UserSettings settings;
DisplayHandler display(sensors, settings);
ButtonHandler buttonUp(upButtonPin, [](){display.pressUp();});
ButtonHandler buttonDown(downButtonPin, [](){display.pressDown();});
ButtonHandler buttonLeft(leftButtonPin, [](){display.pressLeft();});
ButtonHandler buttonRight(rightButtonPin, [](){display.pressRight();});


void setup() {
  Serial.begin(115200);
  display.begin();  // Show loading screen first
  settings.begin();
  sensors.begin();  // IMU autoOffsets happens during loading screen
  
  hwMIDI.begin(MIDI_CHANNEL_OMNI);
}

// Apply curve, floor, and ceiling to sensor value
float applyCurve(float input, int curveType, float floor, float ceiling) {
  // Constrain to 0-1 range first
  input = constrain(input, 0.0, 1.0);
  
  // Apply floor/ceiling trimming
  float output;
  if (input < floor) {
    output = 0.0;
  } else if (input > ceiling) {
    output = 1.0;
  } else {
    // Map input range to 0-1
    float mappedInput = (input - floor) / (ceiling - floor);
    
    // Apply curve type: 1=linear, 2=concave, 3=convex, 4=s-curve
    if (curveType == 1) {
      // Linear
      output = mappedInput;
    } else if (curveType == 2) {
      // Concave (ease-in)
      output = mappedInput * mappedInput;
    } else if (curveType == 3) {
      // Convex (ease-out)
      output = sqrt(mappedInput);
    } else {
      // S-curve (ease-in-out)
      output = mappedInput * mappedInput * (3.0 - 2.0 * mappedInput);
    }
  }
  
  return constrain(output, 0.0, 1.0);
}

void sendMidi(){
  sensors.update();
  
  // Get calibrated sensor values
  float breath = constrain(sensors.getBreathNormalized() * settings.getCalBreath(), 0, 1);
  float expression = constrain(sensors.getExpressionNormalized() * settings.getCalExp(), 0, 1);
  float pinch = constrain(sensors.getPinchNormalized() * settings.getCalPinch(), 0, 1);
  float tilt = constrain(sensors.getGyroX() * settings.getCalTilt(), 0, 1);
  float nod = constrain(sensors.getGyroY() * settings.getCalNod(), 0, 1);
  
  // Apply curve, floor, and ceiling settings
  breath = applyCurve(breath, settings.getBreathCurve(), settings.getBreathFloor(), settings.getBreathCeiling());
  expression = applyCurve(expression, settings.getExpCurve(), settings.getExpFloor(), settings.getExpCeiling());
  pinch = applyCurve(pinch, settings.getPinchCurve(), settings.getPinchFloor(), settings.getPinchCeiling());
  tilt = applyCurve(tilt, settings.getTiltCurve(), settings.getTiltFloor(), settings.getTiltCeiling());
  nod = applyCurve(nod, settings.getNodCurve(), settings.getNodFloor(), settings.getNodCeiling());
    
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
  buttonLeft.update();
  buttonRight.update();

  display.update();

  delay(10);
}
