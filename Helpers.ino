void PrintSPIFFSFileList() {
  Serial.println("SPIFFS File List:");
  File root = SPIFFS.open("/");
  File file = root.openNextFile();
  while (file) {
    Serial.println("  /" + String(file.name()));
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

void WriteSPIFFSFile(String filename, String content) {
  File file = SPIFFS.open(filename, "w");
  if (!file) {
    Serial.println("  ERROR: Failed to open file for writing");
    return;
  }
  file.print(content);
  file.close();
  Serial.println("  File [" + filename + "] saved");
}

uint32_t byteUInt32(byte buffer[4]) {
  // 将缓冲区中的四个字节按小端序转换成uint32_t
  uint32_t result = (uint32_t)buffer[0] |
                    ((uint32_t)buffer[1] << 8) |
                    ((uint32_t)buffer[2] << 16) |
                    ((uint32_t)buffer[3] << 24);

  return result;
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
  Serial.println("  write <filename> <content> - write SPIFFS file content");
  Serial.println("  sleep - enter light-sleep mode");
}

// 修改低功耗模式
void LightSleep() {

    esp_err_t err = esp_sleep_enable_uart_wakeup(0);
    if (err != ESP_OK) {
        Serial.println("esp_sleep_enable_uart_wakeup err: " + String(err));
        return;
    }

    // 唤醒时间秒数变量
    uint64_t wakeup_time_sec = 10;
    esp_sleep_enable_timer_wakeup(wakeup_time_sec * 1000000);

    Serial.println("Light sleep start in " + String(millis() / 1000) + "s");
    //    delay(100); 调用 flush 就不需要 delay 了
    Serial.flush();
    esp_light_sleep_start();
    // 唤醒后会从此处继续执行，所以下面一句叫"Light sleep wakeup"
    Serial.println("Light sleep wakeup in: " + String(millis() / 1000) + "s");

    // 经过上面的代码得到结论，light sleep 模式下，millis() 会继续计时
}
