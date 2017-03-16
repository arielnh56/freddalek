
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
        lcd.print("ZERO ");
        switch (mode_2) {
          case MODE_ZERO_WAIT:
            switch (key) {
              case '1':
                lcd.print("SET HEAD    ");
                lcdLine2 = (char *)F("2 SET UP LIMIT  8 SET DOWN LIMIT5 SET CENTER    ");
                mode_2 = MODE_ZERO_SET_HEAD;
                break;
              case '2':
                lcd.print("SET EYE     ");
                mode_2 = MODE_ZERO_SET_EYE;
                lcdLine2 = (char *)F("1 MOVE UP       7 MOVE DOWN     5 ZERO AND STOPS");
                break;
              case '3':
                lcd.print("SET DOME    ");
                mode_2 = MODE_ZERO_SET_DOME;
                lcdLine2 = (char *)F("4 MOVE LEFT     6 MOVE RIGHT    5 ZERO AND STOPS");
                break;
              default:
                lcd.print("SELECT      ");
                lcdLine2 = (char *)F("1 SET HEAD      2 SET EYE       3 SET DOME      ");
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
