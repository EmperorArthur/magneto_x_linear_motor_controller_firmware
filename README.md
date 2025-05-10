# Peopoly Magneto X Linear Motor Control Board Firmware

![Annotated board photo](./docs/board_photo_annotated.jpg)

This controls two Motion G 2x MotionG "DN1-G60xxN" boards.
Each board is on its own interface, and both boards have a Modbus address of 1.

按键功能:

Disable: 失能两个电机
Enable: 清除错误，和重新使能两个电机

LED灯效:

X轴: 当有错误时，灯是红色，正常时，灯是绿色
Y轴: 当有错误时，灯是红色，正常时，灯是绿色

# Hardware Information
System On Chip: ESP32-WROOM-32D

## Buttons and Interfaces
* Disable Button
* Enable Button
* X Motor Status Red/Green LED
* Y Motor Status Red/Green LED
* USB-C Connector
* JST USB Connector. Shares pins with USB-C Connector
* X Motor RS485 Connector
* Y Motor RS485 Connector
* 3 pin JST Connector to BTT Octopus Pro Board

# Common Linear Motor Error Codes
| error code |                                                       reason                                                      |                                                     how to solve                                                     |   |   |
|:----------:|:-----------------------------------------------------------------------------------------------------------------:|:--------------------------------------------------------------------------------------------------------------------:|---|---|
| 0x33,0x31  | There is poor contact or disconnection in the power lines U, V, and W of the motor.                               | Check whether the motor power cable is properly connected                                                            |   |   |
| 0x32,0x30  | The power supply voltage of the driver is insufficient and below the minimum value of the hardware voltage input. | enable motor again                                                                                                   |   |   |
| 0xFF,0x07  | Hardware overcurrent caused an error in the DRV nFault pin.                                                       | Check whether the power output line of the motor is short-circuited between phases, or short-circuited to the ground |   |   |
| 0x84,0x00  | The protection is activated when the maximum velocity limit switch is turned on.                                  | reduce speed                                                                                                         |   |   |
| 0x86,0x11  | position control error exceeds the tracking error window                                                          | Check whether cable connections are correct 2.Ensure that the motor power is appropriate                             |   |   |

# External Documentation
* [ESP32-WROOM-32D](https://www.espressif.com/sites/default/files/documentation/esp32-wroom-32d_esp32-wroom-32u_datasheet_en.pdf)
* [ESP32 Datasheet](https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_en.pdf)
* [MODBUS APPLICATION PROTOCOL SPECIFICATION](https://modbus.org/docs/Modbus_Application_Protocol_V1_1b3.pdf)

## Peopoly
* [Overview of the Magneto X Electronic Hardware System](https://wiki.peopoly.net/en/magneto/magneto-x/magneto-x-electronic-system)
* [Guide to Capturing Linear Motor Error Codes](https://wiki.peopoly.net/en/magneto/magneto-x/get-error-code-in-touchscreen)
* [Linear Motor Parameters](https://wiki.peopoly.net/en/magneto/magneto-x/parameters-introduce)
* [LinearMotorHost User Guide](https://wiki.peopoly.net/en/magneto/magneto-x/linearmotorhost-user-guide)
* 

## Motion G
* [Hardware User Manual -> DN1-G60xxN  General Control Mode User Manual](https://motiong.feishu.cn/wiki/BSI8w4HKSi02MmkHoTScPmh1nub)
* [Software User Manual -> DN1-G60xxN  General Control Mode Software Guidance](https://motiong.feishu.cn/wiki/R4E0wo3eFigeNsk3YeYcp9C7nkh)
* [Software User Manual -> Upper computer debugging software user manua](https://motiong.feishu.cn/wiki/UKA9wAqvIiimYokaIEFctnPgntf)
