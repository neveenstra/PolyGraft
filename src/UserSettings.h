#ifndef USER_SETTINGS_H
#define USER_SETTINGS_H

#include <EEPROM.h>

class UserSettings {

  public:
    // Initialize and load settings from EEPROM
    void begin();
    
    // Getters
    float getCalBreath() const { return calBreath; }
    float getCalPinch() const { return calPinch; }
    float getCalExp() const { return calExp; }
    float getCalTilt() const { return calTilt; }
    float getCalNod() const { return calNod; }
    
    int getBreathCurve() const { return breathCurve; }
    int getPinchCurve() const { return pinchCurve; }
    int getExpCurve() const { return expCurve; }
    int getTiltCurve() const { return tiltCurve; }
    int getNodCurve() const { return nodCurve; }
    
    float getBreathFloor() const { return breathFloor; }
    float getBreathCeiling() const { return breathCeiling; }
    float getPinchFloor() const { return pinchFloor; }
    float getPinchCeiling() const { return pinchCeiling; }
    float getExpFloor() const { return expFloor; }
    float getExpCeiling() const { return expCeiling; }
    float getTiltFloor() const { return tiltFloor; }
    float getTiltCeiling() const { return tiltCeiling; }
    float getNodFloor() const { return nodFloor; }
    float getNodCeiling() const { return nodCeiling; }
    
    int getBreathCC() const { return breathCC; }
    int getPinchCC() const { return pinchCC; }
    int getExpCC() const { return expCC; }
    int getTiltCC() const { return tiltCC; }
    int getNodCC() const { return nodCC; }
    
    int getScreenSleep() const { return screenSleep; }
    int getDisplayBrightness() const { return displayBrightness; }
    int getMidiChannel() const { return midiChannel; }
    bool getUsbMidiEnabled() const { return usbMidiEnabled; }
    bool getHwMidiEnabled() const { return hwMidiEnabled; }

    // Setters that save to EEPROM
    void setCalBreath(float value);
    void setCalPinch(float value);
    void setCalExp(float value);
    void setCalTilt(float value);
    void setCalNod(float value);
    
    void setBreathCurve(int value);
    void setPinchCurve(int value);
    void setExpCurve(int value);
    void setTiltCurve(int value);
    void setNodCurve(int value);
    
    void setBreathFloor(float value);
    void setBreathCeiling(float value);
    void setPinchFloor(float value);
    void setPinchCeiling(float value);
    void setExpFloor(float value);
    void setExpCeiling(float value);
    void setTiltFloor(float value);
    void setTiltCeiling(float value);
    void setNodFloor(float value);
    void setNodCeiling(float value);
    
    void setBreathCC(int value);
    void setPinchCC(int value);
    void setExpCC(int value);
    void setTiltCC(int value);
    void setNodCC(int value);
    
    void setScreenSleep(int value);
    void setDisplayBrightness(int value);
    void setMidiChannel(int value);
    void setUsbMidiEnabled(bool enabled);
    void setHwMidiEnabled(bool enabled);
    
    // Save all settings to EEPROM
    void saveAll();
    
    // Reset all settings to defaults
    void resetToDefaults();
    
    // Public constants
    static constexpr uint32_t FIRMWARE_VERSION = 1; // Increment this to force reset on new uploads

  private:
    // Calibration values
    float calBreath = 1.0;
    float calPinch = 1.0;
    float calExp = 1.0;
    float calTilt = 1.0;
    float calNod = 1.0;
    
    // Curve settings (1-10)
    uint8_t breathCurve = 5;
    uint8_t pinchCurve = 5;
    uint8_t expCurve = 5;
    uint8_t tiltCurve = 5;
    uint8_t nodCurve = 5;
    
    // Floor and ceiling offsets (0.0-1.0)
    float breathFloor = 0.0;
    float breathCeiling = 1.0;
    float pinchFloor = 0.0;
    float pinchCeiling = 1.0;
    float expFloor = 0.0;
    float expCeiling = 1.0;
    float tiltFloor = 0.0;
    float tiltCeiling = 1.0;
    float nodFloor = 0.0;
    float nodCeiling = 1.0;

    // MIDI CC assignments
    uint8_t breathCC = 1;
    uint8_t pinchCC = 2;
    uint8_t expCC = 3;
    uint8_t tiltCC = 4;
    uint8_t nodCC = 5;

    // Display settings
    uint16_t screenSleep = 10;
    uint8_t displayBrightness = 255;
    
    // MIDI settings
    uint8_t midiChannel = 1;           // Default to channel 1
    bool usbMidiEnabled = true;    // USB MIDI enabled by default
    bool hwMidiEnabled = true;     // Hardware MIDI enabled by default
    
    // EEPROM memory addresses (float = 4 bytes, uint8_t = 1 byte, uint16_t = 2 bytes, bool = 1 byte)
    static const int ADDR_CAL_BREATH = 0;    // 0-3
    static const int ADDR_CAL_PINCH = 4;     // 4-7
    static const int ADDR_CAL_EXP = 8;       // 8-11
    static const int ADDR_CAL_TILT = 12;     // 12-15
    static const int ADDR_CAL_NOD = 16;      // 16-19
    
    static const int ADDR_BREATH_CC = 20;    // 20
    static const int ADDR_PINCH_CC = 21;     // 21
    static const int ADDR_EXP_CC = 22;       // 22
    static const int ADDR_TILT_CC = 23;      // 23
    static const int ADDR_NOD_CC = 24;       // 24
    
    static const int ADDR_SCREEN_SLEEP = 25; // 25-26
    static const int ADDR_DISPLAY_BRIGHTNESS = 27; // 27
    static const int ADDR_MIDI_CHANNEL = 28; // 28
    static const int ADDR_USB_MIDI_EN = 29;  // 29
    static const int ADDR_HW_MIDI_EN = 30;   // 30
    
    // Curve settings
    static const int ADDR_BREATH_CURVE = 31; // 31
    static const int ADDR_PINCH_CURVE = 32;  // 32
    static const int ADDR_EXP_CURVE = 33;    // 33
    static const int ADDR_TILT_CURVE = 34;   // 34
    static const int ADDR_NOD_CURVE = 35;    // 35
    
    // Floor offsets
    static const int ADDR_BREATH_FLOOR = 36;   // 36-39
    static const int ADDR_PINCH_FLOOR = 40;    // 40-43
    static const int ADDR_EXP_FLOOR = 44;      // 44-47
    static const int ADDR_TILT_FLOOR = 48;     // 48-51
    static const int ADDR_NOD_FLOOR = 52;      // 52-55
    
    // Ceiling offsets
    static const int ADDR_BREATH_CEILING = 56;  // 56-59
    static const int ADDR_PINCH_CEILING = 60;   // 60-63
    static const int ADDR_EXP_CEILING = 64;     // 64-67
    static const int ADDR_TILT_CEILING = 68;    // 68-71
    static const int ADDR_NOD_CEILING = 72;     // 72-75
    
    static const int ADDR_MAGIC = 76;        // 76-79
    static const int ADDR_VERSION = 80;      // 80-83
    
    static constexpr uint32_t MAGIC_NUMBER = 0xCAFEBABE;
    
    // Helper functions
    void loadFromEEPROM();
};

#endif