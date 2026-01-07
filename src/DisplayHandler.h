#ifndef DISPLAY_HANDLER_H
#define DISPLAY_HANDLER_H

#include <Arduino.h>
#include <Wire.h>

#define Wire Wire1

#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>
#include "SensorCache.h"

#include "UserSettings.h"

enum class LCDStates {
	DISPLAY_OFF,
	DISPLAY_ON
};

enum class MenuStates {
	LOADING,
	MENU,
	DIAGNOSTIC,
	CONFIG
};

enum class InputStates {
	NAVIGATE,
	MODIFY
};

class DisplayHandler {

  public:
    DisplayHandler(SensorCache& sensorCache, UserSettings& userSettings):m_sensorCache(sensorCache), m_userSettings(userSettings){}
    void begin();
    void update();
    void pressUp();
    void pressDown();
    void pressEnter();
    void pressBack();
    //void write(const char* sayThis);

  private:
    void changeMenuState(MenuStates newState);
    void showLoadingScreen();
    void showMenu();
    void showDiagnosticScreen();
    void showConfigScreen();
    void sleep();
    void wake();
    void setSleepTimeout(unsigned long timeout);


    const SensorCache& m_sensorCache;
    const UserSettings& m_userSettings;
    hd44780_I2Cexp lcd;
    LCDStates lcdState = LCDStates::DISPLAY_ON;
    MenuStates menuState = MenuStates::LOADING;
    InputStates inputState = InputStates::NAVIGATE;
    int menuSelection = 0;
    unsigned long lastActivityTime = 0;
    unsigned long stateStartTime = 0;
    unsigned long sleepTimeout = 10000;

};
#endif