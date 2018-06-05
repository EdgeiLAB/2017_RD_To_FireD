void displayCenter(const uint8_t *textSize, String msg) {
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setFont(textSize);    
  if(msg.length() > 20) {
    char separator = ' ';
    int index = msg.indexOf(separator);
    String tmp1 = msg.substring(0, index);
    String tmp2 = msg.substring(index+1);
    
    display.drawString(64, 22, tmp1);
    display.drawString(64, 33, tmp2);
  }
  else {
    display.drawString(64, 22, msg);
  }
}

void displayBattery() {
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);    
  display.drawString(0, 0, "B : " + String(voltagePercentage));
}
