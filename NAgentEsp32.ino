#include "WiFi.h"
#include "SPIFFS.h"
// 用于parseCommand的动态数组
#include <vector>
// 需要手动安装的Arduino库：
// ArduinoJson - https://arduinojson.org
#include "ArduinoJson.h"

// M5Stack Dev Board
// Button A: GPIO39	Button  B: GPIO38    Button C: GPIO37
// Speaker: GPIO25
//#define USER_BUTTON_PIN 38

// ESP32 Dev Board
#define USER_BUTTON_PIN 0
#define LED_D2_PIN 2

// WiFi状态
bool wifiConnected = false;
// 记录 NAgentAuth status，只有在连接成功后才在loop接收输入
bool NAgentAuthed = false;

// 按钮事件处理
void onButton(){
    Serial.println("Button pressed");
}

// pin设置命令处理
void onPin(std::string pin, std::string value){
    int pinNumber = atoi(pin.c_str());
    int pinValue = atoi(value.c_str());
    Serial.printf("Set pin output Mode [%d] to %d\n", pinNumber, pinValue);
    pinMode(pinNumber, OUTPUT);
    digitalWrite(pinNumber, pinValue);
}

// wifi连接后，可能是开机时候，也可能是配置成功后
void wifiConnectAfter(){
    std::string authJson;
    if (!loadAuthJson(authJson)) {
        Serial.println("No auth.json found, start register");
        if (NAgentRegister(authJson)) {
            saveAuthJson(authJson);
        } else {
            Serial.println("Register failed");
            return;
        }
    }
    if (NAgentAuth(authJson)) {
        NAgentAuthed = true;
        GattServerStart(GetBootNum());
    }
}
void printDebugInfo() {
    uint32_t chipId = 0;
    for(int i=0; i<17; i=i+8) {
      chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
    }

    Serial.printf("ESP32 Chip %d model: %s Rev %d in %d cores\n", chipId, ESP.getChipModel(), ESP.getChipRevision(), ESP.getChipCores());

    // 还有这些可用信息
    // 来自： <User_Dir>\AppData\Local\Arduino15\packages\esp32\hardware\esp32\2.0.11\cores\esp32\Esp.cpp
    // getFreeSketchSpace
    // getSketchMD5 getSketchSize sketchSize
    // getMaxAllocPsram getMinFreePsram
    // getFreePsram getPsramSize
}
void setup() {
    Serial.begin(115200);
    Serial.setDebugOutput(true);
    // 准备按钮
    pinMode(USER_BUTTON_PIN, INPUT_PULLUP);

    Serial.println("");

    bool WAKEUP_BY_TIMER = esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TIMER;
    Serial.printf("\n================= ESP32 %s start =================\n", WAKEUP_BY_TIMER ? "wakeup" : "setup");

    // 打印调试信息
    if(!WAKEUP_BY_TIMER) {
        printDebugInfo();
    }


    // SPIFFS 初始化
    if(!SPIFFS.begin(true)){
        Serial.println("SPIFFS Mount Failed");
        return;
    } else {
        Serial.println("SPIFFS Mount Success");
    }
    // 加载全局配置
    LoadConfig();

    // 打印开机次数
    if (!WAKEUP_BY_TIMER) {
        Serial.printf("Boot number: %d\n", IncBootNum());
    }

    // 如果之前有wifi配置，那么尝试连接
    std::string ssid, password;
    if(getConfigWifi(ssid, password)) {
        if (onWifi(ssid, password)) {
            wifiConnected = true;
        } else {
            Serial.println("WiFi connect failed");
        }
    } else {
        Serial.println("No WiFi config found");
    }

    // 所有初始化完成后，再执行连接
    if (wifiConnected) {
        Serial.println("WiFi wifiConnect after action:");
        wifiConnectAfter();
    }
    Serial.printf("\n================= ESP32 %s complete =================\n", WAKEUP_BY_TIMER ? "wakeup" : "setup");
}

void loop() {
    loopButton();

    loopSerial();

    if (NAgentAuthed) {
        if (!NAgentLoop()) {
            Serial.println("NAgent loop read failed disconnected");
            NAgentAuthed = false;
        }
        if (!NAgentHeartbeatTimer()) {
            Serial.println("NAgent heartbeat failed disconnected");
            NAgentAuthed = false;
        }
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
    std::string input = "";
    while(Serial.available()) {
        input += (char)Serial.read();
    }
    if (input.length() == 0) {
        return;
    }
    // 串口输入的可能有带换行符，将其去掉
    if (input[input.length() - 1] == '\n') {
        input = input.substr(0, input.length() - 1);
    }
    if (input == "") {
        Serial.println("# type \"help\" list all commands");
        return;
    }
    // 将输入内容在前面加上'# '并原样输出，用作输入反馈
    Serial.printf("# %s\n", input.c_str());

    std::string command;
    std::vector<std::string> args;
    std::string lineBuf = input.c_str();

    ParseCommand(lineBuf, command, args);
    dispatchCommand(command, args);
}
