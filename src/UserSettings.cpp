#include "UserSettings.h"

void UserSettings::begin() {
  uint32_t magic;
  EEPROM.get(ADDR_MAGIC, magic);
  
  if (magic != MAGIC_NUMBER) {
    resetToDefaults();
  } else {
    loadFromEEPROM();
  }
}

void UserSettings::loadFromEEPROM() {
  EEPROM.get(ADDR_CAL_BREATH, calBreath);
  EEPROM.get(ADDR_CAL_PINCH, calPinch);
  EEPROM.get(ADDR_CAL_EXP, calExp);
  EEPROM.get(ADDR_CAL_TILT, calTilt);
  EEPROM.get(ADDR_CAL_NOD, calNod);
  
  EEPROM.get(ADDR_BREATH_CC, breathCC);
  EEPROM.get(ADDR_PINCH_CC, pinchCC);
  EEPROM.get(ADDR_EXP_CC, expCC);
  EEPROM.get(ADDR_TILT_CC, tiltCC);
  EEPROM.get(ADDR_NOD_CC, nodCC);
  
  EEPROM.get(ADDR_SCREEN_SLEEP, screenSleep);
  EEPROM.get(ADDR_DISPLAY_BRIGHTNESS, displayBrightness);
  EEPROM.get(ADDR_MIDI_CHANNEL, midiChannel);
  EEPROM.get(ADDR_USB_MIDI_EN, usbMidiEnabled);
  EEPROM.get(ADDR_HW_MIDI_EN, hwMidiEnabled);
  
  EEPROM.get(ADDR_BREATH_CURVE, breathCurve);
  EEPROM.get(ADDR_PINCH_CURVE, pinchCurve);
  EEPROM.get(ADDR_EXP_CURVE, expCurve);
  EEPROM.get(ADDR_TILT_CURVE, tiltCurve);
  EEPROM.get(ADDR_NOD_CURVE, nodCurve);
  
  EEPROM.get(ADDR_BREATH_FLOOR, breathFloor);
  EEPROM.get(ADDR_PINCH_FLOOR, pinchFloor);
  EEPROM.get(ADDR_EXP_FLOOR, expFloor);
  EEPROM.get(ADDR_TILT_FLOOR, tiltFloor);
  EEPROM.get(ADDR_NOD_FLOOR, nodFloor);
  
  EEPROM.get(ADDR_BREATH_CEILING, breathCeiling);
  EEPROM.get(ADDR_PINCH_CEILING, pinchCeiling);
  EEPROM.get(ADDR_EXP_CEILING, expCeiling);
  EEPROM.get(ADDR_TILT_CEILING, tiltCeiling);
  EEPROM.get(ADDR_NOD_CEILING, nodCeiling);
}

void UserSettings::saveAll() {
  EEPROM.put(ADDR_CAL_BREATH, calBreath);
  EEPROM.put(ADDR_CAL_PINCH, calPinch);
  EEPROM.put(ADDR_CAL_EXP, calExp);
  EEPROM.put(ADDR_CAL_TILT, calTilt);
  EEPROM.put(ADDR_CAL_NOD, calNod);
  
  EEPROM.put(ADDR_BREATH_CC, breathCC);
  EEPROM.put(ADDR_PINCH_CC, pinchCC);
  EEPROM.put(ADDR_EXP_CC, expCC);
  EEPROM.put(ADDR_TILT_CC, tiltCC);
  EEPROM.put(ADDR_NOD_CC, nodCC);
  
  EEPROM.put(ADDR_SCREEN_SLEEP, screenSleep);
  EEPROM.put(ADDR_DISPLAY_BRIGHTNESS, displayBrightness);
  EEPROM.put(ADDR_MIDI_CHANNEL, midiChannel);
  EEPROM.put(ADDR_USB_MIDI_EN, usbMidiEnabled);
  EEPROM.put(ADDR_HW_MIDI_EN, hwMidiEnabled);
  
  EEPROM.put(ADDR_BREATH_CURVE, breathCurve);
  EEPROM.put(ADDR_PINCH_CURVE, pinchCurve);
  EEPROM.put(ADDR_EXP_CURVE, expCurve);
  EEPROM.put(ADDR_TILT_CURVE, tiltCurve);
  EEPROM.put(ADDR_NOD_CURVE, nodCurve);
  
  EEPROM.put(ADDR_BREATH_FLOOR, breathFloor);
  EEPROM.put(ADDR_PINCH_FLOOR, pinchFloor);
  EEPROM.put(ADDR_EXP_FLOOR, expFloor);
  EEPROM.put(ADDR_TILT_FLOOR, tiltFloor);
  EEPROM.put(ADDR_NOD_FLOOR, nodFloor);
  
  EEPROM.put(ADDR_BREATH_CEILING, breathCeiling);
  EEPROM.put(ADDR_PINCH_CEILING, pinchCeiling);
  EEPROM.put(ADDR_EXP_CEILING, expCeiling);
  EEPROM.put(ADDR_TILT_CEILING, tiltCeiling);
  EEPROM.put(ADDR_NOD_CEILING, nodCeiling);
  
  // Write magic and version LAST - if power is lost during save,
  // next boot will see invalid magic and reset to defaults
  EEPROM.put(ADDR_VERSION, FIRMWARE_VERSION);
  EEPROM.put(ADDR_MAGIC, MAGIC_NUMBER);
}

