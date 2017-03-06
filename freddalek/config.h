#ifndef Config_h
#define Config_h
#define DEBUG

// I2C Addresses
#define DOMEACEADDR 0x00    // 0x20
#define EYEACEADDR  0x01    // 0x21
#define HEADACEADDR 0x02    // 0x22
#define LCDADDR     0x26
#define KPADDR      0x27

// UNO pin assignments
#define eyePWM 10
#define eyeM1  11
#define eyeM2  12
#define domePWM 9
#define domeM1  8
#define domeM2  7

#define LED_HEAD_GREEN  2 // head indicator
#define LED_HEAD_YELLOW 3 // head indicator
#define LED_HEAD_RED    4 // head indicator

// EEPROM locations for saved settings
#define EEPROM_eyeZero     0
#define EEPROM_eyeMax      1
#define EEPROM_eyeMin      2
#define EEPROM_domeZero    3
#define EEPROM_domeMax     4
#define EEPROM_domeMin     5
#define EEPROM_headZeroV   6
#define EEPROM_headZeroH   7
#define EEPROM_headUp      8
#define EEPROM_headDown    9
#define EEPROM_headLeft   10
#define EEPROM_headRight  11

// movement defs
#define MOVE_STOP 0
#define MOVE_UP 1
#define MOVE_DOWN -1
#define MOVE_RIGHT 2
#define MOVE_LEFT -2

// motor minimum PWM
#define EYE_MINSPEED_UP 170
#define EYE_MINSPEED_DOWN 150
#define DOME_MINSPEED 160
#define eyeUpAccel 20
#define eyeDownAccel 15
#define domeAccel 1
#define domeDecel 3

// mode
#define MODE_ZERO 0
#define MODE_FOLLOW 1
#define MODE_AUTO 2

// submodes
#define MODE_ZERO_WAIT 0
#define MODE_ZERO_GOTO_ZERO 1
#define MODE_ZERO_SET_HEAD 2
#define MODE_ZERO_SET_EYE 3
#define MODE_ZERO_SET_DOME 4
#define MODE_FOLLOW_NORMAL 0
#define MODE_FOLLOW_LEVEL 1
#define MODE_AUTO_BASIC 0
#define MODE_AUTO_SCAN 1
#define MODE_AUTO_SCAN_PAUSE 0
#define MODE_AUTO_SCAN_RIGHT 1
#define MODE_AUTO_SCAN_LEFT 2

#include "Wire.h"
#include "WiiChuck.h"
extern WiiChuck chuck;

// keypad
#include <Keypad_MCP.h>
#include <Wire.h>
#include <Keypad.h>
extern Keypad_MCP keypad;

// LCD
#include <LiquidCrystal_PCF8574.h>
extern LiquidCrystal_PCF8574 lcd;

#include <ACE128.h>
extern ACE128 eyeACE;
extern ACE128 domeACE;
extern ACE128 headACE;


#endif
