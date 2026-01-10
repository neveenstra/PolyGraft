#include "DisplayHandler.h"

const unsigned long LOADING_DURATION = 2000;

// UI Color Scheme
const uint16_t COLOR_BACKGROUND = ILI9341_BLACK;
const uint16_t COLOR_HEADER_TEXT = ILI9341_DARKGREY;
const uint16_t COLOR_HEADER_LINE = ILI9341_DARKGREY;
const uint16_t COLOR_MENU_TEXT = ILI9341_LIGHTGREY;
const uint16_t COLOR_SELECTION_BG = ILI9341_NAVY;
const uint16_t COLOR_SELECTION_TEXT = ILI9341_CYAN;
const uint16_t COLOR_VALUE_NORMAL = ILI9341_WHITE;
const uint16_t COLOR_VALUE_POSITIVE = ILI9341_GREEN;
const uint16_t COLOR_VALUE_NEGATIVE = ILI9341_RED;
const uint16_t COLOR_ACCENT = ILI9341_ORANGE;

// Main menu items
const char* mainMenuItems[] = {
  "Sensors",
  "MIDI",
  "Device",
  "About"
};

// Sensors sub-menu (second level) - list of sensors
const char* sensorsSubMenu[] = {
  "Breath",
  "Pinch",
  "Expression",
  "Tilt",
  "Nod",
  "Reset"
};

// Third-level sub-menu items (under each sensor)
const char* sensorDetailMenu[] = {
  "Calibration",
  "Curve",
  "Floor",
  "Ceiling"
};

const char* midiSubMenu[] = {
  "MIDI Channel",
  "Breath CC",
  "Pinch CC",
  "Expression CC",
  "Tilt CC",
  "Nod CC",
  "USB MIDI",
  "Hardware MIDI",
  "Reset MIDI"
};



const char* deviceSettingsSubMenu[] = {
  "Display Brightness",
  "Sleep Timeout",
  "Factory Reset"
};

DisplayHandler::DisplayHandler(SensorCache& sensorCache, UserSettings& userSettings)
  : m_sensorCache(sensorCache)
  , m_userSettings(userSettings)
  , tft(TFT_CS, TFT_DC, TFT_RST)
{
}

void DisplayHandler::begin() {
  // Turn backlight OFF first
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, LOW);
  
  // Initialize display
  pinMode(TFT_RST, OUTPUT);
  tft.begin(24000000);
  tft.setRotation(1); // Landscape mode
  
  displayState = DisplayState::DISPLAY_ON;
  currentState = MenuState::LOADING;
  stateStartTime = millis();
  lastActivityTime = millis();
  
  // Show logo
  showLoadingScreen();
  digitalWrite(TFT_BL, HIGH);
}

void DisplayHandler::update() {
  unsigned long currentTime = millis();
  
  // Check for sleep timeout (disabled on sensor setting screens to allow monitoring)
  if (displayState == DisplayState::DISPLAY_ON) {
    // Skip timeout check on sensor detail screens
    if (!(menuDepth == 3 && mainMenuSelection == 0)) {
      unsigned long sleepTimeout = m_userSettings.getScreenSleep() * 1000; // Convert seconds to milliseconds
      if (currentTime - lastActivityTime > sleepTimeout) {
        sleep();
        return;
      }
    }
  }
  
  // Auto-advance from loading screen
  if (currentState == MenuState::LOADING) {
    if (currentTime - stateStartTime > LOADING_DURATION) {
      changeMenuState(MenuState::MAIN_MENU);
    }
  }
  
  // Update sensor indicator on curve (only in sensor detail menu)
  if (displayState == DisplayState::DISPLAY_ON && menuDepth == 3 && mainMenuSelection == 0) {
    // Update at 20Hz (50ms intervals)
    if (currentTime - lastCurveUpdate > 50) {
      lastCurveUpdate = currentTime;
      updateCurveSensorIndicator();
    }
  }
  
  // Sensor values screen draws once on entry, not continuously updated
}

void DisplayHandler::pressUp() {
  if (displayState == DisplayState::DISPLAY_OFF) {
    wake();
    return;
  }
  
  lastActivityTime = millis();
  
  if (inlineEditMode) {
    if (editingFloat) {
      // Floor and ceiling use 0.01 increment, calibration uses 0.1
      float increment = (menuDepth == 3 && (thirdMenuSelection == 2 || thirdMenuSelection == 3)) ? 0.01 : 0.1;
      editValueFloat += increment;
      // Cap ceiling at 1.0, other floats at 10.0
      float maxFloat = (menuDepth == 3 && thirdMenuSelection == 3) ? 1.0 : 10.0;
      if (editValueFloat > maxFloat) editValueFloat = maxFloat;
    } else {
      // Increment by 5 for sleep timeout, by 1 for others
      int increment = (mainMenuSelection == 2 && subMenuSelection == 1) ? 5 : 1;
      editValue += increment;
      // Apply context-specific max values
      int maxVal = 255;
      if (mainMenuSelection == 1) { // MIDI
        if (subMenuSelection == 0) maxVal = 16; // MIDI Channel 1-16
        else if (subMenuSelection >= 1 && subMenuSelection <= 5) maxVal = 127; // CC 0-127
        else maxVal = 1; // Boolean ON/OFF
      } else if (mainMenuSelection == 0 && menuDepth == 3 && thirdMenuSelection == 1) { // Sensor Curve
        maxVal = 4; // Curve settings: 1-4 (linear, concave, convex, s-curve)
      } else if (mainMenuSelection == 2) { // Device
        if (subMenuSelection == 0) maxVal = 10; // Brightness 1-10
        else if (subMenuSelection == 1) maxVal = 90; // Sleep timeout 0-90 seconds
        else maxVal = 1; // Boolean
      }
      if (editValue > maxVal) editValue = maxVal;
      int minVal = (mainMenuSelection == 2 && subMenuSelection == 0) ? 1 : 0; // Brightness min is 1
      if (editValue < minVal) editValue = minVal;
      
      // Apply brightness change immediately for preview
      if (mainMenuSelection == 2 && subMenuSelection == 0) {
        int scaledBrightness = 26 + ((editValue - 1) * 25);
        if (editValue == 10) scaledBrightness = 255;
        analogWrite(TFT_BL, scaledBrightness);
      }
    }
    redrawEditValue();
  } else if (currentState == MenuState::SUB_MENU) {
    int maxItems;
    if (menuDepth == 2) {
      maxItems = (mainMenuSelection == 0) ? sensorsSubMenuCount :
                 (mainMenuSelection == 1) ? midiSubMenuCount : deviceSettingsSubMenuCount;
      subMenuSelection--;
      if (subMenuSelection < 0) subMenuSelection = maxItems - 1;
    } else { // menuDepth == 3
      maxItems = sensorDetailMenuCount; // All sensors have 4 settings
      thirdMenuSelection--;
      if (thirdMenuSelection < 0) thirdMenuSelection = maxItems - 1;
    }
    drawSubMenu();
  } else if (currentState == MenuState::MAIN_MENU) {
    mainMenuSelection--;
    if (mainMenuSelection < 0) mainMenuSelection = mainMenuCount - 1;
    drawMainMenu();
  }
}

void DisplayHandler::pressDown() {
  if (displayState == DisplayState::DISPLAY_OFF) {
    wake();
    return;
  }
  
  lastActivityTime = millis();
  
  if (inlineEditMode) {
    if (editingFloat) {
      // Floor and ceiling use 0.01 decrement, calibration uses 0.1
      float decrement = (menuDepth == 3 && (thirdMenuSelection == 2 || thirdMenuSelection == 3)) ? 0.01 : 0.1;
      editValueFloat -= decrement;
      if (editValueFloat < 0.0) editValueFloat = 0.0;
    } else {
      // Decrement by 5 for sleep timeout, by 1 for others
      int decrement = (mainMenuSelection == 2 && subMenuSelection == 1) ? 5 : 1;
      editValue -= decrement;
      // Apply context-specific min values  
      int minVal = 0;
      if (mainMenuSelection == 1 && subMenuSelection == 0) minVal = 1; // MIDI Channel min 1
      else if (mainMenuSelection == 2 && subMenuSelection == 0) minVal = 1; // Brightness min 1
      else if (mainMenuSelection == 2 && subMenuSelection == 1) minVal = 10; // Sleep timeout min 10
      else if (mainMenuSelection == 0 && menuDepth == 3 && thirdMenuSelection == 1) { // Sensor Curve
        minVal = 1; // Curve setting min 1
      }
      if (editValue < minVal) editValue = minVal;
      
      // Apply brightness change immediately for preview
      if (mainMenuSelection == 2 && subMenuSelection == 0) {
        int scaledBrightness = 26 + ((editValue - 1) * 25);
        if (editValue == 10) scaledBrightness = 255;
        analogWrite(TFT_BL, scaledBrightness);
      }
    }
    redrawEditValue();
  } else if (currentState == MenuState::SUB_MENU) {
    int maxItems;
    if (menuDepth == 2) {
      maxItems = (mainMenuSelection == 0) ? sensorsSubMenuCount :
                 (mainMenuSelection == 1) ? midiSubMenuCount : deviceSettingsSubMenuCount;
      subMenuSelection++;
      if (subMenuSelection >= maxItems) subMenuSelection = 0;
    } else { // menuDepth == 3
      maxItems = sensorDetailMenuCount; // All sensors have 4 settings
      thirdMenuSelection++;
      if (thirdMenuSelection >= maxItems) thirdMenuSelection = 0;
    }
    drawSubMenu();
  } else if (currentState == MenuState::MAIN_MENU) {
    mainMenuSelection++;
    if (mainMenuSelection >= mainMenuCount) mainMenuSelection = 0;
    drawMainMenu();
  }
}

