/**
 *@file
 */
#include <cstring>
#include <Arduino.h>
#include <ModbusADU.h>
#include <ModbusSlaveLogic.h>

#include "ModbusDefinitions.hpp"
#include "Button.hpp"
#include "LinearMotor.hpp"
#include "RGLed.hpp"

enum OperatingMode : uint16_t
{
    /**
     * @brief ASCII Serial mode with UX features.
     * @details Buttons and LEDs work automatically.
     */
    ASCII = 0,

    /**
     * @brief Pure RTU Gateway mode.
     * @details Buttons and LEDS must be handled by Modbus master.
     */
    RTU_GATEWAY = 1
};

///@brief For when in RTU Mode
ModbusRTUComm* HostComm;
auto RTUSlaveLogic = ModbusSlaveLogic();
std::array<uint16_t, 3> holdingRegisters = {};
std::array<bool, 2> discreteInputs = {};
bool motorError = true;

LinearMotor* XMotor;
LinearMotor* YMotor;

auto & XMotorSerial = Serial1;
auto & YMotorSerial = Serial2;

auto XLed = RGLed(19, 21);
auto YLed = RGLed(5, 18);

auto EnableButton = Button(15, 1000);
auto DisableButton = Button(4, 1000);

void clearIncomingData(Stream & stream);
void disableBothMotors();
void enableBothMotors();

void processPureData();
void executeRtuGatewayLogic();
void sendCmdByPort(const String &cmd);

#define VERSION "1.0.7-git"

bool recvl_ok = false;
String inData="";
OperatingMode mode = ASCII;

#define MODBUS_BAUD 115200
#define EMERGE_STOP_PIN 14 //stop klipper when error occur

/**
 * @brief Print a value in the format 0xFF
 */
void printHex(const uint8_t value)
{
    Serial.print("0x");
    if (value < 16) 
    {
        Serial.print("0");
    }
    Serial.print(value, HEX);
}

/**
 * @brief Print data in the format "0xFF, 0xFF,"
 * @param hex_data Data to print
 * @param len Length of the data
 */
void printHexArray(const uint8_t hex_data[], const size_t len)
{
    for(size_t i=0; i<len; i++)
    {
        printHex(hex_data[i]);
        Serial.print(",");
    }
    Serial.print("\n");
}

/**
 * @brief print anything as raw hex data.
 * @tparam T type of the value to print
 * @param value value to print
 */
template <typename T>
void printHex(T value)
{
    printHexArray(reinterpret_cast<const uint8_t*>(&value), sizeof(value));
}

void readCmd()
{
  //读取所有的数据
  while (Serial.available() > 0 && recvl_ok == false)
  {
    const char recieved = Serial.read();
    if (recieved == '\n')
    {
      recvl_ok = true;
    }
    inData += recieved;
  }
  if(recvl_ok)
  {
    sendCmdByPort(inData);  
    recvl_ok= false;
      inData = "";
  }
}

/**
 * @brief Query the motor driver for errors.
 * @param motor The motor to query.
 * @param axisName Used for reporting errors
 * @return true if there is an error.  Otherwise, false.
 */
bool checkForError(LinearMotor &motor, const String &axisName)
{
    const auto status = motor.getStatus();
    if (status.modbusError)
    {
        Serial.println(axisName + " axis error: Communication Error");
        return true;
    }

    if (status.isError())
    {
        Serial.print(axisName + " axis error:");
        printHex(status.errorCode);
        return true;
    }
    return false;
}

/**
 * @brief Send a raw Modbus command to a motor, and display the response.
 * @details Commands are in the format "##1,2,3,4,5,6".
 *          The following retrieves X axis position: "##1,3,240,16,0,2"
 * @param cmds String of comma separated ints.  Preceded by two characters.
 * @param motor Motor to send the command to.
 * @param axisName Used to make output more readable to the user.
 */
