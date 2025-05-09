/**
 *@file
 */
#include <cstring>
#include <Arduino.h>
#include <ModbusRTUComm.h>
#include <ModbusADU.h>
#include "LinearMotorCommands.hpp"

ModbusRTUComm* XMotor;
ModbusRTUComm* YMotor;

auto & XMotorSerial = Serial1;
auto & YMotorSerial = Serial2;

bool disableMotor(ModbusRTUComm &rtuComm);
bool enableMotor(ModbusRTUComm &rtuComm);

void processPureData();
void sendCmdByPort(const String &cmd);

inline bool sendAdu(ModbusRTUComm &comm, const ModbusADU &adu) {
    auto local = adu;
    return comm.writeAdu(local);
}

//test1: 01 03 f0 0a 00 01 97 08
//test2: 01 06 f0 0a 00 03 da c9
#define ERROR_CODE_1 0x00
#define ERROR_CODE_2 0x00
#define VERSION "1.0.7-git"

bool recvl_ok = false;

String inData="";

#define MODBUS_BAUD 115200
#define BUTTON_ENABLE_PIN 15
#define BUTTON_DISABLE_PIN 4

#define EMERGE_STOP_PIN 14 //stop klipper when error occur

#define LED1_A_PIN 19  //X axis
#define LED1_B_PIN 21

#define LED2_A_PIN 5  //Y axis
#define LED2_B_PIN 18

int8_t led1_state = 0;
int8_t led2_state = 0;

bool errorStateX = false;
bool errorStateY = false;

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
  }
}

/**
 * @brief Query the motor driver for errors.
 * @param rtuComm Motor Comm Channel
 * @param axisName Used for reporting errors
 * @return true if there is an error.  Otherwise, false.
 */
bool checkForError(ModbusRTUComm &rtuComm, const String &axisName)
{
    sendAdu(rtuComm, get_motor_error_code_cmd);
    delay(100);

    auto adu = ModbusADU();
    const auto readStatus = rtuComm.readAdu(adu);

    if (readStatus)
    {
        digitalWrite(EMERGE_STOP_PIN, LOW);
        delay(100);
        digitalWrite(EMERGE_STOP_PIN, HIGH);
        Serial.println(axisName + " axis error: Communication Error");
        return true;
    }

    if ((adu.rtu[5] != ERROR_CODE_1 && adu.rtu[6] != ERROR_CODE_2))
    {
        Serial.print(axisName + " axis error:");
        printHex(adu.rtu[5]);
        Serial.print(",");
        printHex(adu.rtu[6]);
        Serial.println(" ");
        digitalWrite(EMERGE_STOP_PIN, LOW);
        delay(100);
        digitalWrite(EMERGE_STOP_PIN, HIGH);
        return true;
    }
    return false;
}

/**
 * @brief Read a response ADU, and print it.
 * @param rtuComm Motor Comm Channel
 * @param axisName Used for response.
 */
void readAndPrintResponse(ModbusRTUComm &rtuComm, const String &axisName)
{
    auto adu = ModbusADU();
    rtuComm.readAdu(adu);

    Serial.print(axisName + " axis value:");
    printHex(adu.rtu[5]);
    Serial.print(",");
    printHex(adu.rtu[6]);
    Serial.println(" ");
}

void saveParam(ModbusRTUComm &rtuComm)
{
    sendAdu(rtuComm, save_param_code_cmd);
    delay(100);
    sendAdu(rtuComm, check_save_param_code_cmd);
}

void setInertia(ModbusRTUComm &rtuComm, uint16_t value)
{
    auto command = set_motor_intertia_code_cmd;
    command.rtu[5]= lowByte(value);
    command.rtu[4]= highByte(value);
    command.updateCrc();

    disableMotor(rtuComm);
    delay(100);
    rtuComm.writeAdu(command);
    delay(100);
    saveParam(rtuComm);
    delay(100);
    enableMotor(rtuComm);
}

