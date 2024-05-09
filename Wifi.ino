void clearWifi(){
    clearWifiConfig();
    WiFi.disconnect();
    Serial.println("WiFi config cleared");
}

// wifi:SSID,password 处理
bool onWifi(String ssid, String password){
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), password.c_str());
    Serial.println("Try connect to SSID: " + ssid + " with password: " + password + "...");

    int maxAttempts = 10; // 10s
    int attempt = 0;
    while (WiFi.status() != WL_CONNECTED && attempt < maxAttempts) {
        delay(1000);
        Serial.print(".");
        attempt++;
    }
    Serial.println("");
    if(WiFi.status() == WL_CONNECTED) {
        Serial.println("WiFi connected localIP: " + WiFi.localIP().toString());
        return true;
    } else {
        Serial.println("WiFi connection failed");
        return false;
    }
}