void UserSettings::resetToDefaults() {
  calBreath = 1.0;
  calPinch = 1.0;
  calExp = 1.0;
  calTilt = 1.0;
  calNod = 1.0;
  
  breathCC = 1;
  pinchCC = 2;
  expCC = 3;
  tiltCC = 4;
  nodCC = 5;
  
  screenSleep = 30;
  displayBrightness = 176; // Default to level 7 (26 + 6*25 = 176)
  midiChannel = 1;
  usbMidiEnabled = true;
  hwMidiEnabled = true;
  
  breathCurve = 1;
  pinchCurve = 1;
  expCurve = 1;
  tiltCurve = 1;
  nodCurve = 1;
  
  breathFloor = 0.0;
  breathCeiling = 1.0;
  pinchFloor = 0.0;
  pinchCeiling = 1.0;
  expFloor = 0.0;
  expCeiling = 1.0;
  tiltFloor = 0.0;
  tiltCeiling = 1.0;
  nodFloor = 0.0;
  nodCeiling = 1.0;
  
  saveAll();
}

void UserSettings::setCalBreath(float value) {
  calBreath = value;
  EEPROM.put(ADDR_CAL_BREATH, calBreath);
}

void UserSettings::setCalPinch(float value) {
  calPinch = value;
  EEPROM.put(ADDR_CAL_PINCH, calPinch);
}

void UserSettings::setCalExp(float value) {
  calExp = value;
  EEPROM.put(ADDR_CAL_EXP, calExp);
}

void UserSettings::setCalTilt(float value) {
  calTilt = value;
  EEPROM.put(ADDR_CAL_TILT, calTilt);
}

void UserSettings::setCalNod(float value) {
  calNod = value;
  EEPROM.put(ADDR_CAL_NOD, calNod);
}

void UserSettings::setBreathCC(int value) {
  breathCC = value;
  EEPROM.put(ADDR_BREATH_CC, breathCC);
}

void UserSettings::setPinchCC(int value) {
  pinchCC = value;
  EEPROM.put(ADDR_PINCH_CC, pinchCC);
}

void UserSettings::setExpCC(int value) {
  expCC = value;
  EEPROM.put(ADDR_EXP_CC, expCC);
}

void UserSettings::setTiltCC(int value) {
  tiltCC = value;
  EEPROM.put(ADDR_TILT_CC, tiltCC);
}

void UserSettings::setNodCC(int value) {
  nodCC = value;
  EEPROM.put(ADDR_NOD_CC, nodCC);
}

void UserSettings::setScreenSleep(int value) {
  screenSleep = value;
  EEPROM.put(ADDR_SCREEN_SLEEP, screenSleep);
}

