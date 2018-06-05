bool getGeolocation() {
  if(isConnectingWifi()) {
    WifiLocation *location = new WifiLocation(googleApiKey);
    location_t buf = location->getGeoFromWiFi();
    latitude = buf.lat;
    longtitude = buf.lon;
    accuracy = buf.accuracy;
    delete(location);
      
    if(latitude <= 0.0 || longtitude <= 0.0) {
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
  Serial.println("Longitude: " + String(longtitude, 6));
  Serial.println("Accuracy: " + String(accuracy)); 
}



