  #define DEBUG
#include <EEPROM.h>
#include <ACE128.h>
#include <ACE128map87651234.h>
#include <math.h>

#include "Wire.h"
#include "WiiChuck.h"
//#include "nunchuck_funcs.h"

// keypad
#include <Keypad_MCP.h>
#include <Wire.h>
#include <Keypad.h>

#define KPADDR 0x27

const byte ROWS = 4; //four rows
const byte COLS = 3; //three columns
const char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
byte rowPins[ROWS] = {5, 0, 1, 3}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {4, 6, 2}; //connect to the column pinouts of the keypad

Keypad_MCP keypad = Keypad_MCP( makeKeymap(keys), rowPins, colPins, ROWS, COLS, KPADDR );

WiiChuck chuck = WiiChuck();

ACE128 eyeACE((uint8_t)1, (uint8_t*)encoderMap_87651234);
const byte eyePWM = 10;
const byte eyeM1 = 11;
const byte eyeM2 = 12;

ACE128 domeACE((uint8_t)0, (uint8_t*)encoderMap_87651234);
const byte domePWM = 9;
const byte domeM1 = 8;
const byte domeM2 = 7;

ACE128 headACE((uint8_t)2, (uint8_t*)encoderMap_87651234);

// EEPROM locations for saved settings
const int EEPROM_eyeZero   =  0;
const int EEPROM_eyeMax    =  1;
const int EEPROM_eyeMin    =  2;
const int EEPROM_domeZero  =  3;
const int EEPROM_domeMax   =  4;
const int EEPROM_domeMin   =  5;
const int EEPROM_headZeroV =  6;
const int EEPROM_headZeroH =  7;
const int EEPROM_headUp    =  8;
const int EEPROM_headDown  =  9;
const int EEPROM_headLeft  = 10;
const int EEPROM_headRight = 11;

const uint8_t LOOPTIME = 25;
// corresponding vars
int8_t eyeMin, domeZero, headZeroH;  // ACE raw pos
int8_t domeMax, domeMin, headLeft, headRight; // relative -64 to 63
uint8_t eyeMax, eyeZero;
uint8_t headUp, headDown, headZeroV; // angle 90ish = horizontal

// current positions
int8_t domePos;
int8_t headPos;
uint8_t domeSpeed;
uint8_t targetDomeSpeed;
uint8_t eyePos;
uint8_t eyeSpeed;
uint8_t targetEyeSpeed;
const int MOVE_STOP=0;
const int MOVE_UP=1;
const int MOVE_DOWN=-1;
const int MOVE_RIGHT=2;
const int MOVE_LEFT=-2;
int8_t eyeDir=MOVE_STOP;
int8_t domeDir=MOVE_STOP;
const int EYE_MINSPEED_UP=170;
const int EYE_MINSPEED_DOWN=150;
int EYE_MAXSPEED=255;
const int eyeUpAccel=20;
const int eyeDownAccel=15;
const int DOME_MINSPEED=160;
int DOME_MAXSPEED=255;
const int domeAccel=1;
const int domeDecel=3;

// global temps for scratch use
uint8_t tmp_u8;
int8_t tmp_8;
int16_t tmp_16;

// mode
#define MODE_ZERO 0
#define MODE_FOLLOW 1
#define MODE_AUTO 2
byte mode;
byte mode_2;
byte mode_3;
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
uint32_t MODE_AUTO_SCAN_TIMER=0;
uint32_t mode_auto_lastchange;

// head indicator
const byte LED_HEAD_GREEN=2;
const byte LED_HEAD_YELLOW=3;
const byte LED_HEAD_RED=4;

uint8_t eyeTarget;
uint32_t eyeLastChange;
int8_t domeTarget;
uint32_t domeLastChange;
uint32_t lastLoop;

