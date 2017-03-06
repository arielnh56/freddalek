void readEEPROM() {
  tmp_u8 = EEPROM.read(EEPROM_eyeMin);
  if (tmp_u8 < 128) {
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

}