void DisplayHandler::pressLeft() {
  if (displayState == DisplayState::DISPLAY_OFF) {
    wake();
    return;
  }
  
  lastActivityTime = millis();
  
  if (inlineEditMode) {
    // Cancel inline edit - restore saved brightness if editing brightness
    if (mainMenuSelection == 2 && subMenuSelection == 0) {
      analogWrite(TFT_BL, m_userSettings.getDisplayBrightness());
    }
    inlineEditMode = false;
    needsFullRedraw = true;
    drawSubMenu();
  } else if (currentState == MenuState::SUB_MENU) {
    // Back to previous menu level
    if (menuDepth == 3) {
      menuDepth = 2;
      thirdMenuSelection = 0;
      thirdMenuScroll = 0; // Reset scroll when going back
      prevSensorValue = -1.0; // Reset sensor indicator
      needsFullRedraw = true;
      drawSubMenu();
    } else {
      currentState = MenuState::MAIN_MENU;
      subMenuSelection = 0;
      subMenuScroll = 0; // Reset scroll when going back
      prevSensorValue = -1.0; // Reset sensor indicator
      menuDepth = 1;
      needsFullRedraw = true;
      drawMainMenu();
    }
  } else if (currentState == MenuState::CONFIRM_DIALOG) {
    // Cancel - no reset
    pendingResetType = 0;
    currentState = MenuState::SUB_MENU;
    needsFullRedraw = true;
    drawSubMenu();
  } else if (currentState == MenuState::ABOUT) {
    // Back to main menu
    currentState = MenuState::MAIN_MENU;
    needsFullRedraw = true;
    drawMainMenu();
  }
}

void DisplayHandler::pressRight() {
  if (displayState == DisplayState::DISPLAY_OFF) {
    wake();
    return;
  }
  
  lastActivityTime = millis();
  
  if (inlineEditMode) {
    // Save value based on menu context
    if (mainMenuSelection == 0 && menuDepth == 3) { // Sensors -> [Sensor] -> [Setting]
      // subMenuSelection 0-4 = Breath, Pinch, Expression, Tilt, Nod
      // thirdMenuSelection 0-3 = Calibration, Curve, Floor, Ceiling
      if (thirdMenuSelection == 0) { // Calibration
        if (subMenuSelection == 0) m_userSettings.setCalBreath(editValueFloat);
        else if (subMenuSelection == 1) m_userSettings.setCalPinch(editValueFloat);
        else if (subMenuSelection == 2) m_userSettings.setCalExp(editValueFloat);
        else if (subMenuSelection == 3) m_userSettings.setCalTilt(editValueFloat);
        else if (subMenuSelection == 4) m_userSettings.setCalNod(editValueFloat);
      } else if (thirdMenuSelection == 1) { // Curve
        if (subMenuSelection == 0) m_userSettings.setBreathCurve(editValue);
        else if (subMenuSelection == 1) m_userSettings.setPinchCurve(editValue);
        else if (subMenuSelection == 2) m_userSettings.setExpCurve(editValue);
        else if (subMenuSelection == 3) m_userSettings.setTiltCurve(editValue);
        else if (subMenuSelection == 4) m_userSettings.setNodCurve(editValue);
      } else if (thirdMenuSelection == 2) { // Floor
        if (subMenuSelection == 0) m_userSettings.setBreathFloor(editValueFloat);
        else if (subMenuSelection == 1) m_userSettings.setPinchFloor(editValueFloat);
        else if (subMenuSelection == 2) m_userSettings.setExpFloor(editValueFloat);
        else if (subMenuSelection == 3) m_userSettings.setTiltFloor(editValueFloat);
        else if (subMenuSelection == 4) m_userSettings.setNodFloor(editValueFloat);
      } else if (thirdMenuSelection == 3) { // Ceiling
        if (subMenuSelection == 0) m_userSettings.setBreathCeiling(editValueFloat);
        else if (subMenuSelection == 1) m_userSettings.setPinchCeiling(editValueFloat);
        else if (subMenuSelection == 2) m_userSettings.setExpCeiling(editValueFloat);
        else if (subMenuSelection == 3) m_userSettings.setTiltCeiling(editValueFloat);
        else if (subMenuSelection == 4) m_userSettings.setNodCeiling(editValueFloat);
      }
    } else if (mainMenuSelection == 1) { // MIDI
      if (subMenuSelection == 0) m_userSettings.setMidiChannel(editValue);
      else if (subMenuSelection == 1) m_userSettings.setBreathCC(editValue);
      else if (subMenuSelection == 2) m_userSettings.setPinchCC(editValue);
      else if (subMenuSelection == 3) m_userSettings.setExpCC(editValue);
      else if (subMenuSelection == 4) m_userSettings.setTiltCC(editValue);
      else if (subMenuSelection == 5) m_userSettings.setNodCC(editValue);
      else if (subMenuSelection == 6) m_userSettings.setUsbMidiEnabled(editValue > 0);
      else if (subMenuSelection == 7) m_userSettings.setHwMidiEnabled(editValue > 0);
    } else if (mainMenuSelection == 2) { // Device Settings
      if (subMenuSelection == 0) {
        // Map 1-10 directly to brightness levels: 1=26, 2=51, 3=77, 4=102, 5=128, 6=153, 7=179, 8=204, 9=230, 10=255
        int scaledBrightness = 26 + ((editValue - 1) * 25);
        if (editValue == 10) scaledBrightness = 255; // Ensure max is exactly 255
        m_userSettings.setDisplayBrightness(scaledBrightness);
        analogWrite(TFT_BL, scaledBrightness); // Apply immediately
      }
      else if (subMenuSelection == 1) m_userSettings.setScreenSleep(editValue);
    }
    inlineEditMode = false;
    redrawEditValue();
  } else if (currentState == MenuState::SUB_MENU) {
    // Select sub-menu item
    if (menuDepth == 2) {
      if (mainMenuSelection == 0) {
        // Sensors submenu
        if (subMenuSelection == 5) {
          // Reset all sensors - show confirmation
          pendingResetType = 5;
          currentState = MenuState::CONFIRM_DIALOG;
          drawConfirmDialog();
        } else {
          // Go deeper into individual sensor settings
          menuDepth = 3;
          thirdMenuSelection = 0;
          thirdMenuScroll = 0; // Reset scroll when entering third level
          needsFullRedraw = true;
          drawSubMenu();
        }
      } else {
        // Enter edit mode for second-level items
        editingFloat = false;
        
        if (mainMenuSelection == 1) { // MIDI
          if (subMenuSelection == 8) {
            // Reset MIDI - show confirmation
            pendingResetType = 3;
            currentState = MenuState::CONFIRM_DIALOG;
            drawConfirmDialog();
          } else {
            editingFloat = false;
            if (subMenuSelection == 0) editValue = m_userSettings.getMidiChannel();
            else if (subMenuSelection == 1) editValue = m_userSettings.getBreathCC();
            else if (subMenuSelection == 2) editValue = m_userSettings.getPinchCC();
            else if (subMenuSelection == 3) editValue = m_userSettings.getExpCC();
            else if (subMenuSelection == 4) editValue = m_userSettings.getTiltCC();
            else if (subMenuSelection == 5) editValue = m_userSettings.getNodCC();
            else if (subMenuSelection == 6) editValue = m_userSettings.getUsbMidiEnabled() ? 1 : 0;
            else if (subMenuSelection == 7) editValue = m_userSettings.getHwMidiEnabled() ? 1 : 0;
            
            inlineEditMode = true;
            redrawEditValue();
          }
        } else if (mainMenuSelection == 2) { // Device Settings
          if (subMenuSelection == 2) {
            // Factory Reset - show confirmation
            pendingResetType = 4;
            currentState = MenuState::CONFIRM_DIALOG;
            drawConfirmDialog();
          } else {
            editingFloat = false;
            if (subMenuSelection == 0) {
              // Convert brightness back to 1-10 range
              int brightness = m_userSettings.getDisplayBrightness();
              if (brightness >= 255) editValue = 10;
              else if (brightness < 26) editValue = 1;
              else editValue = 1 + (brightness - 26) / 25;
              // Clamp to valid range
              if (editValue < 1) editValue = 1;
              if (editValue > 10) editValue = 10;
            }
            else if (subMenuSelection == 1) editValue = m_userSettings.getScreenSleep();
            
            inlineEditMode = true;
            redrawEditValue();
          }
        }
      }
    } else { // menuDepth == 3
      // Enter edit mode for third-level items in new sensor structure
      // subMenuSelection 0-4 = Breath, Pinch, Expression, Tilt, Nod
      // thirdMenuSelection 0-3 = Calibration, Curve, Floor, Ceiling
      
      editingFloat = (thirdMenuSelection == 0 || thirdMenuSelection == 2 || thirdMenuSelection == 3); // Calibration, Floor, Ceiling use floats
      
      // Load value based on sensor and setting
      if (thirdMenuSelection == 0) { // Calibration
        if (subMenuSelection == 0) editValueFloat = m_userSettings.getCalBreath();
        else if (subMenuSelection == 1) editValueFloat = m_userSettings.getCalPinch();
        else if (subMenuSelection == 2) editValueFloat = m_userSettings.getCalExp();
        else if (subMenuSelection == 3) editValueFloat = m_userSettings.getCalTilt();
        else if (subMenuSelection == 4) editValueFloat = m_userSettings.getCalNod();
      } else if (thirdMenuSelection == 1) { // Curve (int)
        if (subMenuSelection == 0) editValue = m_userSettings.getBreathCurve();
        else if (subMenuSelection == 1) editValue = m_userSettings.getPinchCurve();
        else if (subMenuSelection == 2) editValue = m_userSettings.getExpCurve();
        else if (subMenuSelection == 3) editValue = m_userSettings.getTiltCurve();
        else if (subMenuSelection == 4) editValue = m_userSettings.getNodCurve();
      } else if (thirdMenuSelection == 2) { // Floor (float)
        if (subMenuSelection == 0) editValueFloat = m_userSettings.getBreathFloor();
        else if (subMenuSelection == 1) editValueFloat = m_userSettings.getPinchFloor();
        else if (subMenuSelection == 2) editValueFloat = m_userSettings.getExpFloor();
        else if (subMenuSelection == 3) editValueFloat = m_userSettings.getTiltFloor();
        else if (subMenuSelection == 4) editValueFloat = m_userSettings.getNodFloor();
      } else if (thirdMenuSelection == 3) { // Ceiling (float)
        if (subMenuSelection == 0) editValueFloat = m_userSettings.getBreathCeiling();
        else if (subMenuSelection == 1) editValueFloat = m_userSettings.getPinchCeiling();
        else if (subMenuSelection == 2) editValueFloat = m_userSettings.getExpCeiling();
        else if (subMenuSelection == 3) editValueFloat = m_userSettings.getTiltCeiling();
        else if (subMenuSelection == 4) editValueFloat = m_userSettings.getNodCeiling();
      }
      
      inlineEditMode = true;
      redrawEditValue();
    }
  } else if (currentState == MenuState::MAIN_MENU) {
    // Enter sub-menu or show about screen
    if (mainMenuSelection == 3) {
      // About menu - show info screen
      currentState = MenuState::ABOUT;
      drawAbout();
    } else {
      currentState = MenuState::SUB_MENU;
      subMenuSelection = 0;
      subMenuScroll = 0; // Reset scroll when entering submenu
      menuDepth = 2;
      needsFullRedraw = true;
      drawSubMenu();
    }
  } else if (currentState == MenuState::CONFIRM_DIALOG) {
    // Yes - perform reset based on type
    if (pendingResetType == 1) {
      // Reset calibration values to 1.0
      m_userSettings.setCalBreath(1.0);
      m_userSettings.setCalPinch(1.0);
      m_userSettings.setCalExp(1.0);
      m_userSettings.setCalTilt(1.0);
      m_userSettings.setCalNod(1.0);
    } else if (pendingResetType == 2) {
      // Reset curves and floor/ceiling to defaults
      m_userSettings.setBreathCurve(1);
      m_userSettings.setBreathFloor(0.0);
      m_userSettings.setBreathCeiling(1.0);
      m_userSettings.setPinchCurve(1);
      m_userSettings.setPinchFloor(0.0);
      m_userSettings.setPinchCeiling(1.0);
      m_userSettings.setExpCurve(1);
      m_userSettings.setExpFloor(0.0);
      m_userSettings.setExpCeiling(1.0);
      m_userSettings.setTiltCurve(1);
      m_userSettings.setTiltFloor(0.0);
      m_userSettings.setTiltCeiling(1.0);
      m_userSettings.setNodCurve(1);
      m_userSettings.setNodFloor(0.0);
      m_userSettings.setNodCeiling(1.0);
    } else if (pendingResetType == 3) {
      // Reset MIDI settings
      m_userSettings.setMidiChannel(1);
      m_userSettings.setBreathCC(1);
      m_userSettings.setPinchCC(2);
      m_userSettings.setExpCC(3);
      m_userSettings.setTiltCC(4);
      m_userSettings.setNodCC(5);
      m_userSettings.setUsbMidiEnabled(true);
      m_userSettings.setHwMidiEnabled(true);
    } else if (pendingResetType == 4) {
      // Factory reset - all settings
      m_userSettings.resetToDefaults();
    } else if (pendingResetType == 5) {
      // Reset all sensor settings (1=linear)
      m_userSettings.setCalBreath(1.0);
      m_userSettings.setCalPinch(1.0);
      m_userSettings.setCalExp(1.0);
      m_userSettings.setCalTilt(1.0);
      m_userSettings.setCalNod(1.0);
      m_userSettings.setBreathCurve(1);
      m_userSettings.setPinchCurve(1);
      m_userSettings.setExpCurve(1);
      m_userSettings.setTiltCurve(1);
      m_userSettings.setNodCurve(1);
      m_userSettings.setBreathFloor(0.0);
      m_userSettings.setBreathCeiling(1.0);
      m_userSettings.setPinchFloor(0.0);
      m_userSettings.setPinchCeiling(1.0);
      m_userSettings.setExpFloor(0.0);
      m_userSettings.setExpCeiling(1.0);
      m_userSettings.setTiltFloor(0.0);
      m_userSettings.setTiltCeiling(1.0);
      m_userSettings.setNodFloor(0.0);
      m_userSettings.setNodCeiling(1.0);
    }
    pendingResetType = 0;
    currentState = MenuState::SUB_MENU;
    needsFullRedraw = true;
    drawSubMenu();
  }
}

