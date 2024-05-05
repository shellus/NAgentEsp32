WiFiClient client;

bool NAgentConnect () {
    String host = "192.168.6.115";
    String port = "8080";
    Serial.println("Start NAgentConnect to " + host + ":" + port + " ...");

    if (!client.connect(host.c_str(), port.toInt())) {
        Serial.println("NAgentConnect connection failed.");
        return false;
    }
    Serial.println("NAgentConnect connection success.");
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


    // 序列化JSON
    String authJson = "";
    serializeJson(doc, authJson);
    // 发送请求，格式为：<type LittleEndian.Uint32><len LittleEndian.Uint32><json String>
    uint32_t type = 1;
    uint32_t len = authJson.length();
    client.write((uint8_t *)&type, sizeof(type));
    client.write((uint8_t *)&len, sizeof(len));
    client.print(authJson);
    Serial.println("NAgentConnect request sent.");

    // 等待返回
    Serial.println("NAgentConnect wait response...");
    while (client.available() == 0) {
        delay(100);
    }
    // 读取返回
    String response = client.readStringUntil('\r');
    Serial.println(response);
    // 解析返回
    DynamicJsonDocument responseDoc(1024);
    DeserializationError error = deserializeJson(responseDoc, response);
    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
        return false;
    }
    // 判断返回
    if (responseDoc["status"] == "success") {
        Serial.println("Auth success.");
        return true;
    } else {
        Serial.println("Auth failed.");
        return false;
    }

    // 如果成功，存为文件
    File configFile = SPIFFS.open("/auth.json", "w");
    if (!configFile) {
        Serial.println("Failed to open auth.json file for writing");
        return false;
    }

    return true;
}