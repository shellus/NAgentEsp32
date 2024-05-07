
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
