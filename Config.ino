// 配置的结构体为JSON
#define configFileName "/config.json"
bool isLoadConfig = false;
DynamicJsonDocument configObj(1024);

// 默认的配置
void setDefaultConfig(){
    configObj["device_name"] = "NA-" + getRandString(8);
//    configObj["nstartup_server_addr"] = "nstartup.dev.lan";
//    configObj["nstartup_server_port"] = "8080";
}
void setConfigServer(std::string addr, std::string port){
    configObj["nstartup_server_addr"] = addr;
    configObj["nstartup_server_port"] = port;
    saveConfig();
//    Serial.println("Success to set server config: " + addr + " = " + port);
    Serial.printf("Success to set server config: %s = %s\n", addr.c_str(), port.c_str());
}
bool getConfigServer(std::string &addr, std::string &port){
    if (!configObj.containsKey("nstartup_server_addr")) {
        return false;
    }
    addr = configObj["nstartup_server_addr"].as<std::string>();
    port = configObj["nstartup_server_port"].as<std::string>();
//    Serial.println("Success to get server config: " + addr + " = " + port);
    Serial.printf("Success to get server config: %s = %s\n", addr.c_str(), port.c_str());
    return true;
}

void setConfigWifi(std::string ssid, std::string password){
    configObj["wifi_ssid"] = ssid;
    configObj["wifi_password"] = password;
    saveConfig();
//    Serial.println("Success to set wifi config: " + ssid + " = " + password);
    Serial.printf("Success to set wifi config: %s = %s\n", ssid.c_str(), password.c_str());
}
bool getConfigWifi(std::string &ssid, std::string &password){
    if (!configObj.containsKey("wifi_ssid")) {
        return false;
    }

    ssid = configObj["wifi_ssid"].as<std::string>();
    password = configObj["wifi_password"].as<std::string>();
//    Serial.println("Success to get wifi config: " + ssid + " = " + password);
    Serial.printf("Success to get wifi config: %s = %s\n", ssid.c_str(), password.c_str());
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
    std::string key = "boot_num";
    uint32_t value = configObj[key] | 0;
    value++;
    configObj[key] = value;
    saveConfig();
    return value;
}

std::string GetBootNum(){
    return configObj["boot_num"];
}

void setConfigString(std::string key, std::string value){
    configObj[key] = value;
    saveConfig();
//    Serial.println("Success to set config string: " + key + " = " + value);
    Serial.printf("Success to set config string: %s = %s\n", key.c_str(), value.c_str());
}

void deleteConfig(std::string key){
    configObj.remove(key);
    saveConfig();
//    Serial.println("Success to delete config key: " + key);
    Serial.printf("Success to delete config key: %s\n", key.c_str());
}

void printConfig(){
    Serial.println("Config:");
    serializeJsonPretty(configObj, Serial);
}

std::string getConfigString(std::string key){
    std::string value = configObj[key];
//    Serial.println("Success to get config string: " + key + " = " + value);
    Serial.printf("Success to get config string: %s = %s\n", key.c_str(), value.c_str());
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
