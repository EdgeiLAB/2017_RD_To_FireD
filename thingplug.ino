bool connectingThingplug() {
  if(!mqttConnect(&mqttClient, addr, id, pw, deviceId)) {
    printf("1. mqtt connect failed\n");
    return false;
  }

  if(!mqttCreateNode(&mqttClient, devicePw)) {
    printf("2. mqtt create node failed\n");
    return false;    
  }

  if(!mqttCreateRemoteCSE(&mqttClient)) {
    printf("3. mqtt create remote cse failed\n");
    return false;    
  }  

  if(!mqttCreateContainer(&mqttClient, containerSmoke)) {
    printf("4. mqtt create container failed\n");
    return false;
  }

  if(!mqttCreateContainer(&mqttClient, containerGeolocation_latitude)) {
    printf("4. mqtt create container failed\n");
    return false;
  }

  if(!mqttCreateContainer(&mqttClient, containerGeolocation_longitude)) {
    printf("4. mqtt create container failed\n");
    return false;
  }
  return true;
}


void mqttPublish_Geolocation() {
  char strLatitude[BUF_SIZE_SMALL];
  char strLongitude[BUF_SIZE_SMALL];  
    sprintf(strLatitude, "%f", latitude);
    sprintf(strLongitude, "%f", longitude);  
    
  if(isConnectingWifi()) {
    mqttCreateContentInstance(&mqttClient, containerGeolocation_latitude, strLatitude);
    mqttCreateContentInstance(&mqttClient, containerGeolocation_longitude, strLongitude);
  }  
  else {
    connectingWifi();
    connectingThingplug(); 
    mqttCreateContentInstance(&mqttClient, containerGeolocation_latitude, strLatitude);
    mqttCreateContentInstance(&mqttClient, containerGeolocation_longitude, strLongitude);    
  }

}

void mqttPublish_UploadData(char* str) {
  if(isConnectingWifi()) {
    mqttCreateContentInstance(&mqttClient, containerSmoke, str);    
  }
  else {
    connectingWifi();
    connectingThingplug();
    mqttCreateContentInstance(&mqttClient, containerSmoke, str);        
  }
}
