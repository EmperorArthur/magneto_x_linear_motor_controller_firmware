/**
 * SPDX-License-Identifier: MIT
 * SPDX-SnippetCopyrightText: 2025 Arthur Moore <Arthur.Moore.git@cd-net.net>
 * @file
 * @copyright Arthur Moore <Arthur.Moore.git@cd-net.net> 2025
 */

#include "LinearMotor.hpp"
#include "LinearMotorCommands.hpp"

LinearMotor::LinearMotor(Stream& serial, uint8_t id):
    id{id},
    rtuComm(serial)
{
}

void LinearMotor::begin()
{

}

void LinearMotor::disable()
{
    sendAdu(disable_motor_cmd);
}

void LinearMotor::enable()
{
    sendAdu(disable_motor_cmd);
    delay(100);
    sendAdu(clean_error_cmd);
    delay(100);
    sendAdu(disable_motor_cmd);  // Needs to be sent here if an error was cleared.
    delay(100);
    sendAdu(enable_motor_cmd);
}

void LinearMotor::setInertia(uint16_t value)
{
    auto command = set_motor_intertia_code_cmd;
    command.rtu[5] = lowByte(value);
    command.rtu[4] = highByte(value);
    command.updateCrc();

    disable();
    delay(100);
    rtuComm.writeAdu(command);
    delay(100);
    saveParam();
    delay(100);
    enable();
}

void LinearMotor::setCurentGain(uint8_t value)
{
    auto command = set_motor_current_gain_code_cmd;
    command.rtu[5] = value;

    disable();
    delay(100);
    rtuComm.writeAdu(command);
    delay(100);
    saveParam();
    delay(100);
    enable();
}

void LinearMotor::setAutoGainOff()
{
    disable();
    delay(100);
    sendAdu(set_auto_gain_cmd);
    delay(100);
    saveParam();
    delay(100);
    enable();
}

void LinearMotor::setFilter1Off()
{
    disable();
    delay(100);
    sendAdu(set_filter1_cmd);
    delay(100);
    saveParam();
    delay(100);
    enable();
}

void LinearMotor::setFilter2Off()
{
    disable();
    delay(100);
    sendAdu(set_filter2_cmd);
    delay(100);
    saveParam();
    delay(100);
    enable();
}

void LinearMotor::saveParam()
{
    sendAdu(save_param_code_cmd);
    delay(100);
    sendAdu(check_save_param_code_cmd);
}

std::optional<uint16_t> LinearMotor::getInertia()
{
    sendAdu(get_motor_intertia_code_cmd);
    auto adu = ModbusADU();
    delay(100);
    const auto readStatus = rtuComm.readAdu(adu);
    if (readStatus)
    {
        return {};
    }
    return aduToUint16(adu);
}

std::optional<uint16_t> LinearMotor::getCurrentGain()
{
    sendAdu(get_motor_current_gain_code_cmd);
    auto adu = ModbusADU();
    delay(100);
    const auto readStatus = rtuComm.readAdu(adu);
    if (readStatus)
    {
        return {};
    }
    return aduToUint16(adu);
}

//test1: 01 03 f0 0a 00 01 97 08
//test2: 01 06 f0 0a 00 03 da c9
LinearMotorStatus LinearMotor::getStatus()
{
    sendAdu(get_motor_error_code_cmd);
    delay(100);

    auto adu = ModbusADU();
    const auto readStatus = rtuComm.readAdu(adu);

    if (readStatus)
    {
        return {0, readStatus};
    }

    return {aduToUint16(adu), readStatus};
}
