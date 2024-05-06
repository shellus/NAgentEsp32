WiFiClient client;

// 心跳间隔时间60秒
#define HEARTBEAT_INTERVAL 60000
// 最后活跃时间 lastActiveTime
unsigned long lastActiveTime = 0;

#define AgentAuthRequest 1
#define TypeHeartbeat 7
#define ResponseOK 5
#define ResponseError 6

// getAuthJson
String getAuthJson() {
    // todo 改为从文件读取json配置
    // 生成一个这样的UUID "77115baf-3e05-485c-a449-24b465e9fdb7"
    String uuid = "77115baf-3e05-485c-a449-24b465e9fdb7";
    // auth.json 内容
//    {
//      "id": "77115baf-3e05-485c-a449-24b465e9fdb7",
//      "wol_infos": [
//        {
//          "name": "myServer-001",
//          "mac_addr": "10:90:27:e9:c4:99",
//          "port": 9,
//          "broadcast_addr": "10.242.255.255",
//          "ip": "10.242.38.27"
//        }
//      ]
//    }

    DynamicJsonDocument doc(1024);
    doc["id"] = uuid;
    JsonArray wol_infos = doc.createNestedArray("wol_infos");
    JsonObject wol_info = wol_infos.createNestedObject();
    wol_info["name"] = "myServer-001";
    wol_info["mac_addr"] = "10:90:27:e9:c4:99";
    wol_info["port"] = 9;
    wol_info["broadcast_addr"] = "10.242.255.255";
    wol_info["ip"] = "10.242.38.27";

    String authJson = "";
    serializeJson(doc, authJson);
    return authJson;
}

bool NAgentConnect () {
    String host = "192.168.6.115";
    String port = "8080";
    Serial.println("Start NAgentConnect to " + host + ":" + port + " ...");

    if (!client.connect(host.c_str(), port.toInt())) {
        Serial.println("NAgentConnect connection failed.");
        return false;
    }
    Serial.println("NAgentConnect connection success.");

    String authJson = getAuthJson();
    // 发送请求，格式为：<type LittleEndian.Uint32><len LittleEndian.Uint32><json String>
    uint32_t type = AgentAuthRequest;
    uint32_t len = authJson.length();
    client.write((uint8_t *)&type, sizeof(type));
    client.write((uint8_t *)&len, sizeof(len));
    client.print(authJson);

    Serial.println("NAgentConnect request sent.");

    // 等待返回
    Serial.println("NAgentConnect wait response...");
    // 使用 NAgentRead 读取返回
    uint32_t responseType;
    String errorStr;
    NAgentReadOkOrError(responseType, errorStr);
    return true;

    // 如果成功，存为文件
    File configFile = SPIFFS.open("/auth.json", "w");
    if (!configFile) {
        Serial.println("Failed to open auth.json file for writing");
        return false;
    }

    // 刷新心跳时间
    lastActiveTime = millis();
    return true;
}
bool NAgentReadOkOrError(uint32_t &responseType, String &errorStr) {
    if (!NAgentRead(responseType, errorStr, 1000)) {
        return false;
    }
    if (responseType == ResponseOK || responseType == ResponseError) {
        return true;
    }
    return false;
}
// 读出 type, authJson
bool NAgentRead(uint32_t &responseType, String &responseJson, uint32_t timeout) {
    // timeout
    unsigned long start = millis();
    while (client.available() < 8) {
        if (millis() - start > timeout) {
            if (timeout != 0) {
                Serial.println("NAgentRead timeout.");
            }
            return false;
        }
        delay(100);
    }

    // 读取返回，格式为：<type LittleEndian.Uint32><len LittleEndian.Uint32><json String>
    uint32_t responseType2;
    uint32_t responseLen;
    // todo 这里的类型有问题，编译不通过
    client.read((uint8_t *)&responseType2, sizeof(responseType2));
    client.read((uint8_t *)&responseLen, sizeof(responseLen));
    responseType = responseType2;
    responseJson = "";
    // 读取到responseLen数据足够为止
    while (responseJson.length() < responseLen) {
        if (millis() - start > timeout) {
            return false;
        }
        responseJson += client.readString();
    }

    Serial.println("NAgentRead response received. responseType: " + String(responseType) + ", responseLen: " + String(responseLen) + ", responseJson: " + responseJson);
    return true;
}

bool NAgentLoop() {
    if (!client.connected()) {
        Serial.println("NAgentLoop client not connected, try to reconnect.");
        return false;
    }

    uint32_t responseType;
    String responseJson = "";
    if (NAgentRead(responseType, responseJson, 0)) {
        Serial.println("NAgentLoop response received. responseType: " + String(responseType) + ", responseJson: " + responseJson);
    }

    return true;
}

void NAgentHeartbeat() {

    // 每隔60秒发送一次心跳
    unsigned long now = millis();
    if (now - lastActiveTime < HEARTBEAT_INTERVAL) {
        return;
    }


    // 发送心跳，格式为：<type LittleEndian.Uint32><len LittleEndian.Uint32><json String>
    uint32_t type = TypeHeartbeat;
    uint32_t len = 0;
    client.write((uint8_t *)&type, sizeof(type));
    client.write((uint8_t *)&len, sizeof(len));

    Serial.println("NAgentHeartbeat request sent.");
    lastActiveTime = now;
}