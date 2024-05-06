#include "WiFi.h"
#include "SPIFFS.h"
// 需要手动安装的Arduino库：
// ArduinoJson - https://arduinojson.org
#include "ArduinoJson.h"


#define USER_BUTTON_PIN 0
#define LED_D2_PIN 2

// WiFi状态
bool wifiConnected = false;
// 记录 NAgentConnect status，只有在连接成功后才在loop接收输入
bool NAgentConnected = false;

// 按钮事件处理
void onButton(){
    Serial.println("Button pressed");
}

// pin设置命令处理
void onPin(String pin, String value){
    int pinNumber = pin.toInt();
    int pinValue = value.toInt();
    Serial.println("Set pin output Mode [" + String(pinNumber) + "] to " + String(pinValue));
    pinMode(pinNumber, OUTPUT);
    digitalWrite(pinNumber, pinValue);
}

// wifi连接后，可能是开机时候，也可能是配置成功后
void wifiConnectAfter(){
    if (!NAgentConnectReady()) {
        Serial.println("NAgent not ready");
        return;
    }
    String authJson;
    if (loadAuthJson(authJson) && NAgentConnect(authJson)) {
        NAgentConnected = true;
    }
}

void setup() {
    Serial.begin(115200);
    Serial.setDebugOutput(true);

    Serial.println("");
    Serial.println("================= ESP32 start =================");

    // 准备按钮
    pinMode(USER_BUTTON_PIN, INPUT_PULLUP);

    // SPIFFS 初始化
    if(!SPIFFS.begin(true)){
        Serial.println("SPIFFS Mount Failed");
        return;
    } else {
        Serial.println("SPIFFS Mount Success");
    }

    // 如果之前有wifi配置，那么尝试连接
    if(SPIFFS.exists("/wifi.json")) {
        String ssid, password;
        if (loadWifiConfig(ssid, password) && onWifi(ssid, password)) {
            wifiConnected = true;
        }
    } else {
        Serial.println("No WiFi config found");
    }

    // 打印调试信息
    Serial.print("ESP32 SDK: ");
    Serial.println(ESP.getSdkVersion());

    Serial.println("================= ESP32 setup complete =================");

    // 所有初始化完成后，再执行连接
    if (wifiConnected) {
        wifiConnectAfter();
    }
}

void loop() {
    loopButton();

    loopSerial();

    if (NAgentConnected) {
        if (!NAgentLoop()) {
            Serial.println("NAgent disconnected");
            NAgentConnected = false;
        }
        NAgentHeartbeat();
    }
}

void loopButton() {
    // 监听按钮按下
    static uint8_t lastPinState = 1;
    uint8_t pinState = digitalRead(USER_BUTTON_PIN);
    if(!pinState && lastPinState){
        onButton();
    }
    lastPinState = pinState;
}

void loopSerial() {
        // 监听串口输入
        String input = "";
        while(Serial.available()) {
            input += (char)Serial.read();
        }
        // 将输入内容在前面加上'# '并原样输出，用于调试
        if(input.length() > 0) {
            Serial.print("# ");
            Serial.println(input);
        }

        // 如果输入内容以 "wifi:" 开头，那么认为这是一个 WiFi 配置信息
        if(input.startsWith("wifi:")) {
            int commaIndex = input.indexOf(',');
            if(commaIndex != -1) {
                String ssid = input.substring(5, commaIndex);
                String password = input.substring(commaIndex + 1);
                password.trim();
                if (onWifi(ssid, password)) {
                    saveWifiConfig(ssid, password);
                    wifiConnected = true;
                    wifiConnectAfter();
                }
            }
        }

        // 如果是authJson，那么认为这是一个认证信息
        // 注意了，arduino所有的“json” 都是“Json” 而不是“JSON”
        if(input.startsWith("authJson:")) {
            String authJson = input.substring(9);
            authJson.trim();
            if (NAgentConnect(authJson)) {
                saveAuthJson(authJson);
                NAgentConnected = true;
                Serial.println("NAgent connected, authJson saved.");
            } else {
                Serial.println("NAgent connect failed, authJson not saved.");
            }
        }
        if(input.startsWith("clearAuthJson")) {
            clearAuthJson();
        }


        if(input.startsWith("clearWifi")) {
            clearWifi();
        }
        // 如果输入内容以 "pin:" 开头，那么认为这是一个 GPIO 控制信息
        if(input.startsWith("pin:")) {
            int commaIndex = input.indexOf(',');
            if(commaIndex != -1) {
                String pin = input.substring(4, commaIndex);
                String value = input.substring(commaIndex + 1);
                value.trim();
                onPin(pin, value);
            }
        }
}