void UserSettings::setDisplayBrightness(int value) {
  if (value >= 0 && value <= 255) {
    displayBrightness = value;
    EEPROM.put(ADDR_DISPLAY_BRIGHTNESS, displayBrightness);
  }
}

void UserSettings::setMidiChannel(int value) {
  if (value >= 1 && value <= 16) {
    midiChannel = value;
    EEPROM.put(ADDR_MIDI_CHANNEL, midiChannel);
  }
}

void UserSettings::setUsbMidiEnabled(bool enabled) {
  usbMidiEnabled = enabled;
  EEPROM.put(ADDR_USB_MIDI_EN, usbMidiEnabled);
}

void UserSettings::setHwMidiEnabled(bool enabled) {
  hwMidiEnabled = enabled;
  EEPROM.put(ADDR_HW_MIDI_EN, hwMidiEnabled);
}

void UserSettings::setBreathCurve(int value) {
  if (value >= 1 && value <= 10) {
    breathCurve = value;
    EEPROM.put(ADDR_BREATH_CURVE, breathCurve);
  }
}

void UserSettings::setPinchCurve(int value) {
  if (value >= 1 && value <= 10) {
    pinchCurve = value;
    EEPROM.put(ADDR_PINCH_CURVE, pinchCurve);
  }
}

void UserSettings::setExpCurve(int value) {
  if (value >= 1 && value <= 10) {
    expCurve = value;
    EEPROM.put(ADDR_EXP_CURVE, expCurve);
  }
}

void UserSettings::setTiltCurve(int value) {
  if (value >= 1 && value <= 10) {
    tiltCurve = value;
    EEPROM.put(ADDR_TILT_CURVE, tiltCurve);
  }
}

void UserSettings::setNodCurve(int value) {
  if (value >= 1 && value <= 10) {
    nodCurve = value;
    EEPROM.put(ADDR_NOD_CURVE, nodCurve);
  }
}

void UserSettings::setBreathFloor(float value) {
  if (value >= 0.0 && value <= 1.0) {
    breathFloor = value;
    EEPROM.put(ADDR_BREATH_FLOOR, breathFloor);
  }
}

void UserSettings::setPinchFloor(float value) {
  if (value >= 0.0 && value <= 1.0) {
    pinchFloor = value;
    EEPROM.put(ADDR_PINCH_FLOOR, pinchFloor);
  }
}

void UserSettings::setExpFloor(float value) {
  if (value >= 0.0 && value <= 1.0) {
    expFloor = value;
    EEPROM.put(ADDR_EXP_FLOOR, expFloor);
  }
}

void UserSettings::setTiltFloor(float value) {
  if (value >= 0.0 && value <= 1.0) {
    tiltFloor = value;
    EEPROM.put(ADDR_TILT_FLOOR, tiltFloor);
  }
}

void UserSettings::setNodFloor(float value) {
  if (value >= 0.0 && value <= 1.0) {
    nodFloor = value;
    EEPROM.put(ADDR_NOD_FLOOR, nodFloor);
  }
}

void UserSettings::setBreathCeiling(float value) {
  if (value >= 0.0 && value <= 1.0) {
    breathCeiling = value;
    EEPROM.put(ADDR_BREATH_CEILING, breathCeiling);
  }
}

void UserSettings::setPinchCeiling(float value) {
  if (value >= 0.0 && value <= 1.0) {
    pinchCeiling = value;
    EEPROM.put(ADDR_PINCH_CEILING, pinchCeiling);
  }
}

void UserSettings::setExpCeiling(float value) {
  if (value >= 0.0 && value <= 1.0) {
    expCeiling = value;
    EEPROM.put(ADDR_EXP_CEILING, expCeiling);
  }
}

void UserSettings::setTiltCeiling(float value) {
  if (value >= 0.0 && value <= 1.0) {
    tiltCeiling = value;
    EEPROM.put(ADDR_TILT_CEILING, tiltCeiling);
  }
}

void UserSettings::setNodCeiling(float value) {
  if (value >= 0.0 && value <= 1.0) {
    nodCeiling = value;
    EEPROM.put(ADDR_NOD_CEILING, nodCeiling);
  }
}