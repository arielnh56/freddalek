#include "config.h"

#define KPROWS 4 //four rows
#define KPCOLS 3 //three columns
const char keys[KPROWS][KPCOLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};

byte rowPins[KPROWS] = {5, 0, 1, 3}; //connect to the row pinouts of the keypad
byte colPins[KPCOLS] = {4, 6, 2}; //connect to the column pinouts of the keypad

Keypad_MCP keypad = Keypad_MCP( makeKeymap(keys), rowPins, colPins, KPROWS, KPCOLS, KPADDR );
LiquidCrystal_PCF8574 lcd(LCDADDR);  // set the LCD address to 0x27 for a 16 chars and 2 line display

#include <ACE128map87651234.h>
#include <ACE128map87654321.h>

ACE128 eyeACE((uint8_t)EYEACEADDR, (uint8_t*)encoderMap_87651234, EEPROM_eyeACE);
ACE128 domeACE((uint8_t)DOMEACEADDR, (uint8_t*)encoderMap_87651234, EEPROM_domeACE);
ACE128 headACE((uint8_t)HEADACEADDR, (uint8_t*)encoderMap_87651234, EEPROM_headACE);
ACE128 handACE((uint8_t)HANDACEADDR, (uint8_t*)encoderMap_87654321);

WiiChuck chuck = WiiChuck();


