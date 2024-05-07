
void Test() {
    TestParseCommand("commandName");
    TestParseCommand("commandName");
    TestParseCommand("commandName:arg1");
    TestParseCommand("commandName:arg1,arg2,arg3,");
}

void TestParseCommand(String input) {
    String command;
    std::vector<String> args;
    if (ParseCommand(input, &command, args)) {
        Serial.print("Command: ");
        Serial.println(command);
        Serial.print("Args: ");
        for (int i = 0; i < args.size(); i++) {
            Serial.print(args[i]);
            Serial.print(" ");
        }
        Serial.println();
    } else {
        Serial.println("Failed to parse command");
    }
}
