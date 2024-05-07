WiFiClient client;

// 心跳间隔时间60秒
#define HEARTBEAT_INTERVAL 60000
// 最后活跃时间 lastActiveTime
unsigned long lastActiveTime = 0;

#define AgentAuthRequest 710
#define TypeHeartbeat 712
#define ResponseOK 100
#define ResponseError 110

bool NAgentConnect (String authJson) {
    String addr = getConfigString("nstartup_server");
    String host = addr.substring(0, addr.indexOf(":"));
    String port = addr.substring(addr.indexOf(":") + 1);
    Serial.println("Start NAgentConnect to " + host + ":" + port + " ...");
    if (!client.connect(host.c_str(), port.toInt())) {
        Serial.println("TCP Client connect failed.");
        return false;
    }
    Serial.println("TCP Client connect success.");

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
    if (NAgentReadOkOrError(responseType, errorStr)) {
        // 刷新心跳时间
        lastActiveTime = millis();
        Serial.println("NAgentConnect response success.");
        return true;
    } else {
        Serial.println("NAgentConnect response fail.");
        return false;
    }
}
bool NAgentReadOkOrError(uint32_t &responseType, String &errorStr) {
    if (!NAgentRead(responseType, errorStr, 1000)) {
        Serial.println("NAgentReadOkOrError timeout.");
        return false;
    }
    if (responseType == ResponseOK) {
        return true;
    } else if(responseType == ResponseError) {
        Serial.println("NAgentReadOkOrError got error response: " + errorStr);
        return false;
    } else {
        Serial.println("NAgentReadOkOrError got other response type: " + String(responseType));
        return false;
    }
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
    // 如果类型为ResponseOK则不打印，因为这没有任何问题
    if (responseType != ResponseOK) {
        Serial.println("NAgentRead response received. responseType: " + String(responseType) + ", responseLen: " + String(responseLen) + ", responseJson: " + responseJson);
    }
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

    Serial.println("NAgentHeartbeat request sent in " + String(now) + ".");
    uint32_t responseType;
    String errorStr;
    if (NAgentReadOkOrError(responseType, errorStr)) {
        lastActiveTime = now;
    }
}