void DisplayHandler::changeMenuState(MenuState newState) {
  currentState = newState;
  stateStartTime = millis();
  needsFullRedraw = true;
  
  switch(newState) {
    case MenuState::LOADING:
      showLoadingScreen();
      break;
    case MenuState::MAIN_MENU:
      mainMenuSelection = 0;
      menuDepth = 1;
      drawMainMenu();
      break;
    case MenuState::SUB_MENU:
      drawSubMenu();
      break;
    case MenuState::EDIT_MODE:
      drawEditMode();
      break;
    case MenuState::CONFIRM_DIALOG:
      drawConfirmDialog();
      break;
    case MenuState::ABOUT:
      drawAbout();
      break;
  }
}

void DisplayHandler::showLoadingScreen() {
  tft.fillScreen(COLOR_BACKGROUND);
  
  // Calculate centered position for logo
  int16_t x = (320 - LOGO_WIDTH) / 2;
  int16_t y = (240 - LOGO_HEIGHT) / 2;
  
  // Draw the logo
  tft.drawRGBBitmap(x, y, logo, LOGO_WIDTH, LOGO_HEIGHT);
}

void DisplayHandler::drawMainMenu() {
  if (needsFullRedraw) {
    tft.fillScreen(COLOR_BACKGROUND);
    tft.setTextColor(COLOR_HEADER_TEXT);
    tft.setTextSize(2);
    tft.setCursor(10, 10);
    tft.println("Main Menu");
    
    tft.drawLine(0, 30, 320, 30, COLOR_HEADER_LINE);
    
    // Draw all items on full redraw
    for (int i = 0; i < mainMenuCount; i++) {
      int y = 50 + (i * 30);
      if (i == mainMenuSelection) {
        tft.fillRect(5, y - 2, 310, 24, COLOR_SELECTION_BG);
        tft.setTextColor(COLOR_SELECTION_TEXT);
      } else {
        tft.setTextColor(COLOR_MENU_TEXT);
      }
      tft.setCursor(15, y);
      tft.println(mainMenuItems[i]);
    }
    needsFullRedraw = false;
  } else {
    // Only update selection changes
    if (prevMainMenuSelection != mainMenuSelection) {
      // Redraw previous selection (deselect)
      if (prevMainMenuSelection >= 0) {
        int y = 50 + (prevMainMenuSelection * 30);
        tft.fillRect(5, y - 2, 310, 24, COLOR_BACKGROUND);
        tft.setTextColor(COLOR_MENU_TEXT);
        tft.setCursor(15, y);
        tft.println(mainMenuItems[prevMainMenuSelection]);
      }
      
      // Redraw new selection (select)
      int y = 50 + (mainMenuSelection * 30);
      tft.fillRect(5, y - 2, 310, 24, COLOR_SELECTION_BG);
      tft.setTextColor(COLOR_SELECTION_TEXT);
      tft.setCursor(15, y);
      tft.println(mainMenuItems[mainMenuSelection]);
    }
  }
  prevMainMenuSelection = mainMenuSelection;
}

