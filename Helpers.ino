
void PrintSPIFFSFileList() {
  Serial.println("SPIFFS File List:");
  File root = SPIFFS.open("/");
  File file = root.openNextFile();
  while (file) {
    Serial.print("  FILE: ");
    Serial.println(file.name());
    file.close();
    file = root.openNextFile();
  }
}

void PrintSPIFFSFileContent(String filename) {
  Serial.print("SPIFFS File [" + filename + "] Content:");

  File file = SPIFFS.open(filename, "r");
  if (!file) {
    Serial.println("  ERROR: File not found");
    return;
  }
  while (file.available()) {
    Serial.write(file.read());
  }
  file.close();
  Serial.println();
}
void PrintHelp() {
  Serial.println("Available commands:");
  Serial.println("  wifi <ssid> <password> - connect to wifi");
  Serial.println("  setConfigString <key> <value> - set config string");
  Serial.println("  getConfigString <key> - get config string");
  Serial.println("  deleteConfig <key> - delete config");
  Serial.println("  printConfig - print config");
  Serial.println("  clearWifi - clear wifi config");
  Serial.println("  pin <pin> <value> - set pin value");
  Serial.println("  restart - restart ESP");
  Serial.println("  test - test function");
  Serial.println("  help - print this help");
  Serial.println("  list - list SPIFFS files");
  Serial.println("  read <filename> - read SPIFFS file content");
  Serial.println("  sleep <enable> - sleep mode");
}

// 修改低功耗模式
void LightSleep(bool enable) {
    // 串口作为唤醒源
    esp_sleep_enable_uart_wakeup();
    if (enable) {
        // 进入低功耗模式
        esp_light_sleep_start();
    } else {
        // 退出低功耗模式
        esp_light_sleep_wakeup();
    }
}
