#pragma once

#include <Arduino.h>


extern void sendModbusCommand(uint8_t id, const uint8_t *tx_buffer, int len);
extern uint16_t crc16 (const uint8_t *nData, uint16_t wLength);
extern HardwareSerial portxSerial;

class ModbusSender
{
public: 
    ModbusSender();
};
