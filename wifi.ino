void connectingWifi() {
  Serial.println();
  Serial.println("------ WiFi Connecting ------");
  
  // Connect to WPA/WPA2 network
  WiFi.mode(WIFI_STA);  
  WiFi.begin(ssid, passwd);  
  
  // 연결 완료 까지 10초 대기 
  for(int i=0; i<10; i++) {
    if(WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.print(".");
    }
    else  break;
  }
//  Serial.print("\nConnected to ");
//  Serial.println(WiFi.SSID());
//  Serial.print("IP address: ");
//  Serial.println(WiFi.localIP());    
//  Serial.println("------ WiFi initialized ------");  
}

bool isConnectingWifi() {
  if(WiFi.status() != WL_CONNECTED) {
    return false;
  }
  return true;
}

