#include <string>
#include <vector>
#include <stdint.h>

std::vector<uint8_t> bytesStringToBytes(std::string bytesString){
    std::vector<uint8_t> bytes;
    // 去掉首尾的[]
    bytesString = bytesString.substr(1, bytesString.length() - 2);
    // 以,为分隔符
    size_t pos = 0;
    std::string token;
    while ((pos = bytesString.find(',')) != std::string::npos) {
        token = bytesString.substr(0, pos);
        bytes.push_back(atoi(token.c_str()));
        bytesString.erase(0, pos + 1);
    }
    if(!bytesString.empty()){
        bytes.push_back(atoi(bytesString.c_str()));
    }
    return bytes;
}
std::string BytesToBytesString(std::vector<uint8_t> bytes){
    std::string bytesString = "[";
    for (size_t i = 0; i < bytes.size(); i++) {
        bytesString += std::to_string(bytes[i]);
        if(i != bytes.size() - 1){
            bytesString += ", ";
        }
    }
    bytesString += "]";
    return bytesString;
}

// 从字符串里面解析出命令，提供输入内容，返回命令和参数数组
// 格式为 commandName arg1 arg2 arg3
// ParseCommand 解析命令
void ParseCommand(std::string lineBuf, std::string &command, std::vector<std::string> &args){
    size_t pos = 0;
    std::string token;
    bool inQuotes = false;
    for (size_t i = 0; i < lineBuf.size(); ++i) {
        if (lineBuf[i] == '\"') {
            inQuotes = !inQuotes;
        } else if (lineBuf[i] == ' ' && !inQuotes) {
            token = lineBuf.substr(pos, i - pos);
            if (!token.empty() && token.front() == '\"' && token.back() == '\"') {
                token = token.substr(1, token.size() - 2);  // 去掉两端的双引号
            }
            if (command.empty()) {
                command = token;
            } else {
                args.push_back(token);
            }
            pos = i + 1;
        }
    }
    token = lineBuf.substr(pos);
    if (!token.empty()) {
        if (token.front() == '\"' && token.back() == '\"') {
            token = token.substr(1, token.size() - 2);  // 去掉两端的双引号
        }
        if (command.empty()) {
            command = token;
        } else {
            args.push_back(token);
        }
    }
}

void dispatchCommand(std::string command, std::vector<std::string> args){
    if (command == "wifi") {
        if (args.size() != 2) {
            Serial.println("Invalid wifi command");
            return;
        }
        std::string ssid = args[0];
        std::string password = args[1];
        if (onWifi(ssid, password)) {
            setConfigWifi(ssid, password);
            wifiConnected = true;
            wifiConnectAfter();
        }
    } else if (command == "server") {
        if (args.size() != 2) {
            Serial.println("Invalid server command");
            return;
        }
        setConfigServer(args[0], args[1]);
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
        std::string value = getConfigString(args[0]);
        Serial.println(value.c_str());
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
    } else if (command == "lightSleep") {
        if (args.size() != 1) {
            Serial.println("Invalid lightSleep command");
            return;
        }
        ESPLightSleep(atoi(args[0].c_str()));
    } else if (command == "deepSleep") {
        if (args.size() != 1) {
            Serial.println("Invalid deepSleep command");
            return;
        }
        ESPDeepSleep(atoi(args[0].c_str()));
    } else if (command == "reboot") {
        ESP.restart();
    } else if (command == "connect") {
        wifiConnectAfter();
    } else if (command == "test") {
        Test();
    } else if (command == "help") {
        PrintHelp();
    } else if (command == "ls") {
        PrintSPIFFSFileList();
    } else if (command == "write") {
        if (args.size() != 2) {
            Serial.println("Invalid write command");
            return;
        }
        WriteSPIFFSFile(args[0], args[1]);
    } else if (command == "cat") {
        if (args.size() != 1) {
            Serial.println("Invalid read command");
            return;
        }
        PrintSPIFFSFileContent(args[0]);
    } else if (command == "rm") {
        if (args.size() != 1) {
            Serial.println("Invalid delete command");
            return;
        }
        DeleteSPIFFSFile(args[0]);
    } else if (command == "send") {
        if (args.size() != 2) {
            Serial.println("Invalid send command");
            return;
        }
        // 发送gatt数据
        // send <bleMac> <bytes str({255, 255})>
        std::string bleMac = args[0];
        std::vector<uint8_t> bytes = bytesStringToBytes(args[1]);
        if (GattClientConnect(bleMac)) {
            if (GattClientWrite(bytes)) {
                std::vector<uint8_t> response;
                if (GattClientRead(&response)) {
                    Serial.printf("Response: %s\n", BytesToBytesString(response).c_str());
                }
            }
        }
    } else {
        Serial.printf("Unknown command [%s], type \"help\" list all commands\n", command.c_str());
    }
}