void DisplayHandler::drawSubMenu() {
  const char** items = nullptr;
  int itemCount = 0;
  int currentSelection;
  
  if (menuDepth == 2) {
    currentSelection = subMenuSelection;
    switch (mainMenuSelection) {
      case 0: // Sensors
        items = sensorsSubMenu;
        itemCount = sensorsSubMenuCount;
        break;
      case 1: // MIDI Settings
        items = midiSubMenu;
        itemCount = midiSubMenuCount;
        break;
      case 2: // Device Settings
        items = deviceSettingsSubMenu;
        itemCount = deviceSettingsSubMenuCount;
        break;
      default:
        return; // Invalid state
    }
  } else { // menuDepth == 3
    currentSelection = thirdMenuSelection;
    // New sensor structure: all sensors use the same detail menu
    items = sensorDetailMenu;
    itemCount = sensorDetailMenuCount;
  }
  
  if (items == nullptr || itemCount == 0) {
    return; // Safety check
  }
  
  // Calculate scroll offset to keep selection visible
  int* scrollOffset = (menuDepth == 2) ? &subMenuScroll : &thirdMenuScroll;
  
  // Adjust scroll to keep current selection visible
  if (currentSelection < *scrollOffset) {
    *scrollOffset = currentSelection;
  } else if (currentSelection >= *scrollOffset + maxVisibleItems) {
    *scrollOffset = currentSelection - maxVisibleItems + 1;
  }
  
  // Clamp scroll offset
  int maxScroll = (itemCount > maxVisibleItems) ? (itemCount - maxVisibleItems) : 0;
  if (*scrollOffset > maxScroll) *scrollOffset = maxScroll;
  if (*scrollOffset < 0) *scrollOffset = 0;
  
  int visibleCount = (itemCount < maxVisibleItems) ? itemCount : maxVisibleItems;
  
  if (needsFullRedraw) {
    tft.fillScreen(COLOR_BACKGROUND);
    tft.setTextColor(COLOR_HEADER_TEXT);
    tft.setTextSize(2);
    tft.setCursor(10, 10);
    
    // Show breadcrumb
    if (menuDepth == 3) {
      tft.print(mainMenuItems[mainMenuSelection]);
      tft.print(" > ");
      tft.println(sensorsSubMenu[subMenuSelection]);
    } else {
      tft.println(mainMenuItems[mainMenuSelection]);
    }
    
    tft.drawLine(0, 30, 320, 30, COLOR_HEADER_LINE);
    
    // Draw visible items only
    for (int i = 0; i < visibleCount; i++) {
      int itemIndex = *scrollOffset + i;
      int y = 50 + (i * 25);
      if (itemIndex == currentSelection) {
        tft.fillRect(5, y - 2, 315, 22, COLOR_SELECTION_BG);
        tft.setTextColor(COLOR_SELECTION_TEXT);
      } else {
        tft.setTextColor(COLOR_MENU_TEXT);
      }
      tft.setCursor(15, y);
      tft.print(items[itemIndex]);
      
      // Show current values (or edit values if in inline edit mode)
      bool isEditing = inlineEditMode && itemIndex == currentSelection;
      
      // Right-aligned value position (right margin = 10 pixels)
      const int rightMargin = 10;
      
      if (menuDepth == 2) {
        if (mainMenuSelection == 1) { // MIDI Settings
          if (itemIndex == 0) { 
            int val = isEditing ? editValue : m_userSettings.getMidiChannel();
            if (isEditing) tft.setTextColor(COLOR_ACCENT);
            tft.setCursor(320 - rightMargin - (String(val).length() * 12), y);
            tft.print(val);
          }
          else if (itemIndex == 1) { 
            int val = isEditing ? editValue : m_userSettings.getBreathCC();
            if (isEditing) tft.setTextColor(COLOR_ACCENT);
            tft.setCursor(320 - rightMargin - (String(val).length() * 12), y);
            tft.print(val);
          }
          else if (itemIndex == 2) { 
            int val = isEditing ? editValue : m_userSettings.getPinchCC();
            if (isEditing) tft.setTextColor(COLOR_ACCENT);
            tft.setCursor(320 - rightMargin - (String(val).length() * 12), y);
            tft.print(val);
          }
          else if (itemIndex == 3) { 
            int val = isEditing ? editValue : m_userSettings.getExpCC();
            if (isEditing) tft.setTextColor(COLOR_ACCENT);
            tft.setCursor(320 - rightMargin - (String(val).length() * 12), y);
            tft.print(val);
          }
          else if (itemIndex == 4) { 
            int val = isEditing ? editValue : m_userSettings.getTiltCC();
            if (isEditing) tft.setTextColor(COLOR_ACCENT);
            tft.setCursor(320 - rightMargin - (String(val).length() * 12), y);
            tft.print(val);
          }
          else if (itemIndex == 5) { 
            int val = isEditing ? editValue : m_userSettings.getNodCC();
            if (isEditing) tft.setTextColor(COLOR_ACCENT);
            tft.setCursor(320 - rightMargin - (String(val).length() * 12), y);
            tft.print(val);
          }
          else if (itemIndex == 6) { 
            const char* val = (isEditing ? editValue : m_userSettings.getUsbMidiEnabled()) ? "ON" : "OFF";
            if (isEditing) tft.setTextColor(COLOR_ACCENT);
            tft.setCursor(320 - rightMargin - (strlen(val) * 12), y);
            tft.print(val);
          }
          else if (itemIndex == 7) { 
            const char* val = (isEditing ? editValue : m_userSettings.getHwMidiEnabled()) ? "ON" : "OFF";
            if (isEditing) tft.setTextColor(COLOR_ACCENT);
            tft.setCursor(320 - rightMargin - (strlen(val) * 12), y);
            tft.print(val);
          }
          // itemIndex 8 is "Reset MIDI" - no value to display
        } else if (mainMenuSelection == 2) { // Device Settings
          if (itemIndex == 0) { 
            int brightness = m_userSettings.getDisplayBrightness();
            int displayVal;
            if (brightness >= 255) displayVal = 10;
            else if (brightness < 26) displayVal = 1;
            else displayVal = 1 + (brightness - 26) / 25;
            int val = isEditing ? editValue : displayVal;
            if (isEditing) tft.setTextColor(COLOR_ACCENT);
            tft.setCursor(320 - rightMargin - (String(val).length() * 12), y);
            tft.print(val);
          }
          else if (itemIndex == 1) { 
            int val = isEditing ? editValue : m_userSettings.getScreenSleep();
            String valStr = String(val) + "s";
            if (isEditing) tft.setTextColor(COLOR_ACCENT);
            tft.setCursor(320 - rightMargin - (valStr.length() * 12), y);
            tft.print(valStr);
          }
          // itemIndex 2 is "Factory Reset" - no value to display
        }
      } else { // menuDepth == 3
        // New sensor-based structure
        // subMenuSelection 0-4 = Breath, Pinch, Expression, Tilt, Nod
        // thirdMenuSelection/itemIndex 0-3 = Calibration, Curve, Floor, Ceiling
        
        if (itemIndex == 0) { // Calibration
          float val = 0.0;
          if (subMenuSelection == 0) val = m_userSettings.getCalBreath();
          else if (subMenuSelection == 1) val = m_userSettings.getCalPinch();
          else if (subMenuSelection == 2) val = m_userSettings.getCalExp();
          else if (subMenuSelection == 3) val = m_userSettings.getCalTilt();
          else if (subMenuSelection == 4) val = m_userSettings.getCalNod();
          if (isEditing) {
            val = editValueFloat;
            tft.setTextColor(COLOR_ACCENT);
          }
          tft.setCursor(320 - rightMargin - (String(val, 2).length() * 12), y);
          tft.print(val, 2);
        }
        else if (itemIndex == 1) { // Curve
          int val = 0;
          if (subMenuSelection == 0) val = m_userSettings.getBreathCurve();
          else if (subMenuSelection == 1) val = m_userSettings.getPinchCurve();
          else if (subMenuSelection == 2) val = m_userSettings.getExpCurve();
          else if (subMenuSelection == 3) val = m_userSettings.getTiltCurve();
          else if (subMenuSelection == 4) val = m_userSettings.getNodCurve();
          if (isEditing) {
            val = editValue;
            tft.setTextColor(COLOR_ACCENT);
          }
          tft.setCursor(320 - rightMargin - (String(val).length() * 12), y);
          tft.print(val);
        }
        else if (itemIndex == 2) { // Floor
          float val = 0.0;
          if (subMenuSelection == 0) val = m_userSettings.getBreathFloor();
          else if (subMenuSelection == 1) val = m_userSettings.getPinchFloor();
          else if (subMenuSelection == 2) val = m_userSettings.getExpFloor();
          else if (subMenuSelection == 3) val = m_userSettings.getTiltFloor();
          else if (subMenuSelection == 4) val = m_userSettings.getNodFloor();
          if (isEditing) {
            val = editValueFloat;
            tft.setTextColor(COLOR_ACCENT);
          }
          tft.setCursor(320 - rightMargin - (String(val, 2).length() * 12), y);
          tft.print(val, 2);
        }
        else if (itemIndex == 3) { // Ceiling
          float val = 0.0;
          if (subMenuSelection == 0) val = m_userSettings.getBreathCeiling();
          else if (subMenuSelection == 1) val = m_userSettings.getPinchCeiling();
          else if (subMenuSelection == 2) val = m_userSettings.getExpCeiling();
          else if (subMenuSelection == 3) val = m_userSettings.getTiltCeiling();
          else if (subMenuSelection == 4) val = m_userSettings.getNodCeiling();
          if (isEditing) {
            val = editValueFloat;
            tft.setTextColor(COLOR_ACCENT);
          }
          tft.setCursor(320 - rightMargin - (String(val, 2).length() * 12), y);
          tft.print(val, 2);
        }
      }
    }
    needsFullRedraw = false;
    
    // Draw curve visualization for sensor detail menu
    if (menuDepth == 3 && mainMenuSelection == 0) {
      drawResponseCurve();
    }
  } else {
    // Check if scroll changed - if so, do full redraw
    static int prevScrollOffset = -1;
    int currentScrollOffset = (menuDepth == 2) ? subMenuScroll : thirdMenuScroll;
    
    // Initialize prevScrollOffset on first draw after full redraw
    if (prevScrollOffset == -1) {
      prevScrollOffset = currentScrollOffset;
    }
    
    if (prevScrollOffset != currentScrollOffset) {
      prevScrollOffset = currentScrollOffset;
      needsFullRedraw = true;
      drawSubMenu(); // Recursive call with needsFullRedraw = true
      return;
    }
    
    // Only update selection changes (no scroll change)
    int prevSelection = (menuDepth == 2) ? prevSubMenuSelection : prevThirdMenuSelection;
    if (prevSelection != currentSelection && prevSelection >= 0) {
      // Calculate screen positions based on scroll offset
      int prevScreenPos = prevSelection - currentScrollOffset;
      int currentScreenPos = currentSelection - currentScrollOffset;
      
      // Only redraw if both are visible on screen
      if (prevScreenPos >= 0 && prevScreenPos < maxVisibleItems) {
        // Redraw previous selection
        int y = 50 + (prevScreenPos * 25);
        tft.fillRect(5, y - 2, 315, 22, COLOR_BACKGROUND);
        tft.setTextColor(COLOR_MENU_TEXT);
        tft.setCursor(15, y);
        tft.print(items[prevSelection]);
      
        // Show value for previous item
        const int rightMargin = 10;
        if (menuDepth == 2) {
          if (mainMenuSelection == 1) {
            if (prevSelection == 0) { int val = m_userSettings.getMidiChannel(); tft.setCursor(320 - rightMargin - (String(val).length() * 12), y); tft.print(val); }
            else if (prevSelection == 1) { int val = m_userSettings.getBreathCC(); tft.setCursor(320 - rightMargin - (String(val).length() * 12), y); tft.print(val); }
            else if (prevSelection == 2) { int val = m_userSettings.getPinchCC(); tft.setCursor(320 - rightMargin - (String(val).length() * 12), y); tft.print(val); }
            else if (prevSelection == 3) { int val = m_userSettings.getExpCC(); tft.setCursor(320 - rightMargin - (String(val).length() * 12), y); tft.print(val); }
            else if (prevSelection == 4) { int val = m_userSettings.getTiltCC(); tft.setCursor(320 - rightMargin - (String(val).length() * 12), y); tft.print(val); }
            else if (prevSelection == 5) { int val = m_userSettings.getNodCC(); tft.setCursor(320 - rightMargin - (String(val).length() * 12), y); tft.print(val); }
            else if (prevSelection == 6) { const char* val = m_userSettings.getUsbMidiEnabled() ? "ON" : "OFF"; tft.setCursor(320 - rightMargin - (strlen(val) * 12), y); tft.print(val); }
            else if (prevSelection == 7) { const char* val = m_userSettings.getHwMidiEnabled() ? "ON" : "OFF"; tft.setCursor(320 - rightMargin - (strlen(val) * 12), y); tft.print(val); }
          } else if (mainMenuSelection == 2) {
            if (prevSelection == 0) { 
              int brightness = m_userSettings.getDisplayBrightness();
              int val;
              if (brightness >= 255) val = 10;
              else if (brightness < 26) val = 1;
              else val = 1 + (brightness - 26) / 25;
              tft.setCursor(320 - rightMargin - (String(val).length() * 12), y); 
              tft.print(val); 
            }
            else if (prevSelection == 1) { int val = m_userSettings.getScreenSleep(); String valStr = String(val) + "s"; tft.setCursor(320 - rightMargin - (valStr.length() * 12), y); tft.print(valStr); }
            // prevSelection == 2 is Factory Reset - no value to display
          }
        } else {
          // New sensor structure: prevSelection 0-3 = Calibration, Curve, Floor, Ceiling
          if (prevSelection == 0) { // Calibration
            float val = 0.0;
            if (subMenuSelection == 0) val = m_userSettings.getCalBreath();
            else if (subMenuSelection == 1) val = m_userSettings.getCalPinch();
            else if (subMenuSelection == 2) val = m_userSettings.getCalExp();
            else if (subMenuSelection == 3) val = m_userSettings.getCalTilt();
            else if (subMenuSelection == 4) val = m_userSettings.getCalNod();
            tft.setCursor(320 - rightMargin - (String(val, 2).length() * 12), y);
            tft.print(val, 2);
          }
          else if (prevSelection == 1) { // Curve
            int val = 0;
            if (subMenuSelection == 0) val = m_userSettings.getBreathCurve();
            else if (subMenuSelection == 1) val = m_userSettings.getPinchCurve();
            else if (subMenuSelection == 2) val = m_userSettings.getExpCurve();
            else if (subMenuSelection == 3) val = m_userSettings.getTiltCurve();
            else if (subMenuSelection == 4) val = m_userSettings.getNodCurve();
            tft.setCursor(320 - rightMargin - (String(val).length() * 12), y);
            tft.print(val);
          }
          else if (prevSelection == 2) { // Floor
            float val = 0.0;
            if (subMenuSelection == 0) val = m_userSettings.getBreathFloor();
            else if (subMenuSelection == 1) val = m_userSettings.getPinchFloor();
            else if (subMenuSelection == 2) val = m_userSettings.getExpFloor();
            else if (subMenuSelection == 3) val = m_userSettings.getTiltFloor();
            else if (subMenuSelection == 4) val = m_userSettings.getNodFloor();
            tft.setCursor(320 - rightMargin - (String(val, 2).length() * 12), y);
            tft.print(val, 2);
          }
          else if (prevSelection == 3) { // Ceiling
            float val = 0.0;
            if (subMenuSelection == 0) val = m_userSettings.getBreathCeiling();
            else if (subMenuSelection == 1) val = m_userSettings.getPinchCeiling();
            else if (subMenuSelection == 2) val = m_userSettings.getExpCeiling();
            else if (subMenuSelection == 3) val = m_userSettings.getTiltCeiling();
            else if (subMenuSelection == 4) val = m_userSettings.getNodCeiling();
            tft.setCursor(320 - rightMargin - (String(val, 2).length() * 12), y);
            tft.print(val, 2);
          }
        }
      }
      
      // Redraw new selection if visible
      if (currentScreenPos >= 0 && currentScreenPos < maxVisibleItems) {
        int y = 50 + (currentScreenPos * 25);
        tft.fillRect(5, y - 2, 315, 22, COLOR_SELECTION_BG);
        tft.setTextColor(COLOR_SELECTION_TEXT);
        tft.setCursor(15, y);
        tft.print(items[currentSelection]);
      
        // Show value for new item
        const int rightMargin = 10;
        if (menuDepth == 2) {
          if (mainMenuSelection == 1) {
            if (currentSelection == 0) { int val = m_userSettings.getMidiChannel(); tft.setCursor(320 - rightMargin - (String(val).length() * 12), y); tft.print(val); }
            else if (currentSelection == 1) { int val = m_userSettings.getBreathCC(); tft.setCursor(320 - rightMargin - (String(val).length() * 12), y); tft.print(val); }
            else if (currentSelection == 2) { int val = m_userSettings.getPinchCC(); tft.setCursor(320 - rightMargin - (String(val).length() * 12), y); tft.print(val); }
            else if (currentSelection == 3) { int val = m_userSettings.getExpCC(); tft.setCursor(320 - rightMargin - (String(val).length() * 12), y); tft.print(val); }
            else if (currentSelection == 4) { int val = m_userSettings.getTiltCC(); tft.setCursor(320 - rightMargin - (String(val).length() * 12), y); tft.print(val); }
            else if (currentSelection == 5) { int val = m_userSettings.getNodCC(); tft.setCursor(320 - rightMargin - (String(val).length() * 12), y); tft.print(val); }
            else if (currentSelection == 6) { const char* val = m_userSettings.getUsbMidiEnabled() ? "ON" : "OFF"; tft.setCursor(320 - rightMargin - (strlen(val) * 12), y); tft.print(val); }
            else if (currentSelection == 7) { const char* val = m_userSettings.getHwMidiEnabled() ? "ON" : "OFF"; tft.setCursor(320 - rightMargin - (strlen(val) * 12), y); tft.print(val); }
          } else if (mainMenuSelection == 2) {
            if (currentSelection == 0) { 
              int brightness = m_userSettings.getDisplayBrightness();
              int val;
              if (brightness >= 255) val = 10;
              else if (brightness < 26) val = 1;
              else val = 1 + (brightness - 26) / 25;
              tft.setCursor(320 - rightMargin - (String(val).length() * 12), y); 
              tft.print(val); 
            }
            else if (currentSelection == 1) { int val = m_userSettings.getScreenSleep(); String valStr = String(val) + "s"; tft.setCursor(320 - rightMargin - (valStr.length() * 12), y); tft.print(valStr); }
            // currentSelection == 2 is Factory Reset - no value to display
          }
        } else {
          // New sensor structure: currentSelection 0-3 = Calibration, Curve, Floor, Ceiling
          if (currentSelection == 0) { // Calibration
            float val = 0.0;
            if (subMenuSelection == 0) val = m_userSettings.getCalBreath();
            else if (subMenuSelection == 1) val = m_userSettings.getCalPinch();
            else if (subMenuSelection == 2) val = m_userSettings.getCalExp();
            else if (subMenuSelection == 3) val = m_userSettings.getCalTilt();
            else if (subMenuSelection == 4) val = m_userSettings.getCalNod();
            tft.setCursor(320 - rightMargin - (String(val, 2).length() * 12), y);
            tft.print(val, 2);
          }
          else if (currentSelection == 1) { // Curve
            int val = 0;
            if (subMenuSelection == 0) val = m_userSettings.getBreathCurve();
            else if (subMenuSelection == 1) val = m_userSettings.getPinchCurve();
            else if (subMenuSelection == 2) val = m_userSettings.getExpCurve();
            else if (subMenuSelection == 3) val = m_userSettings.getTiltCurve();
            else if (subMenuSelection == 4) val = m_userSettings.getNodCurve();
            tft.setCursor(320 - rightMargin - (String(val).length() * 12), y);
            tft.print(val);
          }
          else if (currentSelection == 2) { // Floor
            float val = 0.0;
            if (subMenuSelection == 0) val = m_userSettings.getBreathFloor();
            else if (subMenuSelection == 1) val = m_userSettings.getPinchFloor();
            else if (subMenuSelection == 2) val = m_userSettings.getExpFloor();
            else if (subMenuSelection == 3) val = m_userSettings.getTiltFloor();
            else if (subMenuSelection == 4) val = m_userSettings.getNodFloor();
            tft.setCursor(320 - rightMargin - (String(val, 2).length() * 12), y);
            tft.print(val, 2);
          }
          else if (currentSelection == 3) { // Ceiling
            float val = 0.0;
            if (subMenuSelection == 0) val = m_userSettings.getBreathCeiling();
            else if (subMenuSelection == 1) val = m_userSettings.getPinchCeiling();
            else if (subMenuSelection == 2) val = m_userSettings.getExpCeiling();
            else if (subMenuSelection == 3) val = m_userSettings.getTiltCeiling();
            else if (subMenuSelection == 4) val = m_userSettings.getNodCeiling();
            tft.setCursor(320 - rightMargin - (String(val, 2).length() * 12), y);
            tft.print(val, 2);
          }
        }
      }
    }
  }
  
  if (menuDepth == 2) {
    prevSubMenuSelection = subMenuSelection;
  } else {
    prevThirdMenuSelection = thirdMenuSelection;
  }
}

