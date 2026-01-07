#include "UserSettings.h"

void UserSettings::begin() {
  // Check if EEPROM has been initialized with our magic number
  uint32_t magic;
  EEPROM.get(ADDR_MAGIC, magic);
  
  if (magic != MAGIC_NUMBER) {
    // First time running - save defaults to EEPROM
    resetToDefaults();
  } else {
    // Load existing settings from EEPROM
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
  EEPROM.get(ADDR_MIDI_CHANNEL, midiChannel);
  EEPROM.get(ADDR_USB_MIDI_EN, usbMidiEnabled);
  EEPROM.get(ADDR_HW_MIDI_EN, hwMidiEnabled);
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
  EEPROM.put(ADDR_MIDI_CHANNEL, midiChannel);
  EEPROM.put(ADDR_USB_MIDI_EN, usbMidiEnabled);
  EEPROM.put(ADDR_HW_MIDI_EN, hwMidiEnabled);
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
  
  screenSleep = 10;
  midiChannel = 1;  // Default MIDI channel 1
  usbMidiEnabled = true;   // USB MIDI on by default
  hwMidiEnabled = true;    // Hardware MIDI on by default
  
  saveAll();
}

// Calibration setters
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

// MIDI CC setters
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

// Display settings setter
void UserSettings::setScreenSleep(int value) {
  screenSleep = value;
  EEPROM.put(ADDR_SCREEN_SLEEP, screenSleep);
}

// MIDI settings setter
void UserSettings::setMidiChannel(int value) {
  // Constrain to valid MIDI channels (1-16)
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