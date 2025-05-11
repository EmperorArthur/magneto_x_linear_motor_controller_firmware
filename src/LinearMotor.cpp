/**
 * SPDX-License-Identifier: MIT
 * SPDX-SnippetCopyrightText: 2025 Arthur Moore <Arthur.Moore.git@cd-net.net>
 * @file
 * @copyright Arthur Moore <Arthur.Moore.git@cd-net.net> 2025
 */

#include "LinearMotor.hpp"
#include "ModbusDefinitions.hpp"

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
    rtuComm.setTimeout(500); // This is what ModbusRTUMaster does
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

void LinearMotor::setInertia(uint32_t value)
{
    std::array<uint16_t, 2> raw =
        {static_cast<uint16_t>(value >> 16),static_cast<uint16_t>(value & 0xFFFF)};

    disable();
    delay(100);
    // "Inertia" register (UNS32) Read Write
    driver.writeMultipleHoldingRegisters(id, 0x0028, raw.data(), raw.size());
    persistToFlash();
    delay(100);
    enable();
}

void LinearMotor::setCurrentGain(uint32_t value)
{
    std::array<uint16_t, 2> raw =
        {static_cast<uint16_t>(value >> 16),static_cast<uint16_t>(value & 0xFFFF)};

    disable();
    delay(100);
    // "CurrentBandwidth" register (UNS32) Read Write
    driver.writeMultipleHoldingRegisters(id, 0x0018, raw.data(), raw.size());
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
    const auto result =
        driver.readHoldingRegisters(id, 0x0455, &value, 1);
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
    uint16_t value = 0;
    // "FlashStorageStatus" register (UNS8) Read Only
    const auto result =
        driver.readHoldingRegisters(id, 0x018A, &value, 1);

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
    const auto result =
        driver.readHoldingRegisters(id, 0xF00A, &value, 1);
    if (result)
    {
        return result;
    }
    return value & 0xFF;
}

std::variant<int32_t, ModbusRTUMasterError> LinearMotor::getPositionActual()
{
    std::array<uint16_t, 2> value = {};
    // "Position_actual_value" register (INTEGER32) Read Only
    const auto result =
        driver.readHoldingRegisters(id, 0xF010, value.data(), value.size());
    if (result)
    {
        return result;
    }
    return value[0] << 16 | value[1];
}

std::variant<uint32_t,ModbusRTUMasterError> LinearMotor::getInertia()
{
    std::array<uint16_t, 2> value = {};
    // "Inertia" register (UNS32) Read Write
    const auto result =
        driver.readHoldingRegisters(id, 0x0028, value.data(), value.size());
    if (result)
    {
        return result;
    }
    return value[0] << 16 | value[1];
}

std::variant<uint32_t,ModbusRTUMasterError> LinearMotor::getCurrentGain()
{
    std::array<uint16_t, 2> value = {};
    // "CurrentBandwidth" register (UNS32) Read Write
    const auto result =
        driver.readHoldingRegisters(id, 0x0018, value.data(), value.size());
    if (result)
    {
        return result;
    }
    return value[0] << 16 | value[1];
}

//test1: 01 03 f0 0a 00 01 97 08
//test2: 01 06 f0 0a 00 03 da c9
LinearMotorStatus LinearMotor::getStatus()
{
    uint16_t value = -1;
    // "Error_code" register (UNS16) Read Only
    const auto result =
        driver.readHoldingRegisters(id, 0xF001, &value, 1);
    return {value, result};
}

bool LinearMotor::forwardAdu(ModbusADU& adu)
{
    const auto originalId = adu.getUnitId();

    adu.setUnitId(id);
    writeAdu(adu);
    const auto readStatus = readAdu(adu);
    if (readStatus)
    {
        adu.setUnitId(originalId);
        //adu.prepareExceptionResponse(readStatus);
        adu.prepareExceptionResponse(GATEWAY_TARGET_DEVICE_FAILED_TO_RESPOND);
        return false;
    }
    adu.setUnitId(originalId);
    return true;
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