void DisplayHandler::drawResponseCurve() {
  // Only draw curve on sensor detail menu (depth 3)
  if (menuDepth != 3 || mainMenuSelection != 0) return;
  
  // Get current settings for selected sensor
  int curve = 1;
  float floor = 0.0;
  float ceiling = 1.0;
  
  if (subMenuSelection == 0) { // Breath
    curve = m_userSettings.getBreathCurve();
    floor = m_userSettings.getBreathFloor();
    ceiling = m_userSettings.getBreathCeiling();
  } else if (subMenuSelection == 1) { // Pinch
    curve = m_userSettings.getPinchCurve();
    floor = m_userSettings.getPinchFloor();
    ceiling = m_userSettings.getPinchCeiling();
  } else if (subMenuSelection == 2) { // Expression
    curve = m_userSettings.getExpCurve();
    floor = m_userSettings.getExpFloor();
    ceiling = m_userSettings.getExpCeiling();
  } else if (subMenuSelection == 3) { // Tilt
    curve = m_userSettings.getTiltCurve();
    floor = m_userSettings.getTiltFloor();
    ceiling = m_userSettings.getTiltCeiling();
  } else if (subMenuSelection == 4) { // Nod
    curve = m_userSettings.getNodCurve();
    floor = m_userSettings.getNodFloor();
    ceiling = m_userSettings.getNodCeiling();
  }
  
  // Override with edit values if currently editing
  if (inlineEditMode) {
    if (thirdMenuSelection == 0) { // Calibration (doesn't affect curve shape)
      // No override needed
    } else if (thirdMenuSelection == 1) { // Curve
      curve = editValue;
    } else if (thirdMenuSelection == 2) { // Floor
      floor = editValueFloat;
    } else if (thirdMenuSelection == 3) { // Ceiling
      ceiling = editValueFloat;
    }
  }
  
  // Curve drawing parameters
  const int graphX = 20;
  const int graphY = 150;
  const int graphWidth = 280;
  const int graphHeight = 80;
  const int graphRight = graphX + graphWidth;
  const int graphBottom = graphY + graphHeight;
  
  // Draw axes (complete bounding box)
  tft.drawLine(graphX, graphY, graphRight, graphY, COLOR_HEADER_LINE); // Top X axis
  tft.drawLine(graphX, graphBottom, graphRight, graphBottom, COLOR_HEADER_LINE); // Bottom X axis
  tft.drawLine(graphX, graphY, graphX, graphBottom, COLOR_HEADER_LINE); // Y axis
  
  // Draw curve - 4 types: 1=linear, 2=concave, 3=convex, 4=s-curve
  int prevY = -1;
  
  for (int x = 0; x <= graphWidth; x++) {
    // Normalize screen x to 0-1 (full input range)
    float inputNorm = (float)x / graphWidth;
    
    float outputNorm;
    if (inputNorm < floor) {
      // Below floor: output is 0
      outputNorm = 0.0;
    } else if (inputNorm > ceiling) {
      // Above ceiling: output is 1
      outputNorm = 1.0;
    } else {
      // Within floor-ceiling range: map to 0-1 and apply curve
      float mappedInput = (inputNorm - floor) / (ceiling - floor);
      
      if (curve == 1) {
        // Linear
        outputNorm = mappedInput;
      } else if (curve == 2) {
        // Concave (ease-in)
        outputNorm = pow(mappedInput, 2.0);
      } else if (curve == 3) {
        // Convex (ease-out)
        outputNorm = pow(mappedInput, 0.5);
      } else {
        // S-curve (ease-in-out)
        outputNorm = mappedInput * mappedInput * (3.0 - 2.0 * mappedInput);
      }
    }
    
    // Convert to screen Y coordinate
    int y = graphBottom - (int)(outputNorm * graphHeight);
    
    // Draw line from previous point
    if (prevY >= 0) {
      tft.drawLine(graphX + x - 1, prevY, graphX + x, y, COLOR_ACCENT);
    }
    prevY = y;
  }
}

