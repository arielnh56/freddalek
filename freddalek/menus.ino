
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
        lcdLine2 = nullmenu;
        eyeTarget = eyeZero;
        break;
      case '*':
        mode = MODE_FOLLOW;
        mode_2 = MODE_FOLLOW_LEVEL;
        lcdLine2 = nullmenu;
        break;
      case '#':
        mode = MODE_AUTO;
        mode_2 = MODE_AUTO_BASIC;
        lcdLine2 = nullmenu;
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
                lcdLine2 = (char *)F("1 MOVE UP       7 MOVE DOWN     2 FIND UP STOP  8 FIND DOWN STOP5 SET ZERO      ");
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
              case '4': // left limit
                break;
              case '6': // right limit
                break;
              case '5': // center
                headZeroV = round(chuck.readPitch10() / 10);
                EEPROM.write(EEPROM_headZeroV, headZeroV);
                headZeroH = headACE.rawPos();
                headACE.setZero(headZeroH);
                EEPROM.write(EEPROM_headZeroH, headZeroH);
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
                digitalWrite(eyeM1, HIGH);
                digitalWrite(eyeM2, LOW);
                analogWrite(eyePWM, EYE_MINSPEED_UP);
                delay(20000);
                digitalWrite(eyeM1, LOW);
                digitalWrite(eyeM2, LOW);
                analogWrite(eyePWM, 0);
                tmp_u8 = eyeACE.upos();
                eyeMax = tmp_u8 + 2; // overrun
                if (eyeMax > 127) eyeMax = eyeMax - 128; // roll
                EEPROM.write(EEPROM_eyeMax, eyeMax);
                eyeTarget = eyeMax;
                break;
              case '8': // set min - find lower stop
                digitalWrite(eyeM1, LOW);
                digitalWrite(eyeM2, HIGH);
                analogWrite(eyePWM, EYE_MINSPEED_DOWN);
                delay(10000);
                digitalWrite(eyeM1, LOW);
                digitalWrite(eyeM2, LOW);
                analogWrite(eyePWM, 0);
                tmp_u8 = eyeACE.rawPos();
                eyeMin = tmp_u8 - 2; // overrun
                if (eyeMin > 127) eyeMin = eyeMin - 128; // roll
                EEPROM.write(EEPROM_eyeMin, eyeMin);
                eyeACE.setZero(eyeMin);
                eyeTarget = 0;
                break;
              case '5': // set zero
                eyeZero = constrain(eyeACE.upos(), 1, 127);
                EEPROM.write(EEPROM_eyeZero, eyeZero);
                break;
            } // switch key
            break;
          case MODE_ZERO_SET_DOME:
            switch (key) {
              case '4': // move left
                domeACE.setZero(domeACE.rawPos());
                domeTarget = -1;
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
                domeTarget = 1;
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
                EEPROM.write(EEPROM_domeZero, tmp_u8);
                domeACE.setZero(tmp_u8);
                domeTarget = 0;
                digitalWrite(domeM1, HIGH);
                digitalWrite(domeM2, LOW);
                analogWrite(domePWM, 190);
                delay(10000);
                digitalWrite(domeM1, LOW);
                digitalWrite(domeM2, LOW);
                analogWrite(domePWM, 0);
                tmp_8 = domeACE.pos();
                domeMax = tmp_8 - 1; // overrun
                EEPROM.write(EEPROM_domeMax, domeMax);
                digitalWrite(domeM1, LOW);
                digitalWrite(domeM2, HIGH);
                analogWrite(domePWM, 190);
                delay(15000);
                digitalWrite(domeM1, LOW);
                digitalWrite(domeM2, LOW);
                analogWrite(domePWM, 0);
                tmp_8 = domeACE.pos();
                domeMin = tmp_8 + 1; // overrun
                EEPROM.write(EEPROM_domeMin, domeMin);
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
        lcd.print("TRACK ");
        switch (key) {
          case '1':
            lcd.print("NORMAL    ");
            mode_2 = MODE_FOLLOW_NORMAL;
            break;
          case '2':
            mode_2 = MODE_FOLLOW_LEVEL;
            lcd.print("LEVEL     ");
            eyeTarget = eyeZero;
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
            eyeTarget = eyeZero;
            break;
          case '5':
            lcd.print("LOOK FRONT ");
            mode_2 = MODE_AUTO_BASIC;
            domeTarget = 0;
            eyeTarget = eyeZero;
            break;
          case '6':
            lcd.print("LOOK RIGHT ");
            mode_2 = MODE_AUTO_BASIC;
            domeTarget = 32;
            eyeTarget = eyeZero;
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
