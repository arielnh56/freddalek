#include "config.h"

#include <EEPROM.h>
#include <math.h>

#define LOOPTIME 25
// corresponding vars
int16_t eyeMax, eyeMin, domeMax, domeMin;
uint8_t headUp, headDown, headZeroV; // angle 90ish = horizontal

// current positions
int16_t domePos;
int8_t headPos;
uint8_t domeSpeed;
uint8_t targetDomeSpeed;
int16_t eyePos;
uint8_t eyeSpeed;
uint8_t targetEyeSpeed;
int8_t eyeDir = MOVE_STOP;
int8_t domeDir = MOVE_STOP;
uint8_t EYE_MAXSPEED = 255;
uint8_t DOME_MAXSPEED = 255;

// global temps for scratch use
uint8_t tmp_u8;
int8_t  tmp_8;
int16_t tmp_16;

byte mode;
byte mode_2;
byte mode_3;
uint32_t MODE_AUTO_SCAN_TIMER = 0;
uint32_t mode_auto_lastchange;


int16_t eyeTarget;
uint32_t eyeLastChange;
int16_t domeTarget;
uint32_t domeLastChange;
uint32_t lastLoop;
uint32_t lastLcdLine2;
char lcdbuf[17];

const char PROGMEM nullmenu[] = "                ";
char* lcdLine2 = (char *)nullmenu;

