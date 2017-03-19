void(* resetFunc) (void) = 0; //declare reset function @ address 0

void checkKeyPad() {
  char key = keypad.getKey();
  if (key) {

#ifdef DEBUG
    Serial.print("key press ");
    Serial.println(key);
#endif
    DOME_MAXSPEED = 255;
    EYE_MAXSPEED = 255;
    switch (key) {
      case '0':
        mode = MODE_ZERO;
        mode_2 = MODE_ZERO_WAIT;
        lcdLine2 = (char *)nullmenu;
        eyeTarget = 0;
        break;
      case '*':
        mode = MODE_FOLLOW;
        mode_2 = MODE_FOLLOW_LEVEL;
        lcdLine2 = (char *)nullmenu;
        break;
      case '#':
        mode = MODE_AUTO;
        mode_2 = MODE_AUTO_BASIC;
        lcdLine2 = (char *)nullmenu;
        mode_auto_lastchange = millis();
        break;
    }

    lcd.setCursor(0, 0);

    switch (mode) {
      case MODE_ZERO:
        switch (mode_2) {
          case MODE_ZERO_WAIT:
            switch (key) {
              case '1':
                lcd.print(F("ZERO SET HEAD   "));
                lcdLine2 = (char *)F("2 SET UP LIMIT  8 SET DOWN LIMIT5 SET CENTER    ");
                mode_2 = MODE_ZERO_SET_HEAD;
                break;
              case '2':
                lcd.print(F("ZERO SET EYE    "));
                mode_2 = MODE_ZERO_SET_EYE;
                lcdLine2 = (char *)F("1 MOVE UP       7 MOVE DOWN     5 ZERO AND STOPS");
                break;
              case '3':
                lcd.print(F("ZERO SET DOME   "));
                mode_2 = MODE_ZERO_SET_DOME;
                lcdLine2 = (char *)F("4 MOVE LEFT     6 MOVE RIGHT    5 ZERO AND STOPS");
                break;
              case '4':
                lcd.print(F("EYE Kp          "));
                mode_2 = MODE_ZERO_SET_EYE_PID;
                mode_3 = MODE_PID_KP;
                lcdLine2 = (char *)F("1 SELECT Kp     2 SELECT Ki     3 SELECT Kd     4 SET           5 ZERO          6 RESTORE       ");
                handACE.setMpos((int16_t) (eyeKp * 100.0));
                break;
              case '5':
                lcd.print(F("DOME Kp         "));
                mode_2 = MODE_ZERO_SET_DOME_PID;
                mode_3 = MODE_PID_KP;
                lcdLine2 = (char *)F("1 SELECT Kp     2 SELECT Ki     3 SELECT Kd     4 SET           5 ZERO          6 RESTORE       ");
                handACE.setMpos((int16_t) (domeKp * 100.0));
                break;
              case '6':
                lcd.print(F("RESET           "));
                delay(1000);
                resetFunc();
                break;
              default:
                lcd.print(F("SELECT          "));
                lcdLine2 = (char *)F("1 SET HEAD      2 SET EYE       3 SET DOME      4 SET EYE PID   5 SET DOME PID  6 RESET         ");
                break;
            } // switch key
            break;
          case MODE_ZERO_SET_HEAD:
            chuck.update();
            switch (key) {
              case '2': // top limit
                headUp = round(chuck.readPitch10() / 10);
                EEPROM.write(EEPROM_headUp, headUp);
                break;
              case '8': // bottom limit
                headDown = round(chuck.readPitch10() / 10);
                EEPROM.write(EEPROM_headDown, headDown);
                break;
              case '5': // center
                headZeroV = round(chuck.readPitch10() / 10);
                EEPROM.write(EEPROM_headZeroV, headZeroV);
                headACE.setMpos(0);
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
              case '5': // set zero and find stops
                eyeACE.setMpos(0);
                eyeTarget = 0;
                digitalWrite(eyeM1, HIGH);
                digitalWrite(eyeM2, LOW);
                analogWrite(eyePWM, EYE_MINSPEED_UP + 10);
                for (tmp_u8 = 0; tmp_u8 < 50; tmp_u8++) {
                  delay(100);
                  eyePos = eyeACE.mpos(); // frequent checks to catch rollovers
                }
                digitalWrite(eyeM1, LOW);
                digitalWrite(eyeM2, LOW);
                analogWrite(eyePWM, 0);
                tmp_16 = eyeACE.mpos();
                eyeMax = tmp_16 - 1; // overrun
                EEPROM.put(EEPROM_eyeMax, eyeMax);
                digitalWrite(eyeM1, LOW);
                digitalWrite(eyeM2, HIGH);
                analogWrite(eyePWM, EYE_MINSPEED_DOWN + 8);
                for (tmp_u8 = 0; tmp_u8 < 80; tmp_u8++) {
                  delay(100);
                  eyePos = eyeACE.mpos(); // frequent checks to catch rollovers
                }
                digitalWrite(eyeM1, LOW);
                digitalWrite(eyeM2, LOW);
                analogWrite(eyePWM, 0);
                tmp_16 = eyeACE.mpos();
                eyeMin = tmp_16 + 2; // overrun
                EEPROM.put(EEPROM_eyeMin, eyeMin);
#ifdef DEBUG
                eyePos = eyeACE.mpos();
                Serial.print("eyePos ");
                Serial.println(eyePos);
                Serial.print("eyeTarget ");
                Serial.println(eyeTarget);
                Serial.print("eyeMax ");
                Serial.println(eyeMax);
                Serial.print("eyeMin ");
                Serial.println(eyeMin);

#endif
                break;
            } // switch key
            break;
          case MODE_ZERO_SET_DOME:
            switch (key) {
              case '4': // move left
                domeTarget--;
#ifdef DEBUG
                domePos = domeACE.pos();
                Serial.print("domePos ");
                Serial.println(domePos);
                Serial.print("domeTarget ");
                Serial.println(domeTarget);
#endif
                break;
              case '6': // move right
                domeTarget++;
#ifdef DEBUG
                domePos = domeACE.pos();
                Serial.print("domePos ");
                Serial.println(domePos);
                Serial.print("domeTarget ");
                Serial.println(domeTarget);
#endif
                break;
              case '5': // set zero and find stops
                domeACE.setMpos(0);
                domeTarget = 0;
                digitalWrite(domeM1, HIGH);
                digitalWrite(domeM2, LOW);
                analogWrite(domePWM, DOME_MINSPEED + 8);
                for (tmp_u8 = 0; tmp_u8 < 80; tmp_u8++) {
                  delay(100);
                  domePos = domeACE.pos(); // frequent checks to catch rollovers
                }
                digitalWrite(domeM1, LOW);
                digitalWrite(domeM2, LOW);
                analogWrite(domePWM, 0);
                tmp_16 = domeACE.pos();
                domeMax = tmp_16 - 1; // overrun
                EEPROM.put(EEPROM_domeMax, domeMax);
                digitalWrite(domeM1, LOW);
                digitalWrite(domeM2, HIGH);
                analogWrite(domePWM, DOME_MINSPEED + 8);
                for (tmp_u8 = 0; tmp_u8 < 80; tmp_u8++) {
                  delay(100);
                  domePos = domeACE.pos(); // frequent checks to catch rollovers
                }
                digitalWrite(domeM1, LOW);
                digitalWrite(domeM2, LOW);
                analogWrite(domePWM, 0);
                tmp_16 = domeACE.pos();
                domeMin = tmp_16 + 2; // overrun
                EEPROM.put(EEPROM_domeMin, domeMin);
#ifdef DEBUG
                domePos = domeACE.pos();
                Serial.print("domePos ");
                Serial.println(domePos);
                Serial.print("domeTarget ");
                Serial.println(domeTarget);
                Serial.print("domeMax ");
                Serial.println(domeMax);
                Serial.print("domeMin ");
                Serial.println(domeMin);

#endif
                break;

            } // switch key
            break;
          case MODE_ZERO_SET_EYE_PID:
            switch (key) {
              case '1': // select Kp
                Serial.print("eyeKp "); Serial.println(eyeKp);
                mode_3 = MODE_PID_KP;
                handACE.setMpos((int16_t) (eyeKp * 100.0));
                lcd.setCursor(5, 0);
                lcd.print("p");
                break;
              case '2': // select Ki
                mode_3 = MODE_PID_KI;
                handACE.setMpos((int16_t) (eyeKi * 100.0));
                lcd.setCursor(5, 0);
                lcd.print("i");
                break;
              case '3': // select Kd
                mode_3 = MODE_PID_KD;
                handACE.setMpos((int16_t) (eyeKd * 100.0));
                lcd.setCursor(5, 0);
                lcd.print("d");
                break;
              case '4': // set K?
                if (mode_3 == MODE_PID_KP) {
                  eyeKpS = (int16_t) (eyeKp * 100.0);
                  EEPROM.put(EEPROM_eyeKp, eyeKpS);
                } else if (mode_3 == MODE_PID_KI) {
                  eyeKiS = (int16_t) (eyeKi * 100.0);
                  EEPROM.put(EEPROM_eyeKi, eyeKiS);
                } else if (mode_3 == MODE_PID_KD) {
                  eyeKdS = (int16_t) (eyeKd * 100.0);
                  EEPROM.put(EEPROM_eyeKd, eyeKdS);
                }
                break;
              case '5': // zero K?
                handACE.setMpos(0);
                if (mode_3 == MODE_PID_KP) {
                  eyeKp = 0;
                } else if (mode_3 == MODE_PID_KI) {
                  eyeKi = 0;
                } else if (mode_3 == MODE_PID_KD) {
                  eyeKd = 0;
                }
                break;
              case '6': // restore K?
                if (mode_3 == MODE_PID_KP) {
                  eyeKp = eyeKpS / 100;
                  handACE.setMpos(eyeKpS);
                } else if (mode_3 == MODE_PID_KI) {
                  eyeKi = eyeKiS / 100;
                  handACE.setMpos(eyeKiS);
                } else if (mode_3 == MODE_PID_KD) {
                  eyeKd = eyeKdS / 100;
                  handACE.setMpos(eyeKdS);
                }
                break;
            } // switch key
            break;
          case MODE_ZERO_SET_DOME_PID:
            switch (key) {
              case '1': // select Kp
                Serial.print("domeKp "); Serial.println(domeKp);
                mode_3 = MODE_PID_KP;
                handACE.setMpos((int16_t) (domeKp * 100.0));
                lcd.setCursor(6, 0);
                lcd.print("p");
                break;
              case '2': // select Ki
                mode_3 = MODE_PID_KI;
                handACE.setMpos((int16_t) (domeKi * 100.0));
                lcd.setCursor(6, 0);
                lcd.print("i");
                break;
              case '3': // select Kd
                mode_3 = MODE_PID_KD;
                handACE.setMpos((int16_t) (domeKd * 100.0));
                lcd.setCursor(6, 0);
                lcd.print("d");
                break;
              case '4': // set K?
                if (mode_3 == MODE_PID_KP) {
                  domeKpS = (int16_t) (domeKp * 100.0);
                  EEPROM.put(EEPROM_domeKp, domeKpS);
                } else if (mode_3 == MODE_PID_KI) {
                  domeKiS = (int16_t) (domeKi * 100.0);
                  EEPROM.put(EEPROM_domeKi, domeKiS);
                } else if (mode_3 == MODE_PID_KD) {
                  domeKdS = (int16_t) (domeKd * 100.0);
                  EEPROM.put(EEPROM_domeKd, domeKdS);
                }
                break;
              case '5': // zero K?
                handACE.setMpos(0);
                if (mode_3 == MODE_PID_KP) {
                  domeKp = 0;
                } else if (mode_3 == MODE_PID_KI) {
                  domeKi = 0;
                } else if (mode_3 == MODE_PID_KD) {
                  domeKd = 0;
                }
                break;
              case '6': // restore K?
                if (mode_3 == MODE_PID_KP) {
                  domeKp = domeKpS / 100;
                  handACE.setMpos(domeKpS);
                } else if (mode_3 == MODE_PID_KI) {
                  domeKi = domeKiS / 100;
                  handACE.setMpos(domeKiS);
                } else if (mode_3 == MODE_PID_KD) {
                  domeKd = domeKdS / 100;
                  handACE.setMpos(domeKdS);
                }
                break;
            } // switch key
            break;
        } // switch mode_2
        break;
      case MODE_FOLLOW:
        lcd.print("TRACK ");
        switch (key) {
          case '1':
            lcd.print("NORMAL    ");
            mode_2 = MODE_FOLLOW_NORMAL;
            break;
          case '2':
            mode_2 = MODE_FOLLOW_LEVEL;
            lcd.print("LEVEL     ");
            eyeTarget = 0;
            break;
          default:
            lcd.print("LEVEL    ");
            lcdLine2 = (char *)F("1 NORMAL        2 LEVEL         ");
            break;
        } // switch key
        break;
      case MODE_AUTO:
        lcd.print("AUTO ");
        switch (key) {
          case '1':
            lcd.print("SCAN RIGHT ");
            mode_2 = MODE_AUTO_SCAN;
            mode_3 = MODE_AUTO_SCAN_RIGHT;
            DOME_MAXSPEED = DOME_MINSPEED + 15;
            break;
          case '4':
            lcd.print("LOOK LEFT  ");
            mode_2 = MODE_AUTO_BASIC;
            domeTarget = -32;
            eyeTarget = 0;
            break;
          case '5':
            lcd.print("LOOK FRONT ");
            mode_2 = MODE_AUTO_BASIC;
            domeTarget = 0;
            eyeTarget = 0;
            break;
          case '6':
            lcd.print("LOOK RIGHT ");
            mode_2 = MODE_AUTO_BASIC;
            domeTarget = 32;
            eyeTarget = 0;
            break;
          default:
            lcd.print("SELECT     ");
            lcdLine2 = (char *)F("1 SCAN RIGHT    4 LOOK LEFT     5 LOOK FRONT    6 LOOK RIGHT    ");
            break;
        } // switch key
        break;
    } // switch mode
  } // key processing
}
