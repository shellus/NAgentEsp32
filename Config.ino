// 配置的结构体为JSON
#define configFileName "/config.json"
bool isLoadConfig = false;
DynamicJsonDocument configObj(1024);

// 默认的配置
void setDefaultConfig(){
//    configObj["nstartup_server_addr"] = "nstartup.dev.lan";
//    configObj["nstartup_server_port"] = "8080";
}
void setConfigServer(String addr, String port){
    configObj["nstartup_server_addr"] = addr;
    configObj["nstartup_server_port"] = port;
    saveConfig();
    Serial.println("Success to set server config: " + addr + " = " + port);
}
bool getConfigServer(String &addr, String &port){
    if (!configObj.containsKey("nstartup_server_addr")) {
        return false;
    }
    addr = configObj["nstartup_server_addr"].as<String>();
    port = configObj["nstartup_server_port"].as<String>();
    Serial.println("Success to get server config: " + addr + " = " + port);
    return true;
}

void setConfigWifi(String ssid, String password){
    configObj["wifi_ssid"] = ssid;
    configObj["wifi_password"] = password;
    saveConfig();
    Serial.println("Success to set wifi config: " + ssid + " = " + password);
}
bool getConfigWifi(String &ssid, String &password){
    if (!configObj.containsKey("wifi_ssid")) {
        return false;
    }

    ssid = configObj["wifi_ssid"].as<String>();
    password = configObj["wifi_password"].as<String>();
    Serial.println("Success to get wifi config: " + ssid + " = " + password);
    return ssid.length() > 0;
}
void clearWifiConfig(){
    configObj.remove("wifi_ssid");
    configObj.remove("wifi_password");
    saveConfig();
    Serial.println("Success to clear wifi config");
}

// 自增和返回开机次数
uint32_t IncBootNum(){
    String key = "boot_num";
    uint32_t value = configObj[key] | 0;
    value++;
    configObj[key] = value;
    saveConfig();
    return value;
}

String GetBootNum(){
    return configObj["boot_num"];
}

void setConfigString(String key, String value){
    configObj[key] = value;
    saveConfig();
    Serial.println("Success to set config string: " + key + " = " + value);
}

void deleteConfig(String key){
    configObj.remove(key);
    saveConfig();
    Serial.println("Success to delete config key: " + key);
}

void printConfig(){
    Serial.println("Config:");
    serializeJsonPretty(configObj, Serial);
}

String getConfigString(String key){
    String value = configObj[key];
    Serial.println("Success to get config string: " + key + " = " + value);
    return value;
}
void saveConfig(){
    File configFile = SPIFFS.open(configFileName, "w");
    if (!configFile) {
        Serial.println("Failed to open <configFileName> for writing");
    }
    serializeJson(configObj, configFile);
    configFile.close();
    Serial.println("config config saved");
}

void LoadConfig(){
    // 只会读取一次
    if (isLoadConfig) return;

    // 读取默认值
    if (!SPIFFS.exists(configFileName)) {
        setDefaultConfig();
        saveConfig();
        isLoadConfig = true;
        Serial.println("loadConfig not exist, create default config");
        return;
    }

    // 读取配置
    File configFile = SPIFFS.open(configFileName, "r");
    if (!configFile) {
        Serial.println("Failed to open <configFileName> for reading");
    }
    DeserializationError error = deserializeJson(configObj, configFile);
    if (error) {
        Serial.println("Failed to read <configFileName>");
    }
    configFile.close();
    isLoadConfig = true;
    Serial.printf("Load config from %s\n", configFileName);
}
