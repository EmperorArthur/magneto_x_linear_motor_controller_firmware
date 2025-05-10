/**
 * SPDX-License-Identifier: MIT
 * SPDX-SnippetCopyrightText: 2025 Arthur Moore <Arthur.Moore.git@cd-net.net>
 * @file
 * @copyright Arthur Moore <Arthur.Moore.git@cd-net.net> 2025
 */

#include "LinearMotor.hpp"

LinearMotor::LinearMotor(HardwareSerial& serial, const uint8_t id):
    id{id},
    serial{serial},
    rtuComm(serial),
    driver(serial)
{
}

void LinearMotor::begin(const uint32_t baud, const uint32_t config, const int8_t rxPin, const int8_t txPin)
{
    serial.setRxBufferSize(sizeof(ModbusADU));
    serial.setTxBufferSize(sizeof(ModbusADU));
    serial.begin(baud, config, rxPin, txPin);
    rtuComm.begin(baud, config);
    driver.begin(baud, config);
}

ModbusRTUMasterError LinearMotor::disable()
{
    //"Controlword" register  (UNS16) Read Write
    return driver.writeSingleHoldingRegister(id, 0xF002, 0x06);
}

void LinearMotor::enable()
{
    disable();
    delay(100);
    clearError();
    delay(100);
    disable();  // Needs to be sent here if an error was cleared.
    delay(100);
    sendEnableCommand();
}

void LinearMotor::setInertia(const uint32_t value)
{
    disable();
    delay(100);
    // "Inertia" register (UNS32) Read Write
    driver.writeSingleHoldingRegister(id, 0x0028, value);
    delay(100);
    persistToFlash();
    delay(100);
    enable();
}

void LinearMotor::setCurrentGain(const uint32_t value)
{
    disable();
    delay(100);
    // "CurrentBandwidth" register (UNS32) Read Write
    driver.writeSingleHoldingRegister(id, 0x0018, value);
    delay(100);
    persistToFlash();
    delay(100);
    enable();
}

void LinearMotor::setAutoGain(const bool enabled)
{
    disable();
    delay(100);
    // "AutoGainTuningEnable" register (UNS8) Read Write
    driver.writeSingleHoldingRegister(id, 0x0455, enabled);
    delay(100);
    persistToFlash();
    delay(100);
    enable();
}

std::variant<bool, ModbusRTUMasterError> LinearMotor::getAutoGain()
{
    uint16_t value = -1;
    //"AutoGainTuningEnable" register (UNS8) Read Write
    const auto result = driver.readHoldingRegisters(
        1, 0x0455, &value, 1
        );
    if (result)
    {
        return result;
    }
    return value;
}

void LinearMotor::setFilter1Off()
{
    disable();
    delay(100);
    // "CurrentTargetFilter1Type" register (UNS8) Read Write
    driver.writeSingleHoldingRegister(id, 0x0406, 0x00);
    delay(100);
    persistToFlash();
    delay(100);
    enable();
}

void LinearMotor::setFilter2Off()
{
    disable();
    delay(100);
    // "CurrentTargetFilter2Type" register (UNS8) Read Write
    driver.writeSingleHoldingRegister(id, 0x040B, 0x00);
    delay(100);
    persistToFlash();
    delay(100);
    enable();
}

void LinearMotor::persistToFlash()
{
    // "ControlCmd" register (UNS8)
    driver.writeSingleHoldingRegister(id, 0x6000, 0x01);
    delay(100);

    // Check if save worked.
    uint16_t value = -1;
    // "FlashStorageStatus" register (UNS8) Read Only
    const auto result = driver.readHoldingRegisters(
        1, 0x018A, &value, 1
        );

    if (result || value)
    {
        // Not sure what success is for "value"
        // Do something
    }
}

std::variant<int8_t, ModbusRTUMasterError> LinearMotor::getModeOfOperation()
{
    uint16_t value;
    // "Modes_of_operation_display" register (INTEGER8) Read Only
    const auto result = driver.readHoldingRegisters(
        1, 0xF00A, &value, 1
        );
    if (result)
    {
        return result;
    }
    return value & 0xFF;
}

std::variant<int32_t, ModbusRTUMasterError> LinearMotor::getPositionActual()
{
    int32_t value;
    // "Position_actual_value" register (INTEGER32) Read Only
    const auto result = driver.readHoldingRegisters(
        1, 0xF010, reinterpret_cast<uint16_t*>(&value), 2
        );
    if (result)
    {
        return result;
    }
    return value;
}

std::variant<uint32_t,ModbusRTUMasterError> LinearMotor::getInertia()
{
    uint32_t value;
    // "Inertia" register (UNS32) Read Write
    const auto result = driver.readHoldingRegisters(
        1, 0x0028, reinterpret_cast<uint16_t*>(&value), 2
        );
    if (result)
    {
        return result;
    }
    return value;
}

std::variant<uint32_t,ModbusRTUMasterError> LinearMotor::getCurrentGain()
{
    uint32_t value;
    // "CurrentBandwidth" register (UNS32) Read Write
    const auto result = driver.readHoldingRegisters(
        1, 0x0018, reinterpret_cast<uint16_t*>(&value), 2
        );
    if (result)
    {
        return result;
    }
    return value;
}

//test1: 01 03 f0 0a 00 01 97 08
//test2: 01 06 f0 0a 00 03 da c9
LinearMotorStatus LinearMotor::getStatus()
{
    uint16_t value = -1;
    // "Error_code" register (UNS16) Read Only
    const auto result = driver.readHoldingRegisters(
        1, 0xF001, &value, 1
        );
    return {value, result};
}

ModbusRTUMasterError LinearMotor::clearError()
{
    //"Controlword" register (UNS16) Read Write
    return driver.writeSingleHoldingRegister(id, 0xF002, 0x80);
}

ModbusRTUMasterError LinearMotor::sendEnableCommand()
{
    //"Controlword" register (UNS16) Read Write
    return driver.writeSingleHoldingRegister(id, 0xF002, 0x0F);
}
