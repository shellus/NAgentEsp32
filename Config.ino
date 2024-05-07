// 配置的结构体为JSON
#define configFileName "/config.json"
bool isLoadConfig = false;
DynamicJsonDocument configObj(1024);

// 默认的配置
void setDefaultConfig(){
    // 设置默认配置
    configObj["nstartup_server"] = "192.168.1.2:8888";
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
    loadConfig();
    Serial.println("Config:");
    serializeJsonPretty(configObj, Serial);
}

String getConfigString(String key){
    loadConfig();
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

void loadConfig(){
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
    Serial.println("Load config config from flash");
}
