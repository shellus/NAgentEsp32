WiFiClient client;

// 心跳间隔毫秒数
#define HEARTBEAT_INTERVAL 30000
bool _clientConnected = false;
// 最后活跃时间 lastActiveTime
unsigned long lastActiveTime = 0;

#define TypeNAgentRegister 702
#define TypeNAgentAuth 710
#define TypeHeartbeat 712
#define ResponseOK 100
#define ResponseError 110

// todo WiFiClient 如何知道连接断开，需要维护_clientConnected的状态变化
// todo 例如服务端断开，wifi断开，睡眠唤醒断开

bool _clientConnect() {
    if (_clientConnected) {
        return true;
    }
    std::string host, port;
    if (!getConfigServer(host, port)) {
        Serial.println("TCP Client connect address not found.");
        return false;
    }
    Serial.printf("TCP Client connect to %s:%s ...\n", host.c_str(), port.c_str());
//    if (!client.connect(host.c_str(), port.toInt())) {
    if (!client.connect(host.c_str(), atoi(port.c_str()))) {
        Serial.println("TCP Client connect failed.");
        return false;
    }
    Serial.println("TCP Client connect success.");
    _clientConnected = true;
    return true;
}

bool NAgentRegister(std::string &authJson) {
    if (!_clientConnect()) {
        return false;
    }
    uint32_t type = TypeNAgentRegister;
    uint32_t len = 0;
    client.write((uint8_t *)&type, sizeof(type));
    client.write((uint8_t *)&len, sizeof(len));

    Serial.println("NAgentRegister request sent.");
    uint32_t responseType;
    if (_NAgentReadOkOrError(responseType, authJson)) {
        Serial.printf("NAgentRegister got: %s\n", authJson.c_str());
        return true;
    }
    return false;
}


bool NAgentAuth (std::string authJson) {
    if (!_clientConnect()) {
        return false;
    }
    // 发送请求，格式为：<type LittleEndian.Uint32><len LittleEndian.Uint32><json String>
    uint32_t type = TypeNAgentAuth;
    uint32_t len = authJson.length();
    client.write((uint8_t *)&type, sizeof(type));
    client.write((uint8_t *)&len, sizeof(len));
    client.print(authJson.c_str());

    Serial.println("NAgentAuth request sent.");

    // 等待返回
    Serial.println("NAgentAuth wait response...");
    // 使用 _NAgentRead 读取返回
    uint32_t responseType;
    std::string responseStr;
    if (_NAgentReadOkOrError(responseType, responseStr)) {
        // 刷新心跳时间
        lastActiveTime = millis();
        Serial.println("NAgentAuth response success.");
        return true;
    } else {
        Serial.println("NAgentAuth response fail.");
        return false;
    }
}
bool _NAgentReadOkOrError(uint32_t &responseType, std::string &responseStr) {
    if (!_NAgentRead(responseType, responseStr, 1 * 1000)) {
        Serial.println("_NAgentReadOkOrError timeout.");
        return false;
    }
    if (responseType == ResponseOK) {
        return true;
    } else if(responseType == ResponseError) {
        Serial.printf("_NAgentReadOkOrError got error response: %s\n", responseStr.c_str());
        return false;
    } else {
        Serial.printf("_NAgentReadOkOrError got other response type: %d\n", responseType);
        return false;
    }
}
// 读出 type, authJson
bool _NAgentRead(uint32_t &responseType, std::string &responseStr, uint32_t timeout) {
    // timeout
    unsigned long start = millis();
    while (client.available() < 8) {
        if (millis() - start > timeout) {
            if (timeout != 0) {
                Serial.println("_NAgentRead head timeout.");
            }
            return false;
        }
        delay(10);
    }

    // 读取返回，格式为：<type LittleEndian.Uint32><len LittleEndian.Uint32><json String>
    // WiFiClient 是 Stream 的子类，所以可以直接使用 Stream 的方法，例如readBytes
    byte headBytes[8];
    client.readBytes(headBytes, 8);
    responseType = byteUInt32(headBytes);
    uint32_t responseLen = byteUInt32(headBytes + 4);
    responseStr = "";
    Serial.printf("_NAgentRead response received. responseType: %d, responseLen: %d\n", responseType, responseLen);
    // 读取到responseLen数据足够为止
    while (responseStr.length() < responseLen) {
        if (millis() - start > timeout) {
            return false;
        }
//        responseStr += client.readString();
        while (client.available() > 0) {
            responseStr += (char)client.read();
        }
    }
    // 如果类型为ResponseOK则不打印，因为这没有任何问题
    if (responseType != ResponseOK) {
        Serial.printf("_NAgentRead response received. responseType: %d, responseLen: %d, responseStr: %s\n", responseType, responseLen, responseStr.c_str());
    }
    return true;
}

bool NAgentLoop() {
    if (!client.connected()) {
        Serial.println("NAgentLoop client not connected.");
        return false;
    }

    uint32_t responseType;
    std::string responseStr = "";
    if (_NAgentRead(responseType, responseStr, 0)) {
        Serial.printf("NAgentLoop response received. responseType: %d, responseStr: %s\n", responseType, responseStr.c_str());
    }

    return true;
}

bool NAgentHeartbeatTimer() {
    // 每隔60秒发送一次心跳
    unsigned long now = millis();
    if (now - lastActiveTime < HEARTBEAT_INTERVAL) {
        return true;
    }

    // 发送心跳，格式为：<type LittleEndian.Uint32><len LittleEndian.Uint32><json String>
    uint32_t type = TypeHeartbeat;
    uint32_t len = 0;
    client.write((uint8_t *)&type, sizeof(type));
    client.write((uint8_t *)&len, sizeof(len));

    Serial.printf("NAgentHeartbeat request sent. lastActiveTime: %d, now: %d\n", lastActiveTime, now);
    uint32_t responseType;
    std::string responseStr;
    if (_NAgentReadOkOrError(responseType, responseStr)) {
        lastActiveTime = now;
        return true;
    }
    return false;
}
