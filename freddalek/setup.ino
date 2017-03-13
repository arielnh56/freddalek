void setup() {
  // fast PWM
  TCCR1B = (TCCR1B & 0b11111000) | 0x01;
#ifdef DEBUG
  Serial.begin(115200);
#endif
  chuck.begin();
  chuck.update();
  keypad.begin();

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

  readEEPROM();
  mode = MODE_AUTO;
  mode_2 = MODE_AUTO_BASIC;
  eyeTarget = 0;
  eyeLastChange = millis();
  domeTarget = -32; // lookleft - we may be open
  domeLastChange = millis();

  pinMode(LED_HEAD_RED, OUTPUT);
  pinMode(LED_HEAD_YELLOW, OUTPUT);
  pinMode(LED_HEAD_GREEN, OUTPUT);
  digitalWrite(LED_HEAD_RED, HIGH);
  digitalWrite(LED_HEAD_YELLOW, HIGH);
  digitalWrite(LED_HEAD_GREEN, HIGH);

  lastLoop = millis();

  lcd.begin(16, 2);
  lcd.setBacklight(255);
  lcd.home(); lcd.clear();
  lcd.print("  EXTERMINATE");
  lcd.setCursor(0, 1);
  lcd.print("  ALL  HUMANS");
  delay(2000);
  lastLcdLine2 = millis();
  lcd.clear();
  lcd.print("ZERO GOTO_ZERO");

}