void loop() {
  if (lastLoop + LOOPTIME > millis()) {
    delay(lastLoop + LOOPTIME - millis());
  }
  lastLoop = millis();

  if (lastLcdLine2 + 1000 < millis()) {
    lastLcdLine2 = millis();
    int8_t numMenus = strlen_P(lcdLine2)/16;
    int8_t offset = ((int)(lastLcdLine2/1000) % numMenus) * 16;
    strncpy_P(lcdbuf,lcdLine2+offset,16);
    lcd.setCursor(0,1);
    lcd.print(lcdbuf);
   }
  chuck.update();
  int chuckPitch = chuck.readPitch10();
  eyePos  = eyeACE.mpos();
  domePos = domeACE.mpos();
  headPos = headACE.mpos();

  if (chuckPitch > headZeroV * 10 + 15) {
    digitalWrite(LED_HEAD_RED, LOW);
  } else {
    digitalWrite(LED_HEAD_RED, HIGH);
  }
  if (abs(chuckPitch - headZeroV * 10) < 30) {
    digitalWrite(LED_HEAD_YELLOW, LOW);
  } else {
    digitalWrite(LED_HEAD_YELLOW, HIGH);
  }
  if (chuckPitch < headZeroV * 10 - 15) {
    digitalWrite(LED_HEAD_GREEN, LOW);
  } else {
    digitalWrite(LED_HEAD_GREEN, HIGH);
  }
  // motion decisions
  switch (mode) {
    case MODE_FOLLOW:
      switch (mode_2) {
        case MODE_FOLLOW_NORMAL:
          tmp_16 = chuckPitch - headZeroV * 10;
          if (abs(tmp_16) < 50) {
            eyeTarget = 0;
          } else if (tmp_16 > 0) { // up
            eyeTarget = constrain((tmp_16 - 50) / 10, 0, eyeMax);
          } else { // down
            eyeTarget = constrain((tmp_16 + 50) / 10, eyeMin, 0);
          }
          domeTarget = headPos;
          break;

        case MODE_FOLLOW_LEVEL:
          eyeTarget = 0;
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
  //  domePos = domeACE.mpos();
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
  /*Serial.print(millis());
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
  */
#endif

  // motor control
  // target speed same dir
  // same dir?
  if (((eyeTarget > eyePos) && eyeDir == MOVE_UP)) {
    targetEyeSpeed = constrain(EYE_MINSPEED_UP - 5 + (abs(eyeTarget - eyePos) * 5), EYE_MINSPEED_UP, EYE_MAXSPEED);
    if (eyeSpeed < targetEyeSpeed) { // speed up
      eyeSpeed = constrain(eyeSpeed + eyeUpAccel, EYE_MINSPEED_UP, EYE_MAXSPEED);
    } else if (eyeSpeed > targetEyeSpeed) { // slow down
      eyeSpeed = constrain(eyeSpeed - eyeDownAccel, EYE_MINSPEED_UP, EYE_MAXSPEED);
    }
  } else if ((eyeTarget < eyePos) && eyeDir == MOVE_DOWN) {
    targetEyeSpeed = constrain(EYE_MINSPEED_DOWN - 5 + (abs(eyeTarget - eyePos) * 5), EYE_MINSPEED_DOWN, EYE_MAXSPEED);
    if (eyeSpeed < targetEyeSpeed) { // speed up
      eyeSpeed = constrain(eyeSpeed + eyeDownAccel, EYE_MINSPEED_UP, EYE_MAXSPEED);
    } else if (eyeSpeed > targetEyeSpeed) { // slow down
      eyeSpeed = constrain(eyeSpeed - eyeUpAccel, EYE_MINSPEED_UP, EYE_MAXSPEED);
    }
  } else if ((eyeTarget > eyePos) && eyeDir == MOVE_DOWN) { // direction change
    if (eyeSpeed > EYE_MINSPEED_DOWN) eyeSpeed = constrain(eyeSpeed - eyeUpAccel, EYE_MINSPEED_DOWN, EYE_MAXSPEED); // slow down
    if (eyeSpeed <= EYE_MINSPEED_DOWN) eyeDir = MOVE_UP;
  } else if ((eyeTarget < eyePos) && eyeDir == MOVE_UP) { // direction change
    if (eyeSpeed > EYE_MINSPEED_UP) eyeSpeed = constrain(eyeSpeed - eyeDownAccel, EYE_MINSPEED_UP, EYE_MAXSPEED); // slow down
    if (eyeSpeed <= EYE_MINSPEED_UP) eyeDir = MOVE_DOWN;
  } else if (eyeTarget == eyePos) { // stop
    eyeDir = MOVE_STOP;
    eyeSpeed = 0;
  } else if (eyePos < eyeTarget + 2 && eyeDir == MOVE_STOP) {
    eyeDir = MOVE_UP;
    eyeSpeed = EYE_MINSPEED_UP;
  } else if (eyePos > eyeTarget - 2 && eyeDir == MOVE_STOP) {
    eyeDir = MOVE_DOWN;
    eyeSpeed = EYE_MINSPEED_DOWN;
  }

  // same dir?
  if (((domeTarget < domePos) && domeDir == MOVE_LEFT) || ((domeTarget > domePos) && domeDir == MOVE_RIGHT)) {
    targetDomeSpeed = constrain(DOME_MINSPEED - 3 + (abs(domeTarget - domePos) * 3), DOME_MINSPEED, DOME_MAXSPEED);
    if (domeSpeed < targetDomeSpeed) { // speed up
      domeSpeed = constrain(domeSpeed + domeAccel, DOME_MINSPEED, DOME_MAXSPEED);
    } else if (domeSpeed > targetDomeSpeed) { // slow down
      domeSpeed = constrain(domeSpeed - domeDecel, DOME_MINSPEED, DOME_MAXSPEED);
    }
  } else if (((domeTarget > domePos) && domeDir == MOVE_LEFT) || ((domeTarget < domePos) && domeDir == MOVE_RIGHT)) {
    if (domeSpeed > DOME_MINSPEED) domeSpeed = constrain(domeSpeed - domeDecel, DOME_MINSPEED, DOME_MAXSPEED); // slow down
    if (domeSpeed <= DOME_MINSPEED) domeDir *= -1;
  } else if (domeTarget == domePos) { // stop
    domeDir = MOVE_STOP;
    domeSpeed = 0;
  } else if (domePos < domeTarget && domeDir == MOVE_STOP) {
    domeDir = MOVE_RIGHT;
    domeSpeed = DOME_MINSPEED;
  } else if (domePos > domeTarget && domeDir == MOVE_STOP) {
    domeDir = MOVE_LEFT;
    domeSpeed = DOME_MINSPEED;
  }


  if (domeDir == MOVE_STOP) { // brake
    digitalWrite(domeM1, LOW);
    digitalWrite(domeM2, LOW);
    analogWrite(domePWM, 0);
  } else if (domeDir == MOVE_LEFT) {
    digitalWrite(domeM1, LOW);
    digitalWrite(domeM2, HIGH);
    analogWrite(domePWM, domeSpeed);
  } else if (domeDir == MOVE_RIGHT) {
    digitalWrite(domeM1, HIGH);
    digitalWrite(domeM2, LOW);
    analogWrite(domePWM, domeSpeed);
  }

  if (eyeDir == MOVE_STOP) { // brake
    digitalWrite(eyeM1, LOW);
    digitalWrite(eyeM2, LOW);
    analogWrite(eyePWM, 0);
  } else if (eyeDir == MOVE_UP) {
    digitalWrite(eyeM1, HIGH);
    digitalWrite(eyeM2, LOW);
    analogWrite(eyePWM, eyeSpeed);
  } else if (eyeDir == MOVE_DOWN) {
    digitalWrite(eyeM1, LOW);
    digitalWrite(eyeM2, HIGH);
    analogWrite(eyePWM, eyeSpeed);
  }
  checkKeyPad();
}


