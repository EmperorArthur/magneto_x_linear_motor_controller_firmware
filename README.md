# Peopoly Magneto X Linear Motor Control Board Firmware

按键功能:

Disable: 失能两个电机
Enable: 清除错误，和重新使能两个电机

LED灯效:

X轴: 当有错误时，灯是红色，正常时，灯是绿色
Y轴: 当有错误时，灯是红色，正常时，灯是绿色

# Hardware Information
System On Chip: ESP32-WROOM-32D

This controls two Motion G 2x MotionG "DN1-G60xxN" boards.
Each board is on its own interface, and both boards have a Modbus address of 1.

# External Documentation
https://www.espressif.com/sites/default/files/documentation/esp32-wroom-32d_esp32-wroom-32u_datasheet_en.pdf
https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_en.pdf
https://modbus.org/docs/Modbus_Application_Protocol_V1_1b3.pdf

## Motion G
* [Hardware User Manual -> DN1-G60xxN  General Control Mode User Manual](https://motiong.feishu.cn/wiki/BSI8w4HKSi02MmkHoTScPmh1nub)
* [Software User Manual -> DN1-G60xxN  General Control Mode Software Guidance](https://motiong.feishu.cn/wiki/R4E0wo3eFigeNsk3YeYcp9C7nkh)
* [Software User Manual -> Upper computer debugging software user manua](https://motiong.feishu.cn/wiki/UKA9wAqvIiimYokaIEFctnPgntf)
