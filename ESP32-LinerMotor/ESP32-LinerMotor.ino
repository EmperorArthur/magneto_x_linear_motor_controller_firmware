#include "ModeBusSender.h"

void processPureData();
//test1: 01 03 f0 0a 00 01 97 08
//test2: 01 06 f0 0a 00 03 da c9
#define ERROR_CODE_1 0x00
#define ERROR_CODE_2 0x00
#define VERSION "1.0.7-20240227"
unsigned char current_mode[6]={0x01, 0x03, 0xF0, 0x0A, 0x00, 0x01};
unsigned char current_position[6]={0x01, 0x03, 0xF0, 0x10, 0x00, 0x02};

//设置自动增益开关,设置为关
unsigned char set_auto_gain_cmd[6]={0x01, 0x06, 0x04, 0x55, 0x00, 0x00};

//获取自动增益状态
unsigned char get_auto_gain_cmd[6]={0x01, 0x03, 0x04, 0x55, 0x00, 0x01};

//设置指令电流滤波器1 类型, 不滤波
unsigned char set_filter1_cmd[6]={0x01, 0x06, 0x04, 0x06, 0x00, 0x00};

//设置指令电流滤波器1 类型, 不滤波
unsigned char set_filter2_cmd[6]={0x01, 0x06, 0x04, 0x0B, 0x00, 0x00};

//获取指令电流滤波器1类型
unsigned char get_filter1_cmd[6]={0x01, 0x03, 0x04, 0x06, 0x00, 0x01};

//获取电机动子惯量
unsigned char get_motor_intertia_code_cmd[6]={0x01, 0x03, 0x00, 0x28, 0x00, 0x02};

//设置电机动子惯量
unsigned char set_motor_intertia_code_cmd[6]={0x01, 0x06, 0x00, 0x28, 0x00, 0xE6};


//获取电气增益值
unsigned char get_motor_current_gain_code_cmd[6]={0x01, 0x03, 0x00, 0x18, 0x00, 0x02};
//返回0x01, 0x03, 0x04, 0x00, 0x00, 0x00, 0x64, 0xFB, 0xD8，0x64=100

//设置电气增益值，100
unsigned char set_motor_current_gain_code_cmd[6]={0x01, 0x06, 0x00, 0x18, 0x00, 0x64};

//保存参数
unsigned char save_param_code_cmd[6]={0x01, 0x06, 0x60, 0x00, 0x00, 0x01};

//chack save ok
unsigned char check_save_param_code_cmd[6]={0x01, 0x03, 0x01, 0x8A, 0x00, 0x01};

unsigned char get_motor_error_code_cmd[6]={0x01, 0x03, 0xF0, 0x01, 0x00, 0x02};
unsigned char enable_motor_cmd[6]={0x01, 0x06, 0xF0, 0x02, 0x00, 0x0F};
unsigned char disable_motor_cmd[6]={0x01, 0x06, 0xF0, 0x02, 0x00, 0x06};
unsigned char clean_error_cmd[6]={0x01, 0x06, 0xF0, 0x02, 0x00, 0x80};

bool recvl_ok = false;
bool recvl_x = false;
bool recvl_y = false;

unsigned char read_state = 0;  //0: read error code   1: get params value 

String inData="";
String inPortx="";

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

// HardwareSerial serialPortx(1);
// #define Serial2 Serial2

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

void readPortx()
{
  byte pdata[9];
  int index = 0;
  while (Serial1.available() >0)
  {
    char recieved = Serial1.read();
    inPortx += recieved; 
    if(index<9)
    {
       pdata[index]=recieved;
    }
     index++;
  }

    if(read_state==0)
    {
         // Check if the received data packet is an error packet
        if ((pdata[5] != ERROR_CODE_1) && (pdata[6] != ERROR_CODE_2)) 
        {
          errorStateX = true; // Set error state
          Serial.print("X axis error:");
          printHex(pdata[5]);
          Serial.print(",");
          printHex(pdata[6]);
          Serial.println(" ");
          digitalWrite(EMERGE_STOP_PIN, LOW);
          delay(100);
          digitalWrite(EMERGE_STOP_PIN, HIGH);
        } 
        else 
        {
          errorStateX = false; // No error
        }
    }
    else
    {
          Serial.print("X axis value:");
          printHex(pdata[5]);
          Serial.print(",");
          printHex(pdata[6]);
          Serial.println(" ");
    }

}

