#include "WiFi.h"
#include "SPIFFS.h"
// 需要手动安装的Arduino库：
// ArduinoJson - https://arduinojson.org
#include "ArduinoJson.h"


#define USER_BUTTON_PIN 0
#define LED_D2_PIN 2

// 按钮事件处理
void onButton(){
    Serial.println("Button pressed");
}

// pin:2,1 处理
void onPin(String pin, String value){
    int pinNumber = pin.toInt();
    int pinValue = value.toInt();
    Serial.println("Set pin output Mode [" + String(pinNumber) + "] to " + String(pinValue));
    pinMode(pinNumber, OUTPUT);
    digitalWrite(pinNumber, pinValue);
}


void setup() {
    Serial.begin(115200);
    Serial.setDebugOutput(true);
    // 输出串口清屏代码
    Serial.println("\033[2J\033[H");
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

    bool NAgentConnectExec = false;
    // 如果之前有wifi配置，那么尝试连接
    if(SPIFFS.exists("/wifi.json")) {
        String ssid, password;
        if (loadWifiConfig(ssid, password) && onWifi(ssid, password)) {
            NAgentConnectExec = true;
        }
    } else {
        Serial.println("No WiFi config found");
    }

    // 打印调试信息
    Serial.print("ESP32 SDK: ");
    Serial.println(ESP.getSdkVersion());

    Serial.println("================= ESP32 setup complete =================");

    // 所有初始化完成后，再执行连接
    if (NAgentConnectExec) {
        NAgentConnect();
    }
}

void loop() {
    // 监听按钮按下
    static uint8_t lastPinState = 1;
    uint8_t pinState = digitalRead(USER_BUTTON_PIN);
    if(!pinState && lastPinState){
        onButton();
    }
    lastPinState = pinState;

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
                NAgentConnect();
            }
        }
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