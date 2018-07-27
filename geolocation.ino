bool getGeolocation() {
  if(isConnectingWifi()) {
    Serial.println("Location Measuring");
    
    WifiLocation *location = new WifiLocation(googleApiKey);
    location_t *buf = new location_t(location->getGeoFromWiFi()); 
    latitude = buf->lat;
    longitude = buf->lon;
    accuracy = buf->accuracy;
    
    delete(location);
    delete(buf);
    buf = NULL;
    location = NULL;   

    if(latitude <= 0.0 || longitude <= 0.0) {
      Serial.println("Fail to measure Location");
      return false;
    }
      
    return true;
 
  }
  else
    return false;
}

void printLocation() {
  Serial.println("Latitude: " + String(latitude, 6));
  Serial.println("Longitude: " + String(longitude, 6));
  Serial.println("Accuracy: " + String(accuracy)); 
}