void readPorty()
{
  byte data[9];
  int index = 0;
  while (Serial2.available() >0)
  {
    char recieved = Serial2.read();
    inPortx += recieved; 
    if(index<9)
    {
       data[index]=recieved;
     }
     index++;
  }

    if(read_state==0)
    {
       // Check if the received data packet is an error packet
      if ((data[5] != ERROR_CODE_1) && (data[6] != ERROR_CODE_2)) 
      {
        errorStateY = true; // Set error state
        digitalWrite(EMERGE_STOP_PIN, LOW);
        delay(100);
        digitalWrite(EMERGE_STOP_PIN, HIGH);
        Serial.print("Y axis error:");
        printHex(data[5]);
        Serial.print(",");
        printHex(data[6]);
        Serial.println(" ");
      } 
      else 
      {
        errorStateY = false; // No error
      }
    }
    else 
    {
        Serial.print("Y axis value:");
        printHex(data[5]);
        Serial.print(",");
        printHex(data[6]);
        Serial.println(" ");
    }

}

void setInerdia(int motor_num, int value)
{
//  data[0] = (num >> 8) & 0xFF; // 获取高8位
//    data[1] = num & 0xFF;        // 获取低8位
  set_motor_intertia_code_cmd[5]= (value & 0xFF);
  set_motor_intertia_code_cmd[4]= (value >> 8) & 0xFF; 

  delay(100);
  sendModebusCmd(motor_num, set_motor_intertia_code_cmd, 6);
  delay(100);
  
  disableMotor(motor_num);
  delay(100);
  sendModebusCmd(motor_num, save_param_code_cmd, 6);
  delay(100);
  sendModebusCmd(motor_num, check_save_param_code_cmd, 6);
  delay(100);
  sendModebusCmd(motor_num, clean_error_cmd, 6); 
  delay(100);
  sendModebusCmd(motor_num, enable_motor_cmd, 6);
}

void setCurentGain(int motor_num, unsigned char value)
{
//  Serial.println(set_motor_current_gain_code_cmd[5]);
  set_motor_current_gain_code_cmd[5]=value;
  
  delay(100);
  sendModebusCmd(motor_num, set_motor_current_gain_code_cmd, 6);
  delay(100);
  disableMotor(motor_num);
  delay(100);
  sendModebusCmd(motor_num, save_param_code_cmd, 6);
  delay(100);
  sendModebusCmd(motor_num, check_save_param_code_cmd, 6);
  delay(100);
  sendModebusCmd(motor_num, clean_error_cmd, 6); 
  delay(100);
  sendModebusCmd(motor_num, enable_motor_cmd, 6);
}

void set_auto_gain_off(int motor_num)
{
  delay(100);
  sendModebusCmd(motor_num, set_auto_gain_cmd, 6);
  delay(100);
  disableMotor(motor_num);
  delay(100);
  sendModebusCmd(motor_num, save_param_code_cmd, 6);
  delay(100);
  sendModebusCmd(motor_num, check_save_param_code_cmd, 6);
  delay(100);
  sendModebusCmd(motor_num, clean_error_cmd, 6); 
  delay(100);
  sendModebusCmd(motor_num, enable_motor_cmd, 6);
}

void set_filter1_off(int motor_num)
{
  delay(100);
  sendModebusCmd(motor_num, set_filter1_cmd, 6);
  delay(100);
  disableMotor(motor_num);
  delay(100);
  sendModebusCmd(motor_num, save_param_code_cmd, 6);
  delay(100);
  sendModebusCmd(motor_num, check_save_param_code_cmd, 6);
  delay(100);
  sendModebusCmd(motor_num, clean_error_cmd, 6); 
  delay(100);
  sendModebusCmd(motor_num, enable_motor_cmd, 6);
}


