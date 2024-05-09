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
void onPin(String pin, String value){
    int pinNumber = pin.toInt();
    int pinValue = value.toInt();
    Serial.println("Set pin output Mode [" + String(pinNumber) + "] to " + String(pinValue));
    pinMode(pinNumber, OUTPUT);
    digitalWrite(pinNumber, pinValue);
}

// wifi连接后，可能是开机时候，也可能是配置成功后
void wifiConnectAfter(){
    String authJson;
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
    }
}

void setup() {
    Serial.begin(115200);
    Serial.setDebugOutput(true);

    Serial.println("");
    Serial.println("================= ESP32 start =================");

    // 打印调试信息
    uint32_t chipId = 0;
    for(int i=0; i<17; i=i+8) {
      chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
    }

    Serial.printf("ESP32 Chip model: %s Rev %d in %d cores\n", ESP.getChipModel(), ESP.getChipRevision(), ESP.getChipCores());
    Serial.printf("Chip ID: %d (", chipId);
    Serial.print(ESP.getEfuseMac(), HEX);
    Serial.println(")");
    Serial.print("ESP32 SDK: ");
    Serial.println(ESP.getSdkVersion());

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

    // 所有初始化完成后，再执行连接
    if (wifiConnected) {
        Serial.println("WiFi wifiConnect after action:");
        wifiConnectAfter();
    }
    Serial.println("================= ESP32 setup complete =================");
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
    String input = "";
    while(Serial.available()) {
        input += (char)Serial.read();
    }
    if (input.length() == 0) {
        return;
    }
    // 串口输入的可能有带换行符，将其去掉
    if (input.endsWith("\n")) {
        input = input.substring(0, input.length() - 1);
    }
    if (input == "") {
        Serial.println("# type \"help\" list all commands");
        return;
    }
    // 将输入内容在前面加上'# '并原样输出，用作输入反馈
    Serial.println("# " + input);

    String command;
    std::vector<String> args;
    if (!ParseCommand(input, &command, args)) {
        Serial.println("Parse command failed");
        return;
    }
    if (command == "wifi") {
        if (args.size() != 2) {
            Serial.println("Invalid wifi command");
            return;
        }
        String ssid = args[0];
        String password = args[1];
        if (onWifi(ssid, password)) {
            saveWifiConfig(ssid, password);
            wifiConnected = true;
            wifiConnectAfter();
        }
    } else if (command == "setConfigString") {
        if (args.size() != 2) {
            Serial.println("Invalid setConfigString command");
            return;
        }
        setConfigString(args[0], args[1]);
    } else if (command == "getConfigString") {
        if (args.size() != 1) {
            Serial.println("Invalid getConfigString command");
            return;
        }
        String value = getConfigString(args[0]);
        Serial.println(value);
    } else if (command == "deleteConfig") {
        if (args.size() != 1) {
            Serial.println("Invalid deleteConfig command");
            return;
        }
        deleteConfig(args[0]);
    } else if (command == "printConfig") {
        printConfig();
    } else if (command == "clearWifi") {
        clearWifi();
    } else if (command == "pin") {
        if (args.size() != 2) {
            Serial.println("Invalid pin command");
            return;
        }
        onPin(args[0], args[1]);
    } else if (command == "sleep") {
        LightSleep(10 * 1000);
    } else if (command == "reboot") {
        ESP.restart();
    } else if (command == "connect") {
        wifiConnectAfter();
    } else if (command == "test") {
        Test();
    } else if (command == "help") {
        PrintHelp();
    } else if (command == "list") {
        PrintSPIFFSFileList();
    } else if (command == "write") {
        if (args.size() != 2) {
            Serial.println("Invalid write command");
            return;
        }
        WriteSPIFFSFile(args[0], args[1]);
    } else if (command == "read") {
        if (args.size() != 1) {
            Serial.println("Invalid read command");
            return;
        }
        PrintSPIFFSFileContent(args[0]);
    } else if (command == "delete") {
        if (args.size() != 1) {
            Serial.println("Invalid delete command");
            return;
        }
        DeleteSPIFFSFile(args[0]);
    } else {
        Serial.println("Unknown command [" + command + "], type \"help\" list all commands");
    }
}