void DisplayHandler::updateCurveSensorIndicator() {
  // Get current sensor reading and apply calibration (same math as main.cpp)
  float sensorValue = 0.0;
  if (subMenuSelection == 0) {
    sensorValue = m_sensorCache.getBreathNormalized() * m_userSettings.getCalBreath();
  } else if (subMenuSelection == 1) {
    sensorValue = m_sensorCache.getPinchNormalized() * m_userSettings.getCalPinch();
  } else if (subMenuSelection == 2) {
    sensorValue = m_sensorCache.getExpressionNormalized() * m_userSettings.getCalExp();
  } else if (subMenuSelection == 3) {
    sensorValue = m_sensorCache.getGyroX() * m_userSettings.getCalTilt();
  } else if (subMenuSelection == 4) {
    sensorValue = m_sensorCache.getGyroY() * m_userSettings.getCalNod();
  }
  
  // Apply calibration override if currently editing calibration
  if (inlineEditMode && thirdMenuSelection == 0) {
    // Use the edit value for calibration instead
    float rawValue = 0.0;
    if (subMenuSelection == 0) rawValue = m_sensorCache.getBreathNormalized();
    else if (subMenuSelection == 1) rawValue = m_sensorCache.getPinchNormalized();
    else if (subMenuSelection == 2) rawValue = m_sensorCache.getExpressionNormalized();
    else if (subMenuSelection == 3) rawValue = m_sensorCache.getGyroX();
    else if (subMenuSelection == 4) rawValue = m_sensorCache.getGyroY();
    sensorValue = rawValue * editValueFloat;
  }
  
  // Constrain to 0-1 range
  sensorValue = constrain(sensorValue, 0.0, 1.0);
  
  // Graph dimensions (must match drawResponseCurve)
  const int graphX = 20;
  const int graphY = 150;
  const int graphWidth = 280;
  const int graphHeight = 80;
  const int graphBottom = graphY + graphHeight;
  
  // Calculate X positions
  int prevX = graphX + (int)(prevSensorValue * graphWidth);
  int currentX = graphX + (int)(sensorValue * graphWidth);
  
  // Only redraw if position changed
  if (abs(currentX - prevX) >= 1 || prevSensorValue < 0) {
    // Clear previous indicator line by redrawing what was underneath
    if (prevSensorValue >= 0.0 && prevSensorValue <= 1.0) {
      // Get settings first (needed for both cleanup and marker overlap check)
      int curveVal = 1;
      float floorVal = 0.0;
      float ceilingVal = 1.0;
      
      if (subMenuSelection == 0) {
        curveVal = m_userSettings.getBreathCurve();
        floorVal = m_userSettings.getBreathFloor();
        ceilingVal = m_userSettings.getBreathCeiling();
      } else if (subMenuSelection == 1) {
        curveVal = m_userSettings.getPinchCurve();
        floorVal = m_userSettings.getPinchFloor();
        ceilingVal = m_userSettings.getPinchCeiling();
      } else if (subMenuSelection == 2) {
        curveVal = m_userSettings.getExpCurve();
        floorVal = m_userSettings.getExpFloor();
        ceilingVal = m_userSettings.getExpCeiling();
      } else if (subMenuSelection == 3) {
        curveVal = m_userSettings.getTiltCurve();
        floorVal = m_userSettings.getTiltFloor();
        ceilingVal = m_userSettings.getTiltCeiling();
      } else if (subMenuSelection == 4) {
        curveVal = m_userSettings.getNodCurve();
        floorVal = m_userSettings.getNodFloor();
        ceilingVal = m_userSettings.getNodCeiling();
      }
      
      // Override with edit values if editing
      if (inlineEditMode) {
        if (thirdMenuSelection == 1) curveVal = editValue;
        else if (thirdMenuSelection == 2) floorVal = editValueFloat;
        else if (thirdMenuSelection == 3) ceilingVal = editValueFloat;
      }
      
      // Erase old green line by clearing to background, then redraw what should be there
      // First pass: clear everything to background
      for (int y = graphY + 1; y < graphBottom; y++) {
        if (prevX != graphX) { // Don't overwrite Y-axis
          tft.drawPixel(prevX, y, COLOR_BACKGROUND);
        }
      }
      
      // Redraw axes
      tft.drawPixel(prevX, graphY, COLOR_HEADER_LINE);
      tft.drawPixel(prevX, graphBottom, COLOR_HEADER_LINE);
      
      // Redraw the curve segment at the old X position
      // Calculate the Y positions for this X and adjacent X positions to draw line segment
      int xOffset = prevX - graphX;
      if (xOffset >= 0 && xOffset <= graphWidth) {
        float inputNorm = prevSensorValue;
        
        // Calculate Y for current position
        float outputNorm;
        if (inputNorm < floorVal) {
          outputNorm = 0.0;
        } else if (inputNorm > ceilingVal) {
          outputNorm = 1.0;
        } else {
          float mappedInput = (inputNorm - floorVal) / (ceilingVal - floorVal);
          if (curveVal == 1) {
            outputNorm = mappedInput;
          } else if (curveVal == 2) {
            outputNorm = pow(mappedInput, 2.0);
          } else if (curveVal == 3) {
            outputNorm = pow(mappedInput, 0.5);
          } else {
            outputNorm = mappedInput * mappedInput * (3.0 - 2.0 * mappedInput);
          }
        }
        int y1 = graphBottom - (int)(outputNorm * graphHeight);
        
        // Calculate Y for previous X position (if exists)
        if (xOffset > 0) {
          float prevInput = (float)(xOffset - 1) / graphWidth;
          float prevOutput;
          if (prevInput < floorVal) {
            prevOutput = 0.0;
          } else if (prevInput > ceilingVal) {
            prevOutput = 1.0;
          } else {
            float mappedInput = (prevInput - floorVal) / (ceilingVal - floorVal);
            if (curveVal == 1) {
              prevOutput = mappedInput;
            } else if (curveVal == 2) {
              prevOutput = pow(mappedInput, 2.0);
            } else if (curveVal == 3) {
              prevOutput = pow(mappedInput, 0.5);
            } else {
              prevOutput = mappedInput * mappedInput * (3.0 - 2.0 * mappedInput);
            }
          }
          int y0 = graphBottom - (int)(prevOutput * graphHeight);
          
          // Draw line segment from previous to current
          tft.drawLine(prevX - 1, y0, prevX, y1, COLOR_ACCENT);
        }
        
        // Calculate Y for next X position (if exists)
        if (xOffset < graphWidth) {
          float nextInput = (float)(xOffset + 1) / graphWidth;
          float nextOutput;
          if (nextInput < floorVal) {
            nextOutput = 0.0;
          } else if (nextInput > ceilingVal) {
            nextOutput = 1.0;
          } else {
            float mappedInput = (nextInput - floorVal) / (ceilingVal - floorVal);
            if (curveVal == 1) {
              nextOutput = mappedInput;
            } else if (curveVal == 2) {
              nextOutput = pow(mappedInput, 2.0);
            } else if (curveVal == 3) {
              nextOutput = pow(mappedInput, 0.5);
            } else {
              nextOutput = mappedInput * mappedInput * (3.0 - 2.0 * mappedInput);
            }
          }
          int y2 = graphBottom - (int)(nextOutput * graphHeight);
          
          // Draw line segment from current to next
          tft.drawLine(prevX, y1, prevX + 1, y2, COLOR_ACCENT);
        }
      }
      
      // Redraw axes to ensure they're not left with artifacts
      tft.drawPixel(prevX, graphY, COLOR_HEADER_LINE);
      tft.drawPixel(prevX, graphBottom, COLOR_HEADER_LINE);
    }
    
    // Draw new green line (avoiding Y-axis)
    if (currentX != graphX) {
      for (int y = graphY + 1; y < graphBottom; y++) {
        tft.drawPixel(currentX, y, ILI9341_GREEN);
      }
    }
    
    // Clear previous value text
    if (prevSensorValue >= 0.0) {
      tft.fillRect(260, graphBottom + 2, 55, 10, COLOR_BACKGROUND);
    }
    
    // Draw current value text in bottom right (size 1)
    tft.setTextSize(1);
    tft.setTextColor(ILI9341_GREEN);
    tft.setCursor(265, graphBottom + 2);
    tft.print(sensorValue, 2);
    
    // Reset text size to default for menus
    tft.setTextSize(2);
    
    prevSensorValue = sensorValue;
  }
}

