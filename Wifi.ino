void clearWifi(){
    clearWifiConfig();
    WiFi.disconnect();
    Serial.println("WiFi config cleared");
}

// wifi:SSID,password 处理
bool onWifi(std::string ssid, std::string password){
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), password.c_str());
    Serial.printf("Try connect to SSID: %s with password: %s...\n", ssid.c_str(), password.c_str());

    int maxAttempts = 10; // 10s
    int attempt = 0;
    while (WiFi.status() != WL_CONNECTED && attempt < maxAttempts) {
        delay(1000);
        Serial.print(".");
        attempt++;
    }
    Serial.println("");
    if(WiFi.status() == WL_CONNECTED) {
        Serial.printf("WiFi connected localIP: %s\n", WiFi.localIP().toString().c_str());
        return true;
    } else {
        Serial.println("WiFi connection failed");
        return false;
    }
}
