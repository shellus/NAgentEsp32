# NStartup Agent
基于esp32 arduino开发的NStartup客户端，用于远程网络唤醒管理的程序，它可以通过局域网内的广播包唤醒指定的计算机，也可以通过远程服务器唤醒指定的计算机，以及远程执行预定义动作。

详细介绍请查看 [shellus/NStartup: 一个用于远程网络唤醒管理的程序](https://github.com/shellus/NStartup)

<img src="https://github.com/shellus/NAgentEsp32/blob/master/.files/224006.png?raw=true">

## 硬件选择
- 立创开发板ESP32S3-R8N8
  - 选择 ESP32S3 Dev Module

- LuatOS ESP32C3开发板
  - 选择 AirM2M CORE ESP32C3
  - 简约款应该也可以CDC模式烧录（未测试） 
  - 如果选择 “ESP32C3 Dev Module” 的话需要确认  “Flush Mode” 为DIO，否则会无限重启或串口无输出
  - https://wiki.luatos.com/chips/esp32c3/board.html
  - LED为12、13，BOOT按钮为9，RST非GPIO
  - ESP32C芯片无内置Flush，

- ESP-WROOM-32
  - 选择 ESP32 Dev Module
