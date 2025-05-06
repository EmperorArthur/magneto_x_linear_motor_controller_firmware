#ifndef MODEBUSSENDER_H
#define MODEBUSSENDER_H

#include <Arduino.h>


extern void sendModebusCmd(uint8_t id, unsigned char *tx_buffer, int len);
extern uint16_t crc16 (const uint8_t *nData, uint16_t wLength);
extern HardwareSerial portxSerial;

class ModeBusSender
{
public: 
    ModeBusSender();
    void modebusInit();
    

};

#endif