void set_filter2_off(int motor_num)
{
  delay(100);
  sendModebusCmd(motor_num, set_filter2_cmd, 6);
  delay(100);
  disableMotor(motor_num);
  delay(100);
  sendModebusCmd(motor_num, save_param_code_cmd, 6);
  delay(100);
  sendModebusCmd(motor_num, check_save_param_code_cmd, 6);
  delay(100);
  sendModebusCmd(motor_num, clean_error_cmd, 6); 
  delay(100);
  sendModebusCmd(motor_num, enable_motor_cmd, 6);
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

void pureCMD(String cmds, int motor_num)
{
//  String cmds = "##1,2,3,4,5,6";
  String buffer_tx = cmds.substring(2, cmds.length());
  unsigned char cmdArray[6] = {0};
  int index = 0;
  int startPos = 0;
  int commaPos = buffer_tx.indexOf(',', startPos);
  while (commaPos != -1 && index < 6) 
  {
      cmdArray[index++] = buffer_tx.substring(startPos, commaPos).toInt();
      startPos = commaPos + 1;
      commaPos = buffer_tx.indexOf(',', startPos);
  }
  // 处理最后一个数字（如果有）
  if (startPos < buffer_tx.length() && index < 6) 
  {
      cmdArray[index] = buffer_tx.substring(startPos).toInt();
  }
  printHexArray(cmdArray, 6);
  sendModebusCmd(motor_num, cmdArray, 6);
  if(motor_num==0)
    read_state = 1;
  else if(motor_num==1)
    read_state = 2;
  delay(100);
}

void sendCmdByPort( String cmd_str)
{
    
    String cmd = cmd_str;
    if(cmd.startsWith("DISABLE"))
    {
      sendModebusCmd(0, disable_motor_cmd, 6);
      delay(100);
      sendModebusCmd(1, disable_motor_cmd, 6);
    }
    else if(cmd.startsWith("VERSION"))
    {
      Serial.println(VERSION);
    }
    else if(cmd.startsWith("##"))
    {
       pureCMD(cmd,0);
    }
    else if(cmd.startsWith("@@"))
    {
       pureCMD(cmd,1);
    }
    else if(cmd.startsWith("ENABLE"))
    {
        enableMotor(0); 
        delay(100);
        enableMotor(1); 
    }
    else if(cmd.startsWith("AUTO_GAIN_OFF"))
    {
        set_auto_gain_off(0);
        delay(100);
        set_auto_gain_off(1);
    }
    else if(cmd.startsWith("FILTER_OFF"))
    {
        set_filter1_off(0);
        delay(100);
        set_filter1_off(1);
        delay(100);
        set_filter2_off(0);
        delay(100);
        set_filter2_off(1);
    }
    else if(cmd.startsWith("CURRENT_X:"))
    {
        unsigned char value = (unsigned char)(cmd.substring(10).toInt());
        setCurentGain(0, value);
    }
    else if(cmd.startsWith("CURRENT_Y:"))
    {
        unsigned char value = (unsigned char)(cmd.substring(10).toInt());
        setCurentGain(1, value);
     }
     else if(cmd.startsWith("INERDIA_X:"))
     {
         int value = (int)(cmd.substring(10).toInt());
         setInerdia(0, value);
     }
     else if(cmd.startsWith("INERDIA_Y:"))
     {
         int value = (int)(cmd.substring(10).toInt());
         setInerdia(1, value);
     }
    else if(cmd.startsWith("GET_CURRENT_X"))
    {
        //get_motor_current_gain_code_cmd
        sendModebusCmd(0, get_motor_current_gain_code_cmd, 6);
        read_state = 1;
        delay(20);
    }
    else if(cmd.startsWith("GET_CURRENT_Y"))
    {
        sendModebusCmd(1, get_motor_current_gain_code_cmd, 6);
        read_state = 2;
        delay(20);
    }
   else if(cmd.startsWith("GET_INERDIA_X"))
   {
        sendModebusCmd(0, get_motor_intertia_code_cmd, 6);
        read_state = 1;
        delay(20);
   }
   else if(cmd.startsWith("GET_INERDIA_Y"))
   {
        sendModebusCmd(1, get_motor_intertia_code_cmd, 6);
        read_state = 2;
        delay(20);
   }

   if(read_state == 1)
   {
      readPortx();
      read_state = 0;
   }
   else if(read_state==2)
   {
      readPorty(); 
      read_state = 0;
   }
     
    inData = "";
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

void disableMotor(int number)
{
  sendModebusCmd(number, disable_motor_cmd, 6);
}

void enableMotor(int number)
{
      sendModebusCmd(number, disable_motor_cmd, 6);
      delay(100);
      sendModebusCmd(number, clean_error_cmd, 6);
      delay(100);
      sendModebusCmd(number, enable_motor_cmd, 6);
     
}

void check_button()
{
  if(digitalRead(BUTTON_ENABLE_PIN) == LOW) 
  {
    delay(100);
    if(digitalRead(BUTTON_ENABLE_PIN) == LOW) 
    {
//      if(errorStateX)
      {
        enableMotor(0); 
       }
//       if(errorStateY)
       {
        enableMotor(1); 
       }
    }
  }
  if(digitalRead(BUTTON_DISABLE_PIN) == LOW) 
  {
    delay(100);
    if(digitalRead(BUTTON_DISABLE_PIN) == LOW) 
    {
      sendModebusCmd(0, disable_motor_cmd, 6);
//      errorStateX = true;
      delay(100);
      sendModebusCmd(1, disable_motor_cmd, 6);
//      errorStateY = true;
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

void setup()
{
    Serial.begin(115200);
    Serial1.begin(115200, SERIAL_8N1, 22, 23);
    Serial2.begin(115200, SERIAL_8N1, 16, 17);

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

void get_error_code()
{
  sendModebusCmd(0, get_motor_error_code_cmd, 6);
  delay(50);
  sendModebusCmd(1, get_motor_error_code_cmd, 6);
  delay(50);
}

void loop()
{
  readCmd();
  get_error_code();
  readPortx();
  readPorty();
  check_button();
  led1(errorStateX);
  led2(errorStateY);
    // Serial2.print("hello kitty");
//  delay(10);
  
}
