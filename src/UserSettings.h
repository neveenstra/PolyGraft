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
    
    int getBreathCC() const { return breathCC; }
    int getPinchCC() const { return pinchCC; }
    int getExpCC() const { return expCC; }
    int getTiltCC() const { return tiltCC; }
    int getNodCC() const { return nodCC; }
    
    int getScreenSleep() const { return screenSleep; }
    int getMidiChannel() const { return midiChannel; }
    bool getUsbMidiEnabled() const { return usbMidiEnabled; }
    bool getHwMidiEnabled() const { return hwMidiEnabled; }

    // Setters that save to EEPROM
    void setCalBreath(float value);
    void setCalPinch(float value);
    void setCalExp(float value);
    void setCalTilt(float value);
    void setCalNod(float value);
    
    void setBreathCC(int value);
    void setPinchCC(int value);
    void setExpCC(int value);
    void setTiltCC(int value);
    void setNodCC(int value);
    
    void setScreenSleep(int value);
    void setMidiChannel(int value);
    void setUsbMidiEnabled(bool enabled);
    void setHwMidiEnabled(bool enabled);
    
    // Save all settings to EEPROM
    void saveAll();
    
    // Reset all settings to defaults
    void resetToDefaults();

  private:
    // Calibration values
    float calBreath = 1.0;
    float calPinch = 1.0;
    float calExp = 1.0;
    float calTilt = 1.0;
    float calNod = 1.0;

    // MIDI CC assignments
    int breathCC = 1;
    int pinchCC = 2;
    int expCC = 3;
    int tiltCC = 4;
    int nodCC = 5;

    // Display settings
    int screenSleep = 10;
    
    // MIDI settings
    int midiChannel = 1;           // Default to channel 1
    bool usbMidiEnabled = true;    // USB MIDI enabled by default
    bool hwMidiEnabled = true;     // Hardware MIDI enabled by default
    
    // EEPROM memory addresses (each float = 4 bytes, each int = 2 bytes, each bool = 1 byte)
    static const int ADDR_CAL_BREATH = 0;    // 0-3
    static const int ADDR_CAL_PINCH = 4;     // 4-7
    static const int ADDR_CAL_EXP = 8;       // 8-11
    static const int ADDR_CAL_TILT = 12;     // 12-15
    static const int ADDR_CAL_NOD = 16;      // 16-19
    
    static const int ADDR_BREATH_CC = 20;    // 20-21
    static const int ADDR_PINCH_CC = 22;     // 22-23
    static const int ADDR_EXP_CC = 24;       // 24-25
    static const int ADDR_TILT_CC = 26;      // 26-27
    static const int ADDR_NOD_CC = 28;       // 28-29
    
    static const int ADDR_SCREEN_SLEEP = 30; // 30-31
    static const int ADDR_MIDI_CHANNEL = 32; // 32-33
    static const int ADDR_USB_MIDI_EN = 34;  // 34
    static const int ADDR_HW_MIDI_EN = 35;   // 35
    static const int ADDR_MAGIC = 36;        // 36-39
    
    static constexpr uint32_t MAGIC_NUMBER = 0xCAFEBABE;
    
    // Helper functions
    void loadFromEEPROM();
};

#endif