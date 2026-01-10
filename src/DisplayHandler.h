#ifndef DISPLAY_HANDLER_H
#define DISPLAY_HANDLER_H

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include "SensorCache.h"
#include "UserSettings.h"
#include "logo.h"

// Pin definitions
#define TFT_CS 10
#define TFT_DC 7
#define TFT_RST 9
#define TFT_BL 8

enum class DisplayState {
	DISPLAY_OFF,
	DISPLAY_ON
};

enum class MenuState {
	LOADING,
	MAIN_MENU,
	SUB_MENU,
	EDIT_MODE,
	CONFIRM_DIALOG,
	ABOUT
};

class DisplayHandler {

public:
	DisplayHandler(SensorCache& sensorCache, UserSettings& userSettings);
	void begin();
	void update();
	void pressUp();
	void pressDown();
	void pressLeft();
	void pressRight();

	private:

	void drawMainMenu();
	void drawSubMenu();
	void drawEditMode();
	void drawConfirmDialog();
	void drawAbout();
	void showLoadingScreen();
	void drawDiagnosticScreen();
	void drawSensorValues();
	void drawCalibrationMenu();
	void drawMidiMenu();
	void redrawEditValue(); // Redraw just the value being edited
	void drawResponseCurve(); // Draw curve visualization in sensor detail menu
	void updateCurveSensorIndicator(); // Update live sensor position on curve

	// State management
	void sleep();
	void wake();
	void changeMenuState(MenuState newState);

	// References
	SensorCache& m_sensorCache;
	UserSettings& m_userSettings;

	// Display
	Adafruit_ILI9341 tft;

	// Menu state
	DisplayState displayState = DisplayState::DISPLAY_ON;
	MenuState currentState = MenuState::LOADING;
	int mainMenuSelection = 0;
	int subMenuSelection = 0;
	int thirdMenuSelection = 0;
	int menuDepth = 1; // 1=main, 2=sub, 3=third level
	int editValue = 0;
	float editValueFloat = 0.0;
	bool editingFloat = false;
	bool inlineEditMode = false; // True when editing a value inline
	int pendingResetType = 0; // 0=none, 1=calibration, 2=curves, 3=midi, 4=factory

	// Previous state for partial updates
	int prevMainMenuSelection = -1;
	int prevSubMenuSelection = -1;
	int prevThirdMenuSelection = -1;
	int prevEditValue = -1;
	float prevEditValueFloat = -1.0;
	bool needsFullRedraw = true;
	float prevSensorValue = -1.0; // Previous sensor reading for curve indicator

	// Timing
	unsigned long lastActivityTime = 0;
	unsigned long stateStartTime = 0;
	unsigned long lastCurveUpdate = 0;

	// Menu items
	static const int mainMenuCount = 4;
	static const int sensorsSubMenuCount = 6; // 5 sensors + Reset
	static const int sensorDetailMenuCount = 4; // Calibration, Curve, Floor, Ceiling
	static const int midiSubMenuCount = 9;
	static const int deviceSettingsSubMenuCount = 3;
	
	// Scroll offsets for menus
	int mainMenuScroll = 0;
	int subMenuScroll = 0;
	int thirdMenuScroll = 0;
	static const int maxVisibleItems = 7; // Max items visible on screen at once
};
#endif
