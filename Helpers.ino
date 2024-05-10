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

void DeleteSPIFFSFile(String filename) {
  if (SPIFFS.remove(filename)) {
    Serial.println("  File [" + filename + "] removed");
  } else {
    Serial.println("  ERROR: Failed to remove file [" + filename + "]");
  }
}

uint32_t byteUInt32(byte buffer[4]) {
  // 将缓冲区中的四个字节按小端序转换成uint32_t
  uint32_t result = (uint32_t)buffer[0] |
                    ((uint32_t)buffer[1] << 8) |
                    ((uint32_t)buffer[2] << 16) |
                    ((uint32_t)buffer[3] << 24);

  return result;
}

// 生成a-z, 0-9的随机字符串
String getRandString(uint32_t length) {
  String result = "";
  for (uint32_t i = 0; i < length; ++i) {
    char c;
    if (random(0, 2) == 0) {
      c = random(0, 26) + 'a';
    } else {
      c = random(0, 10) + '0';
    }
    result += c;
  }
  return result;
}
// 生成uuid字符串
String getUUID() {
  String result = "";
  for (uint32_t i = 0; i < 32; ++i) {
    char c;
    if (i == 8 || i == 12 || i == 16 || i == 20) {
      c = '-';
    } else {
      c = random(0, 16);
      if (c < 10) {
        c += '0';
      } else {
        c += 'a' - 10;
      }
    }
    result += c;
  }
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
    Serial.println("  delete <filename> - delete SPIFFS file");
    Serial.println("  lightSleep <seconds> - light sleep for <seconds> seconds");
    Serial.println("  deepSleep <seconds> - deep sleep for <seconds> seconds");
}

void ESPLightSleep(uint32_t sleep_seconds) {
    // 实际测试结果：Light-sleep 会丢失TCP连接，但是不会丢失WiFi连接

// 串口唤醒不管用，不知道是不是因为没设置字符数
//    esp_err_t err = esp_sleep_enable_uart_wakeup(0);
//    if (err != ESP_OK) {
//        Serial.println("esp_sleep_enable_uart_wakeup err: " + String(err));
//        return;
//    }

    // 定时器的精度是微妙：也就是毫秒*1000
    esp_sleep_enable_timer_wakeup(sleep_seconds * 1000000);

    Serial.printf("Light-sleep [%d]seconds in:%d seconds\n", sleep_seconds, millis() / 1000);
    Serial.flush();

    esp_light_sleep_start();
    // 唤醒后会从此处继续执行，所以下面一句叫"Light sleep wakeup"
    Serial.printf("Light-sleep wakeup in: %d seconds\n", millis() / 1000);

    // 经过上面的代码得到结论，light sleep 模式下，millis() 会继续计时
}

RTC_DATA_ATTR bool isDeepSleep = false;
RTC_DATA_ATTR int incDeepSleep = 0;

void ESPDeepSleep(uint32_t sleep_seconds) {

    // 定时器的精度是微妙：也就是毫秒*1000
    esp_sleep_enable_timer_wakeup(sleep_seconds * 1000000);

    Serial.printf("Deep-sleep [%d]seconds in:%d seconds\n", sleep_seconds, millis() / 1000);
    Serial.flush();

    isDeepSleep = true;
    ++incDeepSleep;

    esp_deep_sleep_start();
    // 下一次会从setup()开始执行，所以下面永远不会执行
    Serial.printf("Deep-sleep wakeup in: %d seconds\n", millis() / 1000);
}
