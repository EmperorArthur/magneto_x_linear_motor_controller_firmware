/**
 *@file
 *@brief Pre-defined commands which can be sent to the linear motors.
 */
#pragma once

/**
 * @brief Convert a std::array to an ADU.
 * @tparam n Array Length
 * @param raw RTU data to convert
 * @return A filled Modbus Application Data Unit (ADU)
 */
template<std::size_t n>
ModbusADU rawToADU(const std::array<uint8_t, n> &raw)
{
    auto adu = ModbusADU();
    adu.setLength(raw.size());
    std::copy(raw.begin(), raw.end(), adu.rtu);
    adu.updateCrc();
    return adu;
}

const ModbusADU current_mode = rawToADU<6>({0x01, 0x03, 0xF0, 0x0A, 0x00, 0x01});
const ModbusADU current_position = rawToADU<6>({0x01, 0x03, 0xF0, 0x10, 0x00, 0x02});

//设置自动增益开关,设置为关
const ModbusADU set_auto_gain_cmd = rawToADU<6>({0x01, 0x06, 0x04, 0x55, 0x00, 0x00});

//获取自动增益状态
const ModbusADU get_auto_gain_cmd = rawToADU<6>({0x01, 0x03, 0x04, 0x55, 0x00, 0x01});

//设置指令电流滤波器1 类型, 不滤波
const ModbusADU set_filter1_cmd = rawToADU<6>({0x01, 0x06, 0x04, 0x06, 0x00, 0x00});

//设置指令电流滤波器1 类型, 不滤波
const ModbusADU set_filter2_cmd = rawToADU<6>({0x01, 0x06, 0x04, 0x0B, 0x00, 0x00});

//获取指令电流滤波器1类型
const ModbusADU get_filter1_cmd = rawToADU<6>({0x01, 0x03, 0x04, 0x06, 0x00, 0x01});

//获取电机动子惯量
const ModbusADU get_motor_intertia_code_cmd = rawToADU<6>({0x01, 0x03, 0x00, 0x28, 0x00, 0x02});

//设置电机动子惯量
const ModbusADU set_motor_intertia_code_cmd = rawToADU<6>({0x01, 0x06, 0x00, 0x28, 0x00, 0xE6});


//获取电气增益值
const ModbusADU get_motor_current_gain_code_cmd = rawToADU<6>({0x01, 0x03, 0x00, 0x18, 0x00, 0x02});
//返回0x01, 0x03, 0x04, 0x00, 0x00, 0x00, 0x64, 0xFB, 0xD8，0x64=100

//设置电气增益值，100
const ModbusADU set_motor_current_gain_code_cmd = rawToADU<6>({0x01, 0x06, 0x00, 0x18, 0x00, 0x64});

//保存参数
const ModbusADU save_param_code_cmd = rawToADU<6>({0x01, 0x06, 0x60, 0x00, 0x00, 0x01});

//chack save ok
const ModbusADU check_save_param_code_cmd = rawToADU<6>({0x01, 0x03, 0x01, 0x8A, 0x00, 0x01});

const ModbusADU get_motor_error_code_cmd = rawToADU<6>({0x01, 0x03, 0xF0, 0x01, 0x00, 0x02});
const ModbusADU enable_motor_cmd = rawToADU<6>({0x01, 0x06, 0xF0, 0x02, 0x00, 0x0F});
const ModbusADU disable_motor_cmd = rawToADU<6>({0x01, 0x06, 0xF0, 0x02, 0x00, 0x06});
const ModbusADU clean_error_cmd = rawToADU<6>({0x01, 0x06, 0xF0, 0x02, 0x00, 0x80});