void setup() {
  TCCR1B = TCCR1B & 0b11111000 | 0x01;
#ifdef DEBUG
  Serial.begin(115200);
#endif
  chuck.begin();
  chuck.update();
  keypad.begin( );

  eyeACE.begin();
  domeACE.begin();
  headACE.begin();
  headACE.reverse(true);

  pinMode(eyeM1, OUTPUT);
  digitalWrite(eyeM1, LOW);
  pinMode(eyeM2, OUTPUT);
  digitalWrite(eyeM2, LOW);
  pinMode(eyePWM, OUTPUT);
  analogWrite(eyePWM, 0);

  pinMode(domeM1, OUTPUT);
  digitalWrite(domeM1, LOW);
  pinMode(domeM2, OUTPUT);
  digitalWrite(domeM2, LOW);
  pinMode(domePWM, OUTPUT);
  analogWrite(domePWM, 0);

  tmp_u8 = EEPROM.read(EEPROM_eyeMin);
  if (tmp_u8 <128) {
    eyeMin = tmp_u8;
  } else {
    eyeMin = eyeACE.rawPos();
  }
  eyeACE.setZero(eyeMin);

  tmp_u8 = EEPROM.read(EEPROM_eyeMax);
  if (tmp_u8 <= 127) {
    eyeMax = tmp_u8;
  } else {
    eyeMax = 100;
  }

  tmp_u8 = EEPROM.read(EEPROM_eyeZero);
  if (tmp_u8 < eyeMax) {
    eyeZero = tmp_u8;
  } else {
    eyeZero = eyeMax - 20;
  }
  
  tmp_u8 = EEPROM.read(EEPROM_domeZero);
  if (tmp_u8 < 128) {
    domeZero = tmp_u8;
  } else {
    domeZero = domeACE.rawPos();
  }
  domeACE.setZero(domeZero);

  tmp_8 = EEPROM.read(EEPROM_domeMax);
  if (tmp_8 <= 63 && tmp_8 > 0) {
    domeMax = tmp_8;
  } else {
    domeMax = 32;
  }

  tmp_8 = EEPROM.read(EEPROM_domeMin);
  if (tmp_8 >= -63 && tmp_8 < 0) {
    domeMin = tmp_8;
  } else {
    domeMin = -32;
  }

  tmp_u8 = EEPROM.read(EEPROM_headZeroV);
  if (tmp_u8 < 120 && tmp_u8 > 60) {
    headZeroV = tmp_u8;
  } else {
    headZeroV = 90;
  }
 
  tmp_u8 = EEPROM.read(EEPROM_headUp);
  if (tmp_u8 < 180 && tmp_u8 > headZeroV + 30) {
    headUp = tmp_u8;
  } else {
    headUp = headZeroV + 30;
  }

  tmp_u8 = EEPROM.read(EEPROM_headDown);
  if (tmp_u8 > 0 && tmp_u8 < headZeroV - 20) {
    headDown = tmp_u8;
  } else {
    headDown = headZeroV - 30;
  }

  tmp_u8 = EEPROM.read(EEPROM_headZeroH);
  if (tmp_u8 < 128) {
    headZeroH = tmp_u8;
  } else {
    headZeroH = headACE.rawPos();
  }
  headACE.setZero(headZeroH);

  tmp_8 = EEPROM.read(EEPROM_headLeft);
  if (tmp_8 >= -64 && tmp_8 < 0) {
    headLeft = tmp_8;
  } else {
    headLeft = -32;
  }
  
  tmp_8 = EEPROM.read(EEPROM_headRight);
  if (tmp_8 <= 63 && tmp_8 > 0) {
    headRight = tmp_8;
  } else {
    headRight = 32;
  }
  
  mode = MODE_ZERO;
  mode_2 = MODE_ZERO_GOTO_ZERO;
  eyeTarget = eyeZero;
  eyeLastChange = millis();
  domeTarget = 0;
  domeLastChange = millis();

  pinMode(LED_HEAD_RED, OUTPUT);     
  pinMode(LED_HEAD_YELLOW, OUTPUT);     
  pinMode(LED_HEAD_GREEN, OUTPUT);     
  digitalWrite(LED_HEAD_RED, HIGH);
  digitalWrite(LED_HEAD_YELLOW, HIGH);
  digitalWrite(LED_HEAD_GREEN, HIGH);

  lastLoop = millis();
}