void pureCMD(const String &cmds, LinearMotor &motor, const String &axisName)
{
    auto adu = ModbusADU();

    const auto buffer_tx = cmds.substring(2, cmds.length());

    int index = 0;
    int startPos = 0;
    int commaPos = buffer_tx.indexOf(',', startPos);
    while (commaPos != -1)
    {
        adu.rtu[index++] = buffer_tx.substring(startPos, commaPos).toInt();
        startPos = commaPos + 1;
        commaPos = buffer_tx.indexOf(',', startPos);
    }
    // 处理最后一个数字（如果有）
    //Last digit may not have a comma after it
    if (startPos < buffer_tx.length())
    {
        adu.rtu[index++] = buffer_tx.substring(startPos).toInt();
    }
    adu.setLength(index);
    adu.updateCrc();

    printHexArray(adu.rtu, adu.getRtuLen());
    motor.writeAdu(adu);

    const auto readStatus = motor.readAdu(adu);
    if (readStatus)
    {
        adu.prepareExceptionResponse(GATEWAY_TARGET_DEVICE_FAILED_TO_RESPOND);
    }
    Serial.print(axisName + " axis value: ");
    printHexArray(adu.data, adu.getDataLen());
}

void sendCmdByPort(const String &cmd)
{
    if(cmd.startsWith("DISABLE"))
    {
        disableBothMotors();
    }
    else if(cmd.startsWith("VERSION"))
    {
      Serial.println(VERSION);
    }
    else if(cmd.startsWith("##"))
    {
       pureCMD(cmd, *XMotor,"X");
    }
    else if(cmd.startsWith("@@"))
    {
       pureCMD(cmd, *YMotor,"Y");
    }
    else if(cmd.startsWith("ENABLE"))
    {
        enableBothMotors();
    }
    else if(cmd.startsWith("AUTO_GAIN_OFF"))
    {
        XMotor->setAutoGain(false);
        YMotor->setAutoGain(false);
    }
    else if(cmd.startsWith("FILTER_OFF"))
    {
        XMotor->setFilter1Off();
        XMotor->setFilter2Off();
        YMotor->setFilter1Off();
        YMotor->setFilter2Off();
    }
    else if(cmd.startsWith("CURRENT_X:"))
    {
        const auto value = cmd.substring(10).toInt();
        XMotor->setCurrentGain(value);
    }
    else if(cmd.startsWith("CURRENT_Y:"))
    {
        const auto value = cmd.substring(10).toInt();
        YMotor->setCurrentGain(value);
    }
    else if(cmd.startsWith("INERDIA_X:"))
    {
        const auto value = cmd.substring(10).toInt();
        XMotor->setInertia(value);
    }
    else if(cmd.startsWith("INERDIA_Y:"))
    {
        const auto value = cmd.substring(10).toInt();
        YMotor->setInertia(value);
    }
    else if(cmd.startsWith("GET_CURRENT_X"))
    {
        const auto response = XMotor->getCurrentGain();
        if(std::holds_alternative<ModbusRTUMasterError>(response))
        {
            Serial.println("Communication Error");
        } else
        {
            Serial.println(std::get<uint32_t>(response));
        }
    }
    else if(cmd.startsWith("GET_CURRENT_Y"))
    {
        const auto response = YMotor->getCurrentGain();
        if(std::holds_alternative<ModbusRTUMasterError>(response))
        {
            Serial.println("Communication Error");
        } else
        {
            Serial.println(std::get<uint32_t>(response));
        }
    }
    else if(cmd.startsWith("GET_INERDIA_X"))
    {
        const auto response = XMotor->getInertia();
        if(std::holds_alternative<ModbusRTUMasterError>(response))
        {
            Serial.println("Communication Error");
        } else
        {
            Serial.println(std::get<uint32_t>(response));
        }
    }
    else if(cmd.startsWith("GET_INERDIA_Y"))
    {
        const auto response = YMotor->getInertia();
        if(std::holds_alternative<ModbusRTUMasterError>(response))
        {
            Serial.println("Communication Error");
        } else
        {
            Serial.println(std::get<uint32_t>(response));
        }
    }
    else if(cmd.startsWith("RTU_GATEWAY"))
    {
        mode = RTU_GATEWAY;
        XLed.setColor(OFF);
        YLed.setColor(OFF);
    }
    else
    {
        Serial.println("Unknown Command");
    }
}

