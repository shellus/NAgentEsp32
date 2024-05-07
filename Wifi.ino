// todo wifi配置改为从config获取和设置
void clearWifi(){
    // 删除 WiFi 配置 json
    Serial.println("Clear WiFi config from flash");
    SPIFFS.remove("/wifi.json");
    Serial.println("WiFi config cleared");
    // 断开当前WIFI
    WiFi.disconnect();
}
void saveWifiConfig(String ssid, String password){
    // 储存 WiFi 配置 json 到 flash
    Serial.println("Save WiFi config to flash");
    DynamicJsonDocument doc(1024);
    doc["ssid"] = ssid;
    doc["password"] = password;
    File configFile = SPIFFS.open("/wifi.json", "w");
    if (!configFile) {
        Serial.println("Failed to open wifi.json for writing");
    }
    serializeJson(doc, configFile);
    configFile.close();
    Serial.println("WiFi config saved");
}

bool loadWifiConfig(String &ssid, String &password){
    // 读取 WiFi 配置 json
    Serial.println("Load WiFi config from flash");
    File configFile = SPIFFS.open("/wifi.json", "r");
    if (!configFile) {
        Serial.println("Failed to open wifi.json for reading");
        return false;
    }
    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, configFile);
    if (error) {
        Serial.println("Failed to read wifi.json");
        return false;
    }
    configFile.close();
    ssid = doc["ssid"].as<String>();
    password = doc["password"].as<String>();
    Serial.println("WiFi config loaded SSID: " + ssid + " with password: " + password);
    return true;
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
