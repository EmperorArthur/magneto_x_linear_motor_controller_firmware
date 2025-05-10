/**
 *@file
 */
#include <cstring>
#include <Arduino.h>
#include <filesystem>
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

unsigned char getStringIndexChar(String cmd, int index)
{
  unsigned char ret = 0x00;
  if(cmd.length()>index)
  {
     char tmp = cmd.indexOf(index);
      ret = (unsigned char)tmp;
   }
   return ret;
}

void printHex(unsigned char value) 
{
    Serial.print("0x");
    if (value < 16) 
    {
        Serial.print("0");
    }
    Serial.print(value, HEX);
}

void readCmd()
{
  //读取所有的数据
  while (Serial.available() > 0 && recvl_ok == false)
  {
    char recieved = Serial.read();
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

void printUint16(const uint16_t value)
{
    printHex(value >> 8 & 0xFF);
    Serial.print(",");
    printHex(value & 0xFF);
    Serial.println(" ");
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
        printUint16(status.errorCode);
        return true;
    }
    return false;
}

void printHexArray(unsigned char* hex_data, int len)
{
  for(int i=0; i<len; i++)
  {
    printHex(hex_data[i]);
    Serial.print(",");  
  }
  Serial.println(" ");
}

void pureCMD(const String &cmds, LinearMotor &motor, const String &axisName)
{
    auto adu = ModbusADU();
    adu.setLength(6);

//  String cmds = "##1,2,3,4,5,6";
  const auto buffer_tx = cmds.substring(2, cmds.length());

  int index = 0;
  int startPos = 0;
  int commaPos = buffer_tx.indexOf(',', startPos);
  while (commaPos != -1 && index < 6) 
  {
      adu.rtu[index++] = buffer_tx.substring(startPos, commaPos).toInt();
      startPos = commaPos + 1;
      commaPos = buffer_tx.indexOf(',', startPos);
  }
  // 处理最后一个数字（如果有）
  if (startPos < buffer_tx.length() && index < 6) 
  {
      adu.rtu[index] = buffer_tx.substring(startPos).toInt();
  }
    printHexArray(adu.rtu, 6);
    motor.writeAdu(adu);

    auto incomingAdu = ModbusADU();
    motor.readAdu(incomingAdu);

    Serial.print(axisName + " axis value:");
    printHex(incomingAdu.rtu[5]);
    Serial.print(",");
    printHex(incomingAdu.rtu[6]);
    Serial.println(" ");
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
            printUint16(std::get<uint32_t>(response));
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
            printUint16(std::get<uint32_t>(response));
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
            printUint16(std::get<uint32_t>(response));
        }
    }
    else if(cmd.startsWith("GET_INERDIA_Y"))
    {
        auto response = YMotor->getInertia();
        if(std::holds_alternative<ModbusRTUMasterError>(response))
        {
            Serial.println("Communication Error");
        } else
        {
            printUint16(std::get<uint32_t>(response));
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