void clearIncomingData(Stream & stream)
{
    while(stream.available())
    {
        stream.read();
    }
}

void disableBothMotors()
{
    XMotor->disable();
    YMotor->disable();

    clearIncomingData(XMotorSerial);
    clearIncomingData(YMotorSerial);
}

void enableBothMotors()
{
    XMotor->enable();
    YMotor->enable();

    clearIncomingData(XMotorSerial);
    clearIncomingData(YMotorSerial);
}

inline void setErrorState(const bool isError)
{
    if (motorError == isError)
    {
        return;
    }
    motorError = isError;
    digitalWrite(EMERGE_STOP_PIN, !isError);
}

void setRTURegisters()
{
    holdingRegisters[0] = mode;
    holdingRegisters[1] = XLed.getColor();
    holdingRegisters[2] = YLed.getColor();
    discreteInputs[0] = DisableButton.getState();
    discreteInputs[1] = EnableButton.getState();
    //motorError // Handled automatically
}

void updateFromRTURegisters()
{
    mode = static_cast<OperatingMode>(holdingRegisters[0]);
    XLed.setColor(static_cast<RGLedColor>(holdingRegisters[1]));
    YLed.setColor(static_cast<RGLedColor>(holdingRegisters[2]));
}

/**
 * @brief Forwards packets, while acting as a Modbus slave.
 * @warning This stops all automatic tasks, and relies on the host for all logic.
 * @details Acts as a Modbus slave with an id of 1.
 *          Routes Modbus packets with an id of 2 to X motor.
 *          Routes Modbus packets with an id of 3 to Y motor.
 *          <br/>
 *          Writing a 0 to id 1, holding register 0 exits this mode.
 */
void executeRtuGatewayLogic()
{
    auto adu = ModbusADU();
    if (HostComm->readAdu(adu) != MODBUS_RTU_COMM_SUCCESS)
    {
        return;
    }

    switch (adu.getUnitId())
    {
    case 1:
        setRTURegisters();
        RTUSlaveLogic.processPdu(adu);
        updateFromRTURegisters();
        break;
    case 2:
        XMotor->forwardAdu(adu);
        break;
    case 3:
        YMotor->forwardAdu(adu);
        break;
    default:
        adu.prepareExceptionResponse(GATEWAY_PATH_UNAVAILABLE);
        YLed.setColor(RED);
        break;
    }
    HostComm->writeAdu(adu);
}

void setup()
{
    HostComm = new ModbusRTUComm(Serial);
    Serial.begin(MODBUS_BAUD);
    HostComm->begin(MODBUS_BAUD, SERIAL_8N1);
    RTUSlaveLogic.configureHoldingRegisters(holdingRegisters.data(), holdingRegisters.size());
    RTUSlaveLogic.configureDiscreteInputs(discreteInputs.data(), discreteInputs.size());

    XMotor = new LinearMotor(XMotorSerial, 1);
    XMotor->begin(MODBUS_BAUD, SERIAL_8N1, 22, 23);

    YMotor = new LinearMotor(YMotorSerial, 1);
    YMotor->begin(MODBUS_BAUD, SERIAL_8N1, 16, 17);

    EnableButton.begin(enableBothMotors);
    DisableButton.begin(disableBothMotors);

    XLed.begin();
    YLed.begin();

    pinMode(EMERGE_STOP_PIN, OUTPUT);
    setErrorState(false);

    Serial.print("System inited, Version: magx-eslm-");
    Serial.println(VERSION);
}

void loop()
{
    if (mode == ASCII)
    {
        const auto xError = checkForError(*XMotor, "X");
        const auto yError = checkForError(*YMotor, "Y");

        setErrorState(xError || yError);
        XLed.setColor(xError ? RED : GREEN );
        YLed.setColor(yError ? RED : GREEN );

        EnableButton.update();
        DisableButton.update();

        readCmd();
    }

    if (mode == RTU_GATEWAY)
    {
        executeRtuGatewayLogic();
    }
}
