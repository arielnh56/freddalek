void readEEPROM() {
  EEPROM.get(EEPROM_eyeMin, eyeMin);
  EEPROM.get(EEPROM_eyeMax, eyeMax);
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
  EEPROM.get(EEPROM_domeMin, domeMin);
  EEPROM.get(EEPROM_domeMax, domeMax);

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

  EEPROM.get(EEPROM_domeKp, domeKpS);
  EEPROM.get(EEPROM_domeKi, domeKiS);
  EEPROM.get(EEPROM_domeKd, domeKdS);
  EEPROM.get(EEPROM_eyeKp, eyeKpS);
  EEPROM.get(EEPROM_eyeKi, eyeKiS);
  EEPROM.get(EEPROM_eyeKd, eyeKdS);
#ifdef DEBUG
  Serial.print("eyeKpS "); Serial.println(eyeKpS);
  Serial.print("eyeKiS "); Serial.println(eyeKiS);
  Serial.print("eyeKdS "); Serial.println(eyeKdS);
#endif
}

