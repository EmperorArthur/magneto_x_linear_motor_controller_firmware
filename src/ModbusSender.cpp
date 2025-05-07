/**
 *@file
 */
#include <cstring>
#include "ModbusSender.h"
#include "ModbusADU.h"

//test1: 01 03 f0 0a 00 01 97 08
//test2: 01 06 f0 0a 00 03 da c9
void sendModbusCommand(uint8_t id, const uint8_t *tx_buffer, size_t len)
{
    auto adu = ModbusADU();
    adu.setLength(len);
    std::memcpy(adu.rtu, tx_buffer, len);
    adu.updateCrc();

    if(id==0)
    {
        //clear all buffer
        while (Serial1.available() > 0) 
        {
          Serial1.read();
        }
        Serial1.write(adu.rtu, adu.getRtuLen());
    }
    else if(id==1)
    {
        //clear all buffer
        while (Serial2.available() > 0) 
        {
          Serial2.read();
        }
        Serial2.write(adu.rtu, adu.getRtuLen());
    }
}
