#include <Arduino.h>
#include <MAX17043GU.h>
#include <SSD1306Wire.h>
#include <Wire.h>
#include <WifiLocation.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include "thingplug.h"

#define LOW_BATTERY   3.7
#define SMOKE_DETECT_CONDITION   30

// 5 octave - Do, Re, Mi, Fa, So, La, Ti, 4 octave - Do
const int aFiveOctave[8] = {523, 587, 659, 698, 784, 880, 987, 1046};

// 핀 선언 및 정의
const int pinAdc = A0;
const int pinLed = 16;
const int pinPiezo = 15;  

// 공유기, 구글 API, ThingPlug 엑세스 요구사항
const char* googleApiKey = "AIzaSyBjBymHPAl222oj_9JpM54bUZPmgxQr0Q4";
const char* ssid = "yebin";
const char* passwd = "thdwl15928";
const char *addr = "mqtt.sktiot.com";
const char *id = "edgeilab";
const char *pw = "ZEwxMW9DZmNQK3dudWdRcTV4bVhEK1ByK3U2amtxU3NCWjE0OERNREI3QkUwdCtsSmhZWDQ4eGRURkd0NVFIUw==";

// 변경하기!!!!!!!!!!!!!!!!!!!!!!!!!!!
const char *deviceId = "edgeilab_FireD_10";
///////////////////////////////////////////////////////
const char *devicePw = "123456";
const char containerSmoke[] = "Smoke";
const char containerGeolocation_latitude[] = "Geolocation_latitude";
const char containerGeolocation_longitude[] = "Geolocation_longitude";



// 객체 생성
SSD1306Wire display(0x3c, 4, 5);    // SDA, SCL
MAX17043GU battery;
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);


// 측정 값 저장 변수
float voltageBattery = 0.0;
int voltagePercentage = 0.0;
int smokeAdcData = 0;

// 위 경도 값 저장 변수
float latitude = 0.0;
float longitude = 0.0;
float accuracy = 0.0;

// 상태 확인 변수
bool flagLed = false;
bool flagWarning = false;
bool flagUploadData = false;
bool flagCharge = false;

// 시간 측정 변수
int countSecond = 0;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void setup() {
  Serial.begin(115200);  

  // 타이머  인터럽트 초기화
  timer0_isr_init();
  
  // 초기화 하는 동안 LED Blink
  timer0_attachInterrupt(blinkLed);

  // I2C 초기화
  Wire.begin();
  battery.restart();
  display.init();     
  display.clear();
  
  // Pin Setup
  setupPin_led();
  setupPin_piezo();


  Serial.println("SETUP START");
  displayCenter(ArialMT_Plain_10, "SETUP START");  
  delay(2000);

  // 배터리 측정 --------------------------------------------
  display.clear();  
  displayCenter(ArialMT_Plain_10, "BATTERY MEASURING");
  display.display();
  while(!getBattery()) {
    // 실패
    Serial.println("Please charge the the battery");    
    delay(1000);
  }
  // 성공
  Serial.println("Succeed to measure the battery");
  delay(2000);  
  // --------------------------------------------------------



  // WiFi 연결 ----------------------------------------------
  display.clear();  
  displayCenter(ArialMT_Plain_10, "WiFi CONNECTING");
  display.display();    
  while(!isConnectingWifi()) {
    Serial.println("Fail to connect WiFi");
    Serial.println("Try Again");    
    delay(1000);
    connectingWifi();    
  }
  // 성공
  Serial.println("Succeed to connect WiFI");
  delay(2000);  
  // --------------------------------------------------------



  // ThingPlug 연결 -----------------------------------------
  display.clear();    
  displayCenter(ArialMT_Plain_10, "THINGPLUG CONNECTING");
  display.display();
  while(!connectingThingplug()) {
    Serial.println("Fail to connect ThingPlug");
    delay(1000);    
  }
  // 성공
  Serial.println("Succeed to connect ThingPlug");  
  delay(2000);  
  // --------------------------------------------------------



  // 위경도 측정 --------------------------------------------
  display.clear();  
  displayCenter(ArialMT_Plain_10, "GEOLOCATION MEASURING"); 
  display.display();
  while(!getGeolocation()) {
    // 실패
    Serial.println("Fail to measure Geolocation");
    delay(1000);
  }
  // 성공
  Serial.println("Succeed to measure Geolocation");  
  delay(2000);
  // -------------------------------------------------------


  // 초기화 성공시 Piezo로 소리 출력, LED Blink Off ----------
  startPiezo();
  // LED Blink 정지
  timer0_detachInterrupt();
  display.clear();    
  displayCenter(ArialMT_Plain_10, "SETUP SUCCESS");  
  display.display();
  // ---------------------------------------------------------
  delay(2000);

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void loop() {
  
  mqttClient.loop();

  smokeAdcData = (analogRead(pinAdc)/1023.0)*100;

  countSecond++;
  delay(100);
  

  // 화재 감지기 이상 감지할 경우
  if(smokeAdcData >= SMOKE_DETECT_CONDITION)  flagWarning = true;
  else  flagWarning = false;

  // 각 상황에 맞게 디스플레이
  
  // 화재 감지할 경우, 경고음 출력, LED 점등
  if(flagWarning) {
    timer0_attachInterrupt(warningPiezo);
    digitalWrite(pinLed, HIGH);
    display.clear();
    displayCenter(ArialMT_Plain_24, "WARNING");
    displayBattery();
    displayAdc();
    display.display();
    
    
    // ThingPlug 업로드 Once 
    if(!flagUploadData) {
      flagUploadData = true;
      if(flagWarning) mqttPublish_UploadData("FIRE_START");
    }
  }
  // 방전할 경우, LED 점등
  else if(flagCharge) {
    // 방전시  
    timer0_attachInterrupt(blinkLed);      

    display.clear();
    displayCenter(ArialMT_Plain_10, "NEED TO CHARGE");    
    display.display();    
  }
  else  {
    timer0_detachInterrupt();
    
    digitalWrite(pinLed, LOW);    
    noTone(pinPiezo);
    // 디스플레이
    display.clear();
    displayCenter(ArialMT_Plain_24, "NORMAL");
    displayBattery();
    displayAdc();
    display.display();     
     
    if(flagUploadData) {
      flagUploadData = false;
      mqttPublish_UploadData("STOP");  
    }    
  }




  // 1초마다 프린트
  if(countSecond%10 == 0) {
    Serial.println("Count : " + String(countSecond/10));
  }
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

  // 30초마다 Battery 측정
  if(countSecond%300 == 0 ) {
    if(getBattery()) {
      flagCharge = false;
    }
    else {
      flagCharge = true;     
      printBattery();
    }
  }


  // 1800초마다 위경도 측정
  if((countSecond%18000 == 0) &&(flagWarning == false) ) {
    if(getGeolocation()) {
      printLocation();
      mqttPublish_Geolocation();
    }
  }

}
