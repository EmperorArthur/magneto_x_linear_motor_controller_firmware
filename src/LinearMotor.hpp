/**
 * SPDX-License-Identifier: MIT
 * SPDX-SnippetCopyrightText: 2025 Arthur Moore <Arthur.Moore.git@cd-net.net>
 * @file
 * @copyright Arthur Moore <Arthur.Moore.git@cd-net.net> 2025
 */

#pragma once
#include <ModbusRTUComm.h>

class LinearMotorStatus
{
public:
    uint16_t errorCode = 0;
    ModbusRTUCommError commError = MODBUS_RTU_COMM_SUCCESS;
    [[nodiscard]] bool isError() const
    {
        return commError || errorCode;
        //return commError || errorCode & 0xFF != 0x00 && (errorCode >> 8 & 0xFF) != 0x00;
    }
};

class LinearMotor {
public:
    LinearMotor(Stream& serial, uint8_t id);

    void begin();

    void disable();
    void enable();
    void setInertia(uint16_t value);
    void setCurentGain(uint8_t value);
    void setAutoGainOff();
    //void setAutoGainOn();
    void setFilter1Off();
    void setFilter2Off();

    void saveParam();

    std::optional<uint16_t> getInertia();
    std::optional<uint16_t> getCurrentGain();

    /**
     * @brief Determine if an error is present, and what the status is.
     * @return The error bytes if an error is present.  Otherwise, nothing.
     */
    LinearMotorStatus getStatus();

    /**
     * @brief Modbus Unit Identifier
     */
    const uint8_t id;

    /**
     * @brief Communication interface
     */
    ModbusRTUComm rtuComm;

private:
    /**
     * @brief Simple helper to allow sending const adus.
     * @param adu
     * @return The result of rtuComm.writeAdu
     */
    bool sendAdu(const ModbusADU &adu) {
        auto local = adu;
        return rtuComm.writeAdu(local);
    }

    static uint16_t aduToUint16(const ModbusADU &adu)
    {
        return static_cast<uint16_t>(adu.rtu[5] << 8 | adu.rtu[6]);
    }
};
