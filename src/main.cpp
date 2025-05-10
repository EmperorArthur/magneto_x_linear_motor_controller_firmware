/**
 *@file
 */
#include <cstring>
#include <Arduino.h>
#include <ModbusADU.h>

#include "Button.hpp"
#include "LinearMotor.hpp"
#include "RGLed.hpp"

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
void sendCmdByPort(const String &cmd);

#define VERSION "1.0.7-git"

bool recvl_ok = false;
String inData="";

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

//To get
/**
 * @brief Send a raw Modbus command to a motor, and display the response.
 * @details Commands are in the format "##1,3,240,16,0,2"
 *          The above retrieves X axis position: "##1,3,240,16,0,2"
 * @param cmds String of comma separated ints.  Preceded by two characters.
 * @param motor Motor to send the command to
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
    delay(50);

    auto incomingAdu = ModbusADU();
    motor.readAdu(incomingAdu);
    Serial.print(axisName + " axis value: ");
    printHexArray(incomingAdu.data, incomingAdu.getDataLen());
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
        delay(100);
        YMotor->setAutoGain(false);
    }
    else if(cmd.startsWith("FILTER_OFF"))
    {
        XMotor->setFilter1Off();
        delay(100);
        XMotor->setFilter2Off();
        delay(100);
        YMotor->setFilter1Off();
        delay(100);
        YMotor->setFilter2Off();
    }
    else if(cmd.startsWith("CURRENT_X:"))
    {
        const auto value = static_cast<uint8_t>(cmd.substring(10).toInt());
        XMotor->setCurrentGain(value);
    }
    else if(cmd.startsWith("CURRENT_Y:"))
    {
        const auto value = static_cast<uint8_t>(cmd.substring(10).toInt());
        YMotor->setCurrentGain(value);
    }
    else if(cmd.startsWith("INERDIA_X:"))
    {
        const auto value = static_cast<uint16_t>(cmd.substring(10).toInt());
        XMotor->setInertia(value);
    }
    else if(cmd.startsWith("INERDIA_Y:"))
    {
        const auto value = static_cast<uint16_t>(cmd.substring(10).toInt());
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
    clearIncomingData(XMotorSerial);
    clearIncomingData(YMotorSerial);
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
    delay(50);
    YMotor->disable();

    clearIncomingData(XMotorSerial);
    clearIncomingData(YMotorSerial);
}

void enableBothMotors()
{
    XMotor->enable();
    delay(50);
    YMotor->enable();

    clearIncomingData(XMotorSerial);
    clearIncomingData(YMotorSerial);
}

inline void setErrorPin(const bool isError)
{
    digitalWrite(EMERGE_STOP_PIN, !isError);
}

void setup()
{
    Serial.begin(MODBUS_BAUD);

    XMotor = new LinearMotor(XMotorSerial, 1);
    XMotor->begin(MODBUS_BAUD, SERIAL_8N1, 22, 23);

    YMotor = new LinearMotor(YMotorSerial, 1);
    YMotor->begin(MODBUS_BAUD, SERIAL_8N1, 16, 17);

    EnableButton.Setup(enableBothMotors);
    DisableButton.Setup(disableBothMotors);

    XLed.Setup();
    YLed.Setup();

    Serial.print("System inited, Version: magx-eslm-");
    Serial.println(VERSION);

    pinMode(EMERGE_STOP_PIN, OUTPUT);
    setErrorPin(false);
}

void loop()
{
    const bool xError = checkForError(*XMotor, "X");
    xError ? XLed.SetRed() : XLed.SetGreen();
    setErrorPin(xError);
    delay(50);
    const bool yError = checkForError(*YMotor, "Y");
    yError ? YLed.SetRed() : YLed.SetGreen();
    setErrorPin(yError);
    delay(50);
    readCmd();
    delay(50);
    EnableButton.Update();
    DisableButton.Update();
}
