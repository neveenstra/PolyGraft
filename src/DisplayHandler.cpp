#include "DisplayHandler.h"

const int LCD_COLS = 20;
const int LCD_ROWS = 4;

const unsigned long LOADING_DURATION = 2000;

void DisplayHandler::begin(){

  int status = lcd.begin(LCD_COLS, LCD_ROWS);
	if(status){
		hd44780::fatalError(status);
	}

  lcd.clear();
	lcdState = LCDStates::DISPLAY_ON;
	menuState = MenuStates::LOADING;
	inputState = InputStates::NAVIGATE;
	stateStartTime = millis();
  lastActivityTime = millis();

	showLoadingScreen();
}

void DisplayHandler::update(){
 		unsigned long currentTime = millis();
    
    // Check for sleep timeout
    if (lcdState == LCDStates::DISPLAY_ON) {
        if (currentTime - lastActivityTime > sleepTimeout) {
            sleep();
            return;
        }
    }
    
    // State machine logic
    switch(menuState) {
        case MenuStates::LOADING:
            if (currentTime - stateStartTime > LOADING_DURATION) {
                changeMenuState(MenuStates::MENU);
            }
            break;
            
        case MenuStates::MENU:
            // Menu is shown, waiting for user input
            break;
            
        case MenuStates::DIAGNOSTIC:
						//
            break;
            
        case MenuStates::CONFIG:
            // Config screen logic
            break;
		}
}

void DisplayHandler::pressUp(){
	if (lcdState == LCDStates::DISPLAY_OFF) {
			wake();
			return;
	}
	
	lastActivityTime = millis();
}

void DisplayHandler::pressDown(){
	if (lcdState == LCDStates::DISPLAY_OFF) {
			wake();
			return;
	}
	
	lastActivityTime = millis();
}

void DisplayHandler::pressEnter(){
	if (lcdState == LCDStates::DISPLAY_OFF) {
			wake();
			return;
	}
	
	lastActivityTime = millis();
}

void DisplayHandler::pressBack(){    
	if (lcdState == LCDStates::DISPLAY_OFF) {
        wake();
        return;
    }
    
    lastActivityTime = millis();
}

void DisplayHandler::changeMenuState(MenuStates newState){
    menuState = newState;
    stateStartTime = millis();
    
    switch(newState) {
        case MenuStates::LOADING:
            showLoadingScreen();
            break;
        case MenuStates::MENU:
            menuSelection = 0;
           	showMenu();
            break;
        case MenuStates::DIAGNOSTIC:
            showDiagnosticScreen();
            break;
        case MenuStates::CONFIG:
            showConfigScreen();
            break;
    }
}

void DisplayHandler::showLoadingScreen(){
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("     Loading...     ");
}
void DisplayHandler::showMenu(){
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("==== MAIN MENU ====");
    
    lcd.setCursor(0, 1);
    if (menuSelection == 0) lcd.print("> ");
    else lcd.print("  ");
    lcd.print("Diagnostic");
    
    lcd.setCursor(0, 2);
    if (menuSelection == 1) lcd.print("> ");
    else lcd.print("  ");
    lcd.print("Config");
}

void DisplayHandler::showDiagnosticScreen(){
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("=== DIAGNOSTIC ===");
    // Add your diagnostic data here
}

void DisplayHandler::showConfigScreen(){
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("===== CONFIG =====");
    // Add your config options here
}

void DisplayHandler::sleep(){
    lcdState = LCDStates::DISPLAY_OFF;
    lcd.noBacklight();
    lcd.noDisplay();
}

void DisplayHandler::wake(){
    lcdState = LCDStates::DISPLAY_ON;
    lcd.display();
    lcd.backlight();
    lastActivityTime = millis();
    changeMenuState(MenuStates::MENU);
}

void DisplayHandler::setSleepTimeout(unsigned long timeout){
    sleepTimeout = timeout;
}

/*
void DisplayHandler::write(const char* sayThis){
  lcd.setCursor(0, 0);
  lcd.print(sayThis);
}
*/