void setCurentGain(ModbusRTUComm &rtuComm, uint8_t value)
{
    auto command = set_motor_current_gain_code_cmd;
    command.rtu[5]=value;

    disableMotor(rtuComm);
    delay(100);
    rtuComm.writeAdu(command);
    delay(100);
    saveParam(rtuComm);
    delay(100);
    enableMotor(rtuComm);
}

void set_auto_gain_off(ModbusRTUComm &rtuComm)
{
    disableMotor(rtuComm);
    delay(100);
    sendAdu(rtuComm, set_auto_gain_cmd);
    delay(100);
    saveParam(rtuComm);
    delay(100);
    enableMotor(rtuComm);
}

void set_filter1_off(ModbusRTUComm &rtuComm)
{
    disableMotor(rtuComm);
    delay(100);
    sendAdu(rtuComm, set_filter1_cmd);
    delay(100);
    saveParam(rtuComm);
    delay(100);
    enableMotor(rtuComm);
}


void set_filter2_off(ModbusRTUComm &rtuComm)
{
    disableMotor(rtuComm);
    delay(100);
    sendAdu(rtuComm, set_filter2_cmd);
    delay(100);
    saveParam(rtuComm);
    delay(100);
    enableMotor(rtuComm);
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

void pureCMD(const String &cmds, ModbusRTUComm &rtuComm, const String &axisName)
{
    auto adu = ModbusADU();
    adu.setLength(6);

//  String cmds = "##1,2,3,4,5,6";
  String buffer_tx = cmds.substring(2, cmds.length());

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
    rtuComm.writeAdu(adu);
  delay(100);
    readAndPrintResponse(rtuComm, axisName);
}

void sendCmdByPort(const String &cmd)
{
    if(cmd.startsWith("DISABLE"))
    {
      disableMotor(*XMotor);
      delay(100);
      disableMotor(*YMotor);
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
        enableMotor(*XMotor);
        delay(100);
        enableMotor(*YMotor);
    }
    else if(cmd.startsWith("AUTO_GAIN_OFF"))
    {
        set_auto_gain_off(*XMotor);
        delay(100);
        set_auto_gain_off(*YMotor);
    }
    else if(cmd.startsWith("FILTER_OFF"))
    {
        set_filter1_off(*XMotor);
        delay(100);
        set_filter1_off(*YMotor);
        delay(100);
        set_filter2_off(*XMotor);
        delay(100);
        set_filter2_off(*YMotor);
    }
    else if(cmd.startsWith("CURRENT_X:"))
    {
        unsigned char value = (unsigned char)(cmd.substring(10).toInt());
        setCurentGain(*XMotor, value);
    }
    else if(cmd.startsWith("CURRENT_Y:"))
    {
        unsigned char value = (unsigned char)(cmd.substring(10).toInt());
        setCurentGain(*YMotor, value);
     }
     else if(cmd.startsWith("INERDIA_X:"))
     {
         int value = (int)(cmd.substring(10).toInt());
         setInertia(*XMotor, value);
     }
     else if(cmd.startsWith("INERDIA_Y:"))
     {
         int value = (int)(cmd.substring(10).toInt());
         setInertia(*YMotor, value);
     }
    else if(cmd.startsWith("GET_CURRENT_X"))
    {
        sendAdu(*XMotor, get_motor_current_gain_code_cmd);
        delay(20);
        readAndPrintResponse(*XMotor, "X");
    }
    else if(cmd.startsWith("GET_CURRENT_Y"))
    {
        sendAdu(*YMotor, get_motor_current_gain_code_cmd);
        delay(20);
        readAndPrintResponse(*YMotor, "Y");
    }
   else if(cmd.startsWith("GET_INERDIA_X"))
   {
        sendAdu(*XMotor, get_motor_intertia_code_cmd);
        delay(20);
        readAndPrintResponse(*XMotor, "X");
   }
   else if(cmd.startsWith("GET_INERDIA_Y"))
   {
        sendAdu(*YMotor, get_motor_intertia_code_cmd);
        delay(20);
        readAndPrintResponse(*YMotor, "Y");
   }
}

void led1(int state)
{
  if(state==0)
  {
    digitalWrite(LED1_A_PIN, HIGH);
    digitalWrite(LED1_B_PIN, LOW);
  }
  else
  {
    digitalWrite(LED1_A_PIN, LOW);
    digitalWrite(LED1_B_PIN, HIGH);
  }
}

void led2(int state)
{
  if(state==0)
  {
    digitalWrite(LED2_A_PIN, HIGH);
    digitalWrite(LED2_B_PIN, LOW);
  }
  else
  {
    digitalWrite(LED2_A_PIN, LOW);
    digitalWrite(LED2_B_PIN, HIGH);
  }
}

bool disableMotor(ModbusRTUComm &rtuComm)
{
    return sendAdu(rtuComm, disable_motor_cmd);
}

bool enableMotor(ModbusRTUComm &rtuComm)
{
    bool result = sendAdu(rtuComm, disable_motor_cmd);
    delay(100);
    result &= sendAdu(rtuComm, clean_error_cmd);
    delay(100);
    result &= sendAdu(rtuComm, enable_motor_cmd);
    return result;
}

void check_button()
{
  if(digitalRead(BUTTON_ENABLE_PIN) == LOW) 
  {
    delay(100);
    if(digitalRead(BUTTON_ENABLE_PIN) == LOW) 
    {
        enableMotor(*XMotor);
        enableMotor(*YMotor);
    }
  }
  if(digitalRead(BUTTON_DISABLE_PIN) == LOW) 
  {
    delay(100);
    if(digitalRead(BUTTON_DISABLE_PIN) == LOW) 
    {
        disableMotor(*XMotor);
        disableMotor(*YMotor);
    }
  }

  if(digitalRead(BUTTON_ENABLE_PIN) == HIGH) 
  {
    led1(0);
  }
  if(digitalRead(BUTTON_DISABLE_PIN) == HIGH)
  {
    led2(0);
  }
}

void clearIncomingData(Stream & stream)
{
    while(stream.available())
    {
        stream.read();
    }
}

void setup()
{
    Serial.begin(MODBUS_BAUD);

    XMotorSerial.setRxBufferSize(sizeof(ModbusADU));
    XMotorSerial.setTxBufferSize(sizeof(ModbusADU));
    XMotorSerial.begin(MODBUS_BAUD, SERIAL_8N1, 22, 23);
    XMotor = new ModbusRTUComm(XMotorSerial);
    XMotor->begin(MODBUS_BAUD);

    YMotorSerial.setRxBufferSize(sizeof(ModbusADU));
    YMotorSerial.setTxBufferSize(sizeof(ModbusADU));
    YMotorSerial.begin(MODBUS_BAUD, SERIAL_8N1, 16, 17);
    YMotor = new ModbusRTUComm(YMotorSerial);
    YMotor->begin(MODBUS_BAUD);

    pinMode(BUTTON_ENABLE_PIN, INPUT_PULLUP);
    pinMode(BUTTON_DISABLE_PIN, INPUT_PULLUP);

    pinMode(LED1_A_PIN, OUTPUT);
    pinMode(LED1_B_PIN, OUTPUT);
    pinMode(LED2_A_PIN, OUTPUT);
    pinMode(LED2_B_PIN, OUTPUT);

    Serial.print("System inited, Version: magx-eslm-");
    Serial.println(VERSION);

    delay(1000);
    pinMode(EMERGE_STOP_PIN, OUTPUT);
    digitalWrite(EMERGE_STOP_PIN, HIGH);
}

void loop()
{
    errorStateX = checkForError(*XMotor, "X");
    errorStateY = checkForError(*YMotor, "Y");
  readCmd();
    clearIncomingData(XMotorSerial);
    clearIncomingData(YMotorSerial);
    check_button();
    clearIncomingData(XMotorSerial);
    clearIncomingData(YMotorSerial);
  led1(errorStateX);
  led2(errorStateY);
}
