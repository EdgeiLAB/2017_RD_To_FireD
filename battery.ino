// 배터리 측정
bool getBattery() {
  Serial.println("Before Battery");
  voltageBattery = battery.voltageLevel();
  voltagePercentage = int(battery.fuelLevel());
  Serial.println("After Battery");
  if(voltagePercentage >= 100)  voltagePercentage = 100;

  if(voltageBattery > LOW_BATTERY)  return true;
  else  return false;
}  



// 측정 값 시리얼 출력 함수
void printBattery() {
  Serial.print("Battery :");
  Serial.println(voltageBattery);
  Serial.print("Voltage : ");
  Serial.println(voltagePercentage);
}

