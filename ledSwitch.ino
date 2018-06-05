void setupPin_ledSwitch(void) {
  pinMode(pinLed, OUTPUT);
  pinMode(pinSwitch, INPUT);
  digitalWrite(pinLed, LOW);
}

// 외부 인터럽트 발생 시 호출 함수
void handleInterrupt() {
  flagSwitch = !flagSwitch;
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