void DisplayHandler::redrawEditValue() {
  // Calculate which line the current selection is on
  int currentSelection = (menuDepth == 2) ? subMenuSelection : thirdMenuSelection;
  int* scrollOffset = (menuDepth == 2) ? &subMenuScroll : &thirdMenuScroll;
  int screenPos = currentSelection - *scrollOffset;
  
  // Only redraw if visible on screen
  if (screenPos < 0 || screenPos >= maxVisibleItems) {
    return;
  }
  
  int y = 50 + (screenPos * 25);
  const int rightMargin = 10;
  
  // Get the appropriate menu items array to find the label
  const char** items = nullptr;
  if (menuDepth == 2) {
    if (mainMenuSelection == 0) items = sensorsSubMenu;
    else if (mainMenuSelection == 1) items = midiSubMenu;
    else if (mainMenuSelection == 2) items = deviceSettingsSubMenu;
  } else {
    items = sensorDetailMenu; // All sensors use same detail menu
  }
  
  // Print label
  tft.setTextSize(2);
  tft.setTextColor(COLOR_SELECTION_TEXT);
  tft.setCursor(15, y);
  tft.print(items[currentSelection]);
  
  // Calculate value position and clear area
  int valueX = 0;
  String valueStr = "";
  
  // Build value string to calculate its width
  if (menuDepth == 2) {
    if (mainMenuSelection == 1) { // MIDI
      if (subMenuSelection == 0 || (subMenuSelection >= 1 && subMenuSelection <= 5)) {
        valueStr = String((int)editValue);
      } else if (subMenuSelection == 6 || subMenuSelection == 7) {
        valueStr = editValue > 0 ? "ON" : "OFF";
      }
    } else if (mainMenuSelection == 2) { // Device
      if (subMenuSelection == 0) {
        valueStr = String((int)editValue);
      } else if (subMenuSelection == 1) {
        valueStr = String((int)editValue) + "s";
      }
    }
  } else { // menuDepth == 3
    // New sensor structure: thirdMenuSelection 0-3 = Calibration, Curve, Floor, Ceiling
    if (thirdMenuSelection == 0 || thirdMenuSelection == 2 || thirdMenuSelection == 3) {
      // Calibration, Floor, Ceiling are floats
      valueStr = String(editValueFloat, 2);
    } else {
      // Curve is int
      valueStr = String((int)editValue);
    }
  }
  
  // Calculate right-justified X position
  valueX = 320 - rightMargin - (valueStr.length() * 12);
  
  // Clear the entire value area from after label to right edge
  tft.fillRect(tft.getCursorX() + 5, y - 2, 320 - tft.getCursorX() - 5, 22, COLOR_SELECTION_BG);
  
  // Draw the value right-justified in accent color if editing, white if not
  tft.setTextColor(inlineEditMode ? COLOR_ACCENT : COLOR_VALUE_NORMAL);
  tft.setCursor(valueX, y);
  
  // Print the value
  if (menuDepth == 2) {
    if (mainMenuSelection == 1) { // MIDI
      if (subMenuSelection == 0 || (subMenuSelection >= 1 && subMenuSelection <= 5)) {
        tft.print((int)editValue);
      } else if (subMenuSelection == 6 || subMenuSelection == 7) {
        tft.print(editValue > 0 ? "ON" : "OFF");
      }
    } else if (mainMenuSelection == 2) { // Device
      if (subMenuSelection == 0) {
        tft.print((int)editValue);
      } else if (subMenuSelection == 1) {
        tft.print((int)editValue);
        tft.print("s");
      }
    }
  } else { // menuDepth == 3
    // New sensor structure: thirdMenuSelection 0-3 = Calibration, Curve, Floor, Ceiling
    if (thirdMenuSelection == 0 || thirdMenuSelection == 2 || thirdMenuSelection == 3) {
      // Calibration, Floor, Ceiling are floats
      tft.print(editValueFloat, 2);
    } else {
      // Curve is int
      tft.print((int)editValue);
    }
  }
  
  // Redraw curve if in sensor detail menu (shows live preview while editing)
  if (menuDepth == 3 && mainMenuSelection == 0) {
    // Clear curve area first (excluding sensor value text area at bottom)
    tft.fillRect(15, 145, 290, 86, COLOR_BACKGROUND);
    drawResponseCurve();
    // Force sensor indicator update on next cycle
    prevSensorValue = -1.0;
  }
}

void DisplayHandler::drawSensorValues() {
  tft.fillScreen(COLOR_BACKGROUND);
  tft.setTextColor(COLOR_HEADER_TEXT);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.println("SENSOR VALUES");
  
  tft.drawLine(0, 30, 320, 30, COLOR_HEADER_LINE);
  
  tft.setTextColor(COLOR_MENU_TEXT);
  tft.setTextSize(1);
  
  int y = 45;
  tft.setCursor(10, y);
  tft.print("Breath:     ");
  tft.print(m_sensorCache.getBreathRaw());
  tft.print(" (");
  tft.print(m_sensorCache.getBreathNormalized(), 2);
  tft.print(")");
  
  y += 20;
  tft.setCursor(10, y);
  tft.print("Pinch:      ");
  tft.print(m_sensorCache.getPinchRaw());
  tft.print(" (");
  tft.print(m_sensorCache.getPinchNormalized(), 2);
  tft.print(")");
  
  y += 20;
  tft.setCursor(10, y);
  tft.print("Expression: ");
  tft.print(m_sensorCache.getExpressionRaw());
  tft.print(" (");
  tft.print(m_sensorCache.getExpressionNormalized(), 2);
  tft.print(")");
  
  if (m_sensorCache.isIMUAvailable()) {
    y += 30;
    tft.setCursor(10, y);
    tft.setTextColor(COLOR_VALUE_POSITIVE);
    tft.print("IMU Available");
    
    tft.setTextColor(COLOR_VALUE_NORMAL);
    y += 20;
    tft.setCursor(10, y);
    tft.print("Accel: ");
    tft.print(m_sensorCache.getAccelX(), 1);
    tft.print(", ");
    tft.print(m_sensorCache.getAccelY(), 1);
    tft.print(", ");
    tft.print(m_sensorCache.getAccelZ(), 1);
    
    y += 15;
    tft.setCursor(10, y);
    tft.print("Gyro:  ");
    tft.print(m_sensorCache.getGyroX(), 1);
    tft.print(", ");
    tft.print(m_sensorCache.getGyroY(), 1);
    tft.print(", ");
    tft.print(m_sensorCache.getGyroZ(), 1);
  } else {
    y += 30;
    tft.setCursor(10, y);
    tft.setTextColor(COLOR_VALUE_NEGATIVE);
    tft.print("IMU Not Available");
  }
  
  tft.setTextColor(COLOR_ACCENT);
  tft.setTextSize(1);
  tft.setCursor(10, 220);
  tft.print("LEFT: Back");
}

