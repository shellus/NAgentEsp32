// 注意了，arduino所有的“json” 都是“Json” 而不是“JSON”

void clearAuthJson() {
    if (SPIFFS.exists("/auth.json")) {
        SPIFFS.remove("/auth.json");
        Serial.println("auth.json file removed.");
    }
}

bool saveAuthJson(std::string authJson) {
    // 如果成功，存为文件
    File configFile = SPIFFS.open("/auth.json", "w");
    if (!configFile) {
        Serial.println("Failed to open auth.json file for writing");
        return false;
    }
    configFile.print(authJson.c_str());
    configFile.close();
    Serial.println("auth.json file saved.");
    return true;
}

bool loadAuthJson(std::string authJson) {
    if (!SPIFFS.exists("/auth.json")) {
        Serial.println("auth.json file does not exist.");
        return false;
    }
    File configFile = SPIFFS.open("/auth.json", "r");
    if (!configFile) {
        Serial.println("Failed to open auth.json file for reading");
        return false;
    }
    char * buffer = new char[configFile.size()];
    configFile.readBytes(buffer, configFile.size());
    authJson = buffer;
    delete[] buffer;

    configFile.close();
    Serial.println("auth.json file loaded.");
    return true;
}
