// 草图展示了如何使用 SimpleBLE 来宣传设备名称，并在按下按钮时更改名称。
// 如果您想发布某种信息，它将非常有用
// 按钮连接在 GPIO 0 和 GND 之间，每次按下按钮时设备名称都会改变。

#include "SimpleBLE.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

SimpleBLE ble;

void onButton(){
    Serial.println("Button pressed");
    String out = "BLE32 name: ";
    out += String(millis() / 1000);
    Serial.println(out);
    ble.begin(out);
}

void setup() {
    Serial.begin(115200);
    Serial.setDebugOutput(true);
    pinMode(0, INPUT_PULLUP);
    Serial.print("ESP32 SDK: ");
    Serial.println(ESP.getSdkVersion());
    ble.begin("ESP32 SimpleBLE");
    Serial.println("Press the button to change the device's name");
}

void loop() {
    static uint8_t lastPinState = 1;
    uint8_t pinState = digitalRead(0);
    if(!pinState && lastPinState){
        onButton();
    }
    lastPinState = pinState;
    while(Serial.available()) Serial.write(Serial.read());
}
