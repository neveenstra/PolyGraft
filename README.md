# PolyGraft

test harness pins:
blue 17
purple 16
orange 18
brown 19

display pins:
TFT_DC 7
TFT_BL 8
TFT_RST 9
TFT_CS 10
TFT_DIN 11
TFT_MISO 12 (not used)
TFT_CLK 13

switch pins:
DOWN 2
LEFT 3
UP 4
RIGHT 5

other pins:
MIDI 14
PRESSURE 15 
EXPRESSION 23

  // Calculate centered position for logo
  int16_t x = (320 - LOGO_WIDTH) / 2;
  int16_t y = (240 - LOGO_HEIGHT) / 2;
  
  // Draw the logo
  tft.drawRGBBitmap(x, y, logo, LOGO_WIDTH, LOGO_HEIGHT);
