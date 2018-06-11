void setupPin_led(void) {
  pinMode(pinLed, OUTPUT);
  digitalWrite(pinLed, LOW);
}


void blinkLed(){
  if (flagLed) {
    digitalWrite(pinLed, LOW);
    flagLed = false;
  } else {
    digitalWrite(pinLed, HIGH);
    flagLed = true;
  }
  timer0_write(ESP.getCycleCount() + 40000000L); // 80MHz == 1sec
}

