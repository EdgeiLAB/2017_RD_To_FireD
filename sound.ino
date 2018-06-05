void setupPin_piezo(void) {
  pinMode(pinPiezo, OUTPUT);
  digitalWrite(pinPiezo, LOW);
}

void startPiezo(void){
  for(int i=0; i<8; i++) {
    tone(pinPiezo, aFiveOctave[i]);
    delay(100);
    noTone(pinPiezo);
  }
}

void warningPiezo(void) {
  for(int i=0; i<5; i++) {
    tone(pinPiezo, 1046);
    timer0_write(ESP.getCycleCount() + 16000000L); // 80MHz == 1sec
    tone(pinPiezo, 784);
    timer0_write(ESP.getCycleCount() + 16000000L); // 80MHz == 1sec
  }
}