void DisplayHandler::drawAbout() {
  tft.fillScreen(COLOR_BACKGROUND);
  
  tft.setTextColor(COLOR_HEADER_TEXT);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.println("About");
  
  tft.drawLine(0, 30, 320, 30, COLOR_HEADER_LINE);
  
  // Draw small logo centered (240x180)
  int logoX = (320 - LOGO_SMALL_WIDTH) / 2;  // Center horizontally
  int logoY = 32;  // Below header
  tft.drawRGBBitmap(logoX, logoY, logo_small, LOGO_SMALL_WIDTH, LOGO_SMALL_HEIGHT);
  
  // "A MIDI controller." text below logo (overlapping)
  int y = logoY + LOGO_SMALL_HEIGHT - 20;
  tft.setTextColor(COLOR_MENU_TEXT);
  tft.setTextSize(2);
  String controllerText = "A MIDI controller";
  int textWidth = controllerText.length() * 12;  // Size 2 is 12px per char
  tft.setCursor((320 - textWidth) / 2, y);
  tft.print(controllerText);
  
  // Firmware version below
  y += 20;
  tft.setTextColor(COLOR_HEADER_TEXT);
  tft.setTextSize(1);
  String versionText = "Firmware Version: " + String(UserSettings::FIRMWARE_VERSION);
  textWidth = versionText.length() * 6;  // Size 1 is 6px per char
  tft.setCursor((320 - textWidth) / 2, y);
  tft.print(versionText);
}

void DisplayHandler::drawEditMode() {
  if (needsFullRedraw) {
    tft.fillScreen(COLOR_BACKGROUND);
    tft.setTextColor(COLOR_ACCENT);
    tft.setTextSize(2);
    tft.setCursor(10, 10);
    tft.println("EDIT MODE");
    
    tft.drawLine(0, 30, 320, 30, COLOR_ACCENT);
    
    tft.setTextColor(COLOR_MENU_TEXT);
    tft.setCursor(20, 60);
    
    // Show what's being edited based on current menu structure
    if (mainMenuSelection == 0 && menuDepth == 3) { // Sensors submenu
      if (subMenuSelection == 1) { // Calibration
        if (thirdMenuSelection == 0) tft.print("Breath Calibration");
        else if (thirdMenuSelection == 1) tft.print("Pinch Calibration");
        else if (thirdMenuSelection == 2) tft.print("Expression Calibration");
        else if (thirdMenuSelection == 3) tft.print("Tilt Calibration");
        else if (thirdMenuSelection == 4) tft.print("Nod Calibration");
      } else if (subMenuSelection == 2) { // Sensor Settings
        if (thirdMenuSelection == 0) tft.print("Breath Curve");
        else if (thirdMenuSelection == 1) tft.print("Breath Floor");
        else if (thirdMenuSelection == 2) tft.print("Breath Ceiling");
        else if (thirdMenuSelection == 3) tft.print("Pinch Curve");
        else if (thirdMenuSelection == 4) tft.print("Pinch Floor");
        else if (thirdMenuSelection == 5) tft.print("Pinch Ceiling");
        else if (thirdMenuSelection == 6) tft.print("Expression Curve");
        else if (thirdMenuSelection == 7) tft.print("Expression Floor");
        else if (thirdMenuSelection == 8) tft.print("Expression Ceiling");
        else if (thirdMenuSelection == 9) tft.print("Tilt Curve");
        else if (thirdMenuSelection == 10) tft.print("Tilt Floor");
        else if (thirdMenuSelection == 11) tft.print("Tilt Ceiling");
        else if (thirdMenuSelection == 12) tft.print("Nod Curve");
        else if (thirdMenuSelection == 13) tft.print("Nod Floor");
        else if (thirdMenuSelection == 14) tft.print("Nod Ceiling");
      }
    } else if (mainMenuSelection == 1) { // MIDI
      if (subMenuSelection == 0) tft.print("MIDI Channel");
      else if (subMenuSelection == 1) tft.print("Breath CC");
      else if (subMenuSelection == 2) tft.print("Pinch CC");
      else if (subMenuSelection == 3) tft.print("Expression CC");
      else if (subMenuSelection == 4) tft.print("Tilt CC");
      else if (subMenuSelection == 5) tft.print("Nod CC");
      else if (subMenuSelection == 6) tft.print("USB MIDI");
      else if (subMenuSelection == 7) tft.print("Hardware MIDI");
    } else if (mainMenuSelection == 2) { // Device Settings
      if (subMenuSelection == 0) tft.print("Display Brightness");
      else if (subMenuSelection == 1) tft.print("Sleep Timeout (s)");
    }
    
    // Instructions
    tft.setTextSize(1);
    tft.setTextColor(COLOR_HEADER_TEXT);
    tft.setCursor(20, 180);
    tft.println("UP/DOWN: Change value");
    tft.setCursor(20, 195);
    tft.println("RIGHT: Save    LEFT: Cancel");
    
    needsFullRedraw = false;
  }
  
  // Always update value (erase old, draw new)
  bool valueChanged = editingFloat ? (prevEditValueFloat != editValueFloat) : (prevEditValue != editValue);
  if (valueChanged || needsFullRedraw) {
    // Clear value area
    tft.fillRect(90, 105, 140, 35, COLOR_BACKGROUND);
    
    // Show current value in large font
    tft.setTextSize(4);
    tft.setTextColor(COLOR_SELECTION_TEXT);
    tft.setCursor(100, 110);
    if (editingFloat) {
      tft.print(editValueFloat, 2);
      prevEditValueFloat = editValueFloat;
    } else {
      // Check for boolean ON/OFF settings
      if (mainMenuSelection == 1 && (subMenuSelection == 6 || subMenuSelection == 7)) {  // USB/HW MIDI
        tft.print(editValue > 0 ? "ON" : "OFF");
      } else {
        tft.print(editValue);
      }
      prevEditValue = editValue;
    }
  }
}

void DisplayHandler::drawConfirmDialog() {
  tft.fillRect(40, 70, 240, 100, 0x2945);
  tft.drawRect(40, 70, 240, 100, COLOR_HEADER_LINE);
  tft.drawRect(41, 71, 238, 98, COLOR_HEADER_LINE);
  
  tft.setTextColor(COLOR_ACCENT);
  tft.setTextSize(2);
  
  // Show appropriate title based on reset type (centered)
  int titleX = 160; // Center X
  if (pendingResetType == 1) {
    titleX = 160 - (11 * 12) / 2; // "RESET CAL?" = 11 chars
    tft.setCursor(titleX, 85);
    tft.println("RESET CAL?");
  } else if (pendingResetType == 2) {
    titleX = 160 - (14 * 12) / 2; // "RESET CURVES?" = 14 chars
    tft.setCursor(titleX, 85);
    tft.println("RESET CURVES?");
  } else if (pendingResetType == 3) {
    titleX = 160 - (12 * 12) / 2; // "RESET MIDI?" = 12 chars
    tft.setCursor(titleX, 85);
    tft.println("RESET MIDI?");
  } else if (pendingResetType == 5) {
    titleX = 160 - (15 * 12) / 2; // "RESET SENSORS?" = 15 chars
    tft.setCursor(titleX, 85);
    tft.println("RESET SENSORS?");
  } else {
    titleX = 160 - (15 * 12) / 2; // "FACTORY RESET?" = 15 chars
    tft.setCursor(titleX, 85);
    tft.println("FACTORY RESET?");
  }
  
  tft.setTextColor(COLOR_MENU_TEXT);
  tft.setTextSize(1);
  // "Are you sure you want to" = 25 chars * 6 pixels = 150 pixels, centered at 160
  tft.setCursor(160 - 150/2, 110);
  tft.println("Are you sure you want to");
  
  if (pendingResetType == 4) {
    // "reset ALL settings?" = 19 chars * 6 pixels = 114 pixels
    tft.setCursor(160 - 114/2, 122);
    tft.println("reset ALL settings?");
  } else {
    // "reset to default values?" = 25 chars * 6 pixels = 150 pixels
    tft.setCursor(160 - 150/2, 122);
    tft.println("reset to default values?");
  }
  
  tft.setTextSize(2);
  tft.setCursor(65, 145);
  tft.setTextColor(COLOR_VALUE_NEGATIVE);
  tft.print("<: No");
  tft.setTextColor(COLOR_VALUE_POSITIVE);
  tft.setCursor(175, 145);
  tft.print(">: Yes");
}

void DisplayHandler::sleep() {
  displayState = DisplayState::DISPLAY_OFF;
    
  // Dim the backlight using PWM (adjust value 0-255, lower = dimmer)
  analogWrite(TFT_BL, 20);

  // Show logo dimly
  tft.fillScreen(COLOR_BACKGROUND);
  int16_t x = (320 - LOGO_WIDTH) / 2;
  int16_t y = (240 - LOGO_HEIGHT) / 2;
  tft.drawRGBBitmap(x, y, logo, LOGO_WIDTH, LOGO_HEIGHT);
  
}

void DisplayHandler::wake() {
  displayState = DisplayState::DISPLAY_ON;
  analogWrite(TFT_BL, m_userSettings.getDisplayBrightness());
  lastActivityTime = millis();
  changeMenuState(MenuState::MAIN_MENU);
}