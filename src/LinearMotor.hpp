/**
 * SPDX-License-Identifier: MIT
 * SPDX-SnippetCopyrightText: 2025 Arthur Moore <Arthur.Moore.git@cd-net.net>
 * @file
 * @copyright Arthur Moore <Arthur.Moore.git@cd-net.net> 2025
 */

#pragma once
#include <ModbusRTUComm.h>
#include <ModbusRTUMaster.h>
#include <variant>

class LinearMotorStatus
{
public:
    uint16_t errorCode;
    ModbusRTUMasterError modbusError;
    [[nodiscard]] bool isError() const
    {
        return modbusError || errorCode;
        //return modbusError || errorCode & 0xFF != 0x00 && (errorCode >> 8 & 0xFF) != 0x00;
    }
};

class LinearMotor {
public:
    LinearMotor(HardwareSerial& serial, uint8_t id);

    /**
     * @copydoc HardwareSerial::begin
     */
    void begin(uint32_t baud, uint32_t config, int8_t rxPin, int8_t txPin);

    ModbusRTUMasterError disable();
    void enable();

    void setInertia(uint32_t value);
    std::variant<uint32_t, ModbusRTUMasterError> getInertia();

    /**
     * @brief Set the electrical gain.
     * @param value New value to set
     */
    void setCurrentGain(uint32_t value = 100);

    /**
     * @brief Get the current electrical gain.
     * @return The current electrical gain, or the error code.
     */
    std::variant<uint32_t, ModbusRTUMasterError> getCurrentGain();

    /**
     * @brief Enable or disable AutoGainTuning
     */
    void setAutoGain(bool enabled);
    std::variant<bool, ModbusRTUMasterError> getAutoGain();

    void setFilter1Off();
    void setFilter2Off();

    /**
     * @brief Save the active settings to permanent storage.
     */
    void persistToFlash();

    std::variant<int8_t, ModbusRTUMasterError> getModeOfOperation();
    std::variant<int32_t, ModbusRTUMasterError> getPositionActual();

    /**
     * @brief Determine if an error is present, and what the status is.
     * @return The error bytes if an error is present.  Otherwise, nothing.
     */
    LinearMotorStatus getStatus();

    [[nodiscard]] uint8_t getId() const
    {
        return id;
    }

    /**
     * @see ModBusRTUComm::writeAdu
     */
    bool writeAdu(ModbusADU& adu)
    {
        return rtuComm.writeAdu(adu);
    }

    /**
     * @see ModBusRTUComm::readAdu
     */
    ModbusRTUCommError readAdu(ModbusADU& adu)
    {
        return rtuComm.readAdu(adu);
    }

    /**
     * @brief Forward an ADU to a motor, adjusting the id & CRC as needed.
     * @details The caller is responsible for returning the response message to the sender.
     *          Response message may be an error if sending or receiving fail.
     * @param adu To forward.  Will be changed to the response message.
     * @return true if forwarding succeeded, otherwise false.
     */
    bool forwardAdu(ModbusADU& adu);

private:
    /**
     * @brief Modbus Unit Identifier
     */
    const uint8_t id;

    /**
     * @brief Underlying connection.
     */
    HardwareSerial& serial;

    /**
     * @brief Communication interface.
     * @details For manual adu handling.
     */
    ModbusRTUComm rtuComm;

    /**
     * @brief High level interface.
     */
    ModbusRTUMaster driver;

    ModbusRTUMasterError clearError();
    ModbusRTUMasterError sendEnableCommand();
};