void loop() {
   char key = keypad.getKey();
   if (lastLoop + LOOPTIME > millis()) {
     delay(lastLoop + LOOPTIME - millis());
   }
   lastLoop = millis();
    
  chuck.update();
  int chuckPitch = chuck.readPitch10();
  eyePos = eyeACE.upos();
  domePos = domeACE.pos();
  headPos = headACE.pos();

  if (chuckPitch > headZeroV*10 + 15) {
    digitalWrite(LED_HEAD_RED, LOW);
  } else {
    digitalWrite(LED_HEAD_RED, HIGH);
  }
  if (abs(chuckPitch - headZeroV*10) < 30) {
    digitalWrite(LED_HEAD_YELLOW, LOW);
  } else {
    digitalWrite(LED_HEAD_YELLOW, HIGH);
  }
  if (chuckPitch < headZeroV*10 - 15) {
    digitalWrite(LED_HEAD_GREEN, LOW);
  } else {
    digitalWrite(LED_HEAD_GREEN, HIGH);
  }
   // motion decisions
  switch (mode) {
    case MODE_FOLLOW:
      switch (mode_2) {
        case MODE_FOLLOW_NORMAL:
          tmp_16 = chuckPitch - headZeroV*10;
          if (abs(tmp_16) < 30) {
            eyeTarget = eyeZero;
          } else if (tmp_16 > 0) { // up
            eyeTarget = constrain(eyeZero+(tmp_16-30)/10,eyeZero,eyeMax);            
          } else { // down
            eyeTarget = constrain(eyeZero+(tmp_16+30)/10,0,eyeZero);            
          }
          domeTarget = headPos;
          break;
 
       case MODE_FOLLOW_LEVEL:
          eyeTarget = eyeZero;
          domeTarget = headPos;
          break;
      }
      break;
    case MODE_ZERO:
       if (mode_2 == MODE_ZERO_WAIT) {
         eyeTarget = eyePos;
         domeTarget = domePos;
         eyeDir = MOVE_STOP;
         domeDir = MOVE_STOP;
       }
       break;
    case MODE_AUTO:
      if (mode_2 == MODE_AUTO_SCAN) {
        if (mode_3 == MODE_AUTO_SCAN_RIGHT) {
          domeTarget = domeMax;
          if (domePos > 32) {
            mode_3 = MODE_AUTO_SCAN_LEFT;
          } 
        } else  if (mode_3 == MODE_AUTO_SCAN_LEFT) {
          domeTarget = domeMin;
          if (domePos < -32) {
            mode_3 = MODE_AUTO_SCAN_RIGHT;
          } 
        } 
      }
      break;
  }  
#ifdef DEBUG
//  domePos = domeACE.pos();
//  Serial.print(" domeRawPos ");
//  Serial.print(domeACE.rawPos());
//  Serial.print(" eyeRawPos ");
//  Serial.print(eyeACE.rawPos());
//  Serial.print("domePos ");
//  Serial.print(domePos);
//  Serial.print(" domeTarget ");
//  Serial.print(domeTarget);
//  Serial.print(" domeDir ");
//  Serial.print(domeDir);
//  Serial.print(" domeSpeed ");
//  Serial.print(domeSpeed);
//  Serial.print(" targetDomeSpeed ");
//  Serial.println(targetDomeSpeed);
Serial.print(millis());
Serial.print(" ");
Serial.print(micros());
Serial.print(" ");
Serial.print(" eyePos ");
Serial.print(eyePos);
Serial.print(" eyeTarget ");
Serial.print(eyeTarget);
Serial.print(" eyeDir ");
Serial.print(eyeDir);
Serial.print(" eyeSpeed ");
Serial.print(eyeSpeed);
Serial.print(" targetEyeSpeed ");
Serial.print(targetEyeSpeed);
Serial.print(" chuckPitch ");
Serial.println(chuckPitch);
#endif

  // motor control
  // target speed same dir
  // same dir?
  if (((eyeTarget > eyePos) && eyeDir == MOVE_UP)) {
    targetEyeSpeed = constrain(EYE_MINSPEED_UP-5+(abs(eyeTarget - eyePos)*5),EYE_MINSPEED_UP,EYE_MAXSPEED);
    if (eyeSpeed < targetEyeSpeed) { // speed up
      eyeSpeed=constrain(eyeSpeed + eyeUpAccel,EYE_MINSPEED_UP,EYE_MAXSPEED);
    } else if (eyeSpeed > targetEyeSpeed) { // slow down
      eyeSpeed=constrain(eyeSpeed - eyeDownAccel,EYE_MINSPEED_UP,EYE_MAXSPEED);
    }
  } else if ((eyeTarget < eyePos) && eyeDir == MOVE_DOWN) {
    targetEyeSpeed = constrain(EYE_MINSPEED_DOWN-5+(abs(eyeTarget - eyePos)*5),EYE_MINSPEED_DOWN,EYE_MAXSPEED);
    if (eyeSpeed < targetEyeSpeed) { // speed up
      eyeSpeed=constrain(eyeSpeed + eyeDownAccel,EYE_MINSPEED_UP,EYE_MAXSPEED);
    } else if (eyeSpeed > targetEyeSpeed) { // slow down
      eyeSpeed=constrain(eyeSpeed - eyeUpAccel,EYE_MINSPEED_UP,EYE_MAXSPEED);
    }
  } else if ((eyeTarget > eyePos) && eyeDir == MOVE_DOWN) { // direction change
    if (eyeSpeed > EYE_MINSPEED_DOWN) eyeSpeed=constrain(eyeSpeed - eyeUpAccel,EYE_MINSPEED_DOWN,EYE_MAXSPEED); // slow down
    if (eyeSpeed <= EYE_MINSPEED_DOWN) eyeDir = MOVE_UP;
  } else if ((eyeTarget < eyePos) && eyeDir == MOVE_UP) { // direction change
    if (eyeSpeed > EYE_MINSPEED_UP) eyeSpeed=constrain(eyeSpeed - eyeDownAccel,EYE_MINSPEED_UP,EYE_MAXSPEED); // slow down
    if (eyeSpeed <= EYE_MINSPEED_UP) eyeDir = MOVE_DOWN;
  } else if (eyeTarget==eyePos) { // stop
    eyeDir = MOVE_STOP;
    eyeSpeed=0;
  } else if (eyePos < eyeTarget+2 && eyeDir == MOVE_STOP) {
    eyeDir = MOVE_UP;
    eyeSpeed=EYE_MINSPEED_UP;
  } else if (eyePos > eyeTarget-2 && eyeDir == MOVE_STOP) {
    eyeDir = MOVE_DOWN;
    eyeSpeed=EYE_MINSPEED_DOWN;
  }

  // same dir?
  if (((domeTarget < domePos) && domeDir == MOVE_LEFT) || ((domeTarget > domePos) && domeDir == MOVE_RIGHT)) {
    targetDomeSpeed = constrain(DOME_MINSPEED-3+(abs(domeTarget - domePos)*3),DOME_MINSPEED,DOME_MAXSPEED);
    if (domeSpeed < targetDomeSpeed) { // speed up
      domeSpeed=constrain(domeSpeed + domeAccel,DOME_MINSPEED,DOME_MAXSPEED);
    } else if (domeSpeed > targetDomeSpeed) { // slow down
      domeSpeed=constrain(domeSpeed - domeDecel,DOME_MINSPEED,DOME_MAXSPEED);
    }
  } else if (((domeTarget > domePos) && domeDir == MOVE_LEFT) || ((domeTarget < domePos) && domeDir == MOVE_RIGHT)) {
    if (domeSpeed > DOME_MINSPEED) domeSpeed=constrain(domeSpeed - domeDecel,DOME_MINSPEED,DOME_MAXSPEED); // slow down
    if (domeSpeed <= DOME_MINSPEED) domeDir *= -1;
  } else if (domeTarget==domePos) { // stop
    domeDir = MOVE_STOP;
    domeSpeed=0;
  } else if (domePos < domeTarget && domeDir == MOVE_STOP) {
    domeDir = MOVE_RIGHT;
    domeSpeed=DOME_MINSPEED;
  } else if (domePos > domeTarget && domeDir == MOVE_STOP) {
    domeDir = MOVE_LEFT;
    domeSpeed=DOME_MINSPEED;
  }


  if (domeDir == MOVE_STOP) { // brake
    digitalWrite(domeM1,LOW);
    digitalWrite(domeM2,LOW);
    analogWrite(domePWM,0);
  } else if (domeDir == MOVE_LEFT) {
    digitalWrite(domeM1,LOW);
    digitalWrite(domeM2,HIGH);
    analogWrite(domePWM,domeSpeed);
  } else if (domeDir == MOVE_RIGHT) {
    digitalWrite(domeM1,HIGH);
    digitalWrite(domeM2,LOW);
    analogWrite(domePWM,domeSpeed);
  }

  if (eyeDir == MOVE_STOP) { // brake
    digitalWrite(eyeM1,LOW);
    digitalWrite(eyeM2,LOW);
    analogWrite(eyePWM,0);
  } else if (eyeDir == MOVE_UP) {
    digitalWrite(eyeM1,HIGH);
    digitalWrite(eyeM2,LOW);
    analogWrite(eyePWM,eyeSpeed);
  } else if (eyeDir == MOVE_DOWN) {
    digitalWrite(eyeM1,LOW);
    digitalWrite(eyeM2,HIGH);
    analogWrite(eyePWM,eyeSpeed);
  }
  if (key){
    
#ifdef DEBUG
    Serial.print("key press ");
    Serial.println(key);
#endif 
    DOME_MAXSPEED=255;
    EYE_MAXSPEED=255;
    switch (key) {
      case '0':
        mode = MODE_ZERO;
        mode_2 = MODE_ZERO_WAIT;            
        eyeTarget = eyeZero;
        break;
      case '*':
        mode = MODE_FOLLOW;
        mode_2 = MODE_FOLLOW_LEVEL;            
        break;
      case '#':
        mode = MODE_AUTO;
        mode_2 = MODE_AUTO_BASIC;            
        mode_auto_lastchange = millis();
        break;
    }

    switch (mode) {
      case MODE_ZERO:
        switch (mode_2) {
          case MODE_ZERO_WAIT:
            switch (key) {
              case '1':
                mode_2 = MODE_ZERO_SET_HEAD;
                break;
              case '2':
                mode_2 = MODE_ZERO_SET_EYE;
                break;
              case '3':
                mode_2 = MODE_ZERO_SET_DOME;  
                break;
            } // switch key
            break;          
          case MODE_ZERO_SET_HEAD:
            chuck.update();
            switch (key) {
              case '2': // top limit
                headUp = round(chuck.readPitch10()/10);
                EEPROM.write(EEPROM_headUp,headUp);
                break;
              case '8': // bottom limit
                headDown = round(chuck.readPitch10()/10);
                EEPROM.write(EEPROM_headDown,headDown);
                break;
              case '4': // left limit
                break;
              case '6': // right limit
                break;
              case '5': // center
                headZeroV = round(chuck.readPitch10()/10);
                EEPROM.write(EEPROM_headZeroV,headZeroV);
                headZeroH = headACE.rawPos();
                headACE.setZero(headZeroH);
                EEPROM.write(EEPROM_headZeroH,headZeroH);
                break;
            } // switch key
            break;          
          case MODE_ZERO_SET_EYE:
            switch (key) {
              case '1': // move up
                eyeTarget++;
                break;
              case '7': // move down
                eyeTarget--;
                break;
              case '2': // set max m- find upper stop
                digitalWrite(eyeM1,HIGH);
                digitalWrite(eyeM2,LOW);
                analogWrite(eyePWM,EYE_MINSPEED_UP);
                delay(20000);
                digitalWrite(eyeM1,LOW);
                digitalWrite(eyeM2,LOW);
                analogWrite(eyePWM,0);
                tmp_u8 = eyeACE.upos();
                eyeMax = tmp_u8 + 2; // overrun
                if (eyeMax > 127) eyeMax = eyeMax - 128; // roll
                EEPROM.write(EEPROM_eyeMax,eyeMax);
                eyeTarget = eyeMax;
                break;
              case '8': // set min - find lower stop
                digitalWrite(eyeM1,LOW);
                digitalWrite(eyeM2,HIGH);
                analogWrite(eyePWM,EYE_MINSPEED_DOWN);
                delay(10000);
                digitalWrite(eyeM1,LOW);
                digitalWrite(eyeM2,LOW);
                analogWrite(eyePWM,0);
                tmp_u8 = eyeACE.rawPos();
                eyeMin = tmp_u8 - 2; // overrun
                if (eyeMin > 127) eyeMin = eyeMin - 128; // roll
                EEPROM.write(EEPROM_eyeMin,eyeMin);
                eyeACE.setZero(eyeMin);
                eyeTarget = 0;
                break;
              case '5': // set zero
                eyeZero = constrain(eyeACE.upos(), 1, 127);
                EEPROM.write(EEPROM_eyeZero,eyeZero);
                break;
            } // switch key
            break;          
          case MODE_ZERO_SET_DOME:
            switch (key) {
              case '4': // move left
                domeACE.setZero(domeACE.rawPos());
                domeTarget=-1;
#ifdef DEBUG
  domePos = domeACE.pos();
  Serial.print("domePos ");
  Serial.println(domePos);
  Serial.print("domeTarget ");
  Serial.println(domeTarget);
#endif
                break;
              case '6': // move right
                domeACE.setZero(domeACE.rawPos());
                domeTarget=1;
#ifdef DEBUG
  domePos = domeACE.pos();
  Serial.print("domePos ");
  Serial.println(domePos);
  Serial.print("domeTarget ");
  Serial.println(domeTarget);
#endif
                break;
              case '5': // set zero and find stops
                tmp_u8 = domeACE.rawPos();
                EEPROM.write(EEPROM_domeZero,tmp_u8);
                domeACE.setZero(tmp_u8);
                domeTarget = 0;
                digitalWrite(domeM1,HIGH);
                digitalWrite(domeM2,LOW);
                analogWrite(domePWM,190);
                delay(10000);
                digitalWrite(domeM1,LOW);
                digitalWrite(domeM2,LOW);
                analogWrite(domePWM,0);
                tmp_8 = domeACE.pos();
                domeMax = tmp_8 - 1; // overrun
                EEPROM.write(EEPROM_domeMax,domeMax);
                digitalWrite(domeM1,LOW);
                digitalWrite(domeM2,HIGH);
                analogWrite(domePWM,190);
                delay(15000);
                digitalWrite(domeM1,LOW);
                digitalWrite(domeM2,LOW);
                analogWrite(domePWM,0);
                tmp_8 = domeACE.pos();
                domeMin = tmp_8 + 1; // overrun
                EEPROM.write(EEPROM_domeMin,domeMin);
#ifdef DEBUG
  domePos = domeACE.pos();
  Serial.print("domePos ");
  Serial.println(domePos);
  Serial.print("domeTarget ");
  Serial.println(domeTarget);
#endif
                break;
            } // switch key
            break;          
        } // switch mode_2
        break;
      case MODE_FOLLOW:
        switch (key) {
          case '1':
            mode_2 = MODE_FOLLOW_NORMAL;
            break;
          case '2':
            mode_2 = MODE_FOLLOW_LEVEL;
            eyeTarget=eyeZero;
            break;
        } // switch key
        break;
      case MODE_AUTO:
        switch (key) {
          case '1':
            mode_2 = MODE_AUTO_SCAN;
            mode_3 = MODE_AUTO_SCAN_RIGHT;
            DOME_MAXSPEED = DOME_MINSPEED + 15;
            break;
          case '4':
            mode_2 = MODE_AUTO_BASIC;
            domeTarget=-32;
            eyeTarget=eyeZero;
            break;
          case '5':
            mode_2 = MODE_AUTO_BASIC;
            domeTarget=0;
            eyeTarget=eyeZero;
            break;
          case '6':
            mode_2 = MODE_AUTO_BASIC;
            domeTarget=32;
            eyeTarget=eyeZero;
            break;
        } // switch key
        break;
    } // switch mode
  } // key processing

}


