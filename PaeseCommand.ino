
// 从字符串里面解析出命令，提供输入内容，返回命令和参数数组
// 格式为 commandName arg1 arg2 arg3
bool ParseCommand(String input, String* command, std::vector<String>& args) {
    // 按照Linux规则，开头结尾的空格都是可以忽略的
    input.trim();
    // 如果是空字符串
    if (input.length() == 0) {
        return false;
    }
    int colonIndex = input.indexOf(' ');

    // 如果没有空格，那么就是命令，没有参数
    if (colonIndex == -1) {
        *command = input;
        return true;
    }
    *command = input.substring(0, colonIndex);
    String argString = input.substring(colonIndex + 1);
    while (argString.length() > 0) {
        int commaIndex = argString.indexOf(' ');
        if (commaIndex == -1) {
          args.push_back(argString);
          break;
        }
        args.push_back(argString.substring(0, commaIndex));
        argString = argString.substring(commaIndex + 1);
    }
    return true;
}
