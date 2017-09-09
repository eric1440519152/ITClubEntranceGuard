/*
   IT社门禁系统
   作者：何泽恩
*/



/*
   @ include
   包含库文件
*/
#include <SPI.h>
#include <RFID.h>
#include <stdio.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

/*
   @ 定义引脚
*/
LiquidCrystal_I2C lcd(0x27, 16, 2);
RFID rfid(53, 5); //SS引脚、RST引脚
int Touch_ID_Moniter = 46;
int Buzzer = A8;
int Doorpin = 2;

/*
   @ 变量初始化
*/

//定义系统变量
String Get_ID;
String Card_ID;
String Touch_ID;
String Last_Get_ID;
String Last_Card_ID;
String Last_Touch_ID;

bool If_Card;
bool Door_Opened;

/*
   @ 预置账号
*/

//奇偶无特殊要求，包含卡号和指纹ID
const String Admin_ID[] = {/*社长校牌*/"28118121122105", "20218013195176", "1"};
//卡号在奇数，指纹在偶数，只能有一个指纹且必须配对
const String User_ID[] = {"18127158221237", "0",/*社长工牌*/"2112316622121", "5"};

/*
   @ 子程序声明
*/

/*
   @ 初始化程序
*/
void setup() {
  //初始化LCD屏幕
  lcd.init();
  lcd.backlight();

  //显示正在初始化
  LCD_Show_Starting();

  //初始化串口设置
  Serial.begin(9600);
  Serial2.begin(115200);
  Serial3.begin(115200);

  //初始化读卡器
  SPI.begin();
  rfid.init();

  //初始化引脚
  pinMode(Buzzer, OUTPUT);

  //初始化系统变量
  Door_Opened = false;
  If_Card = false;
}

/*
   @ 主循环程序
*/
void loop() {
  //局部状态变量声明
  String Listen_State;

  //变量替换
  Last_Card_ID = Card_ID;
  Last_Get_ID = Get_ID;
  Last_Touch_ID = Touch_ID;

  Card_ID = "";
  Get_ID = "";
  Touch_ID = "";

  //显示主界面
  LCD_Show_Main();

  //启动监听
  Listen_State = Listen();
  if (Listen_State != "No_Action") {

    //进行下一步验证

    if (Listen_State == "Card_Success" or Listen_State == "Touch_Success") {

      //第一阶段验证成功

      //定义局部变量
      int p = 0;
      bool If_Admin =	false;
      bool If_User = false;

      //下一步验证，判断是管理员还是普通用户
      //循环次数等于数组成员数
      while (p < (sizeof(Admin_ID) / sizeof(Admin_ID[0]))) {
        if ((Admin_ID[p] == Touch_ID or Admin_ID[p] == Card_ID) and (Admin_ID[p] != "")) {
          //是超管，执行超管动作

          // @!
          Serial.println("R:" + Admin_ID[p]);
          //Serial.println(p);

          // @!
          Serial.println("admin!");

          //更新全局变量
          If_Admin = true;

          //显示管理员欢迎
          LCD_Show_Admin();

          //蜂鸣器提示通过
          Buzzer_Show_Success();

          //开门
          Open_Door();

          break;
        }
        p++;
      }

      if (!If_Admin) {
        //超管判断未通过

        p = 0;
        while (p < (sizeof(User_ID) / sizeof(User_ID[0]))) {
          if ((User_ID[p] == Touch_ID or User_ID[p] == Card_ID) and (User_ID[p] != "")) {
            //是普通用户，二次验证

            // @!
            Serial.println("R:" + User_ID[p]);
            //Serial.println(p);

            if (Listen_State == "Card_Success") {

              //显示请求按指纹
              LCD_Show_Tip("Touch", "Need Your Touch");

              delay(2000);

              //提交二次验证请求
              If_User = Check_Again(Card_ID, "Card_Success", p);

              if (If_User) {

                //显示用户欢迎
                LCD_Show_User();

                //蜂鸣器提示通过
                Buzzer_Show_Success();

                //开门
                Open_Door();
              }
            } else if (Listen_State == "Touch_Success") {

              //显示请求按刷卡
              LCD_Show_Tip("Swing Card", "Swing Your Card");

              delay(2000);

              //提交二次验证请求
              If_User = Check_Again(Touch_ID, "Touch_Success", p);

              if (If_User) {

                //显示用户欢迎
                LCD_Show_User();

                //蜂鸣器提示通过
                Buzzer_Show_Success();

                //开门
                Open_Door();
              }

            }
            break;
          }
          p++;
        }

      }

      if (!If_Admin and !If_User) {
        //都不是

        //无权限报错
        LCD_Show_Err("Permission", "No Permission");

        //蜂鸣器提示失败
        Buzzer_Show_Fail();

      }

    }

    //其他情况跳出无视
  }

}

/*
   @子程序
*/

// @ LCD

//显示初始化
void LCD_Show_Starting() {
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("IT CLUB SYSTEM");
  lcd.setCursor(4, 1);
  lcd.print("Starting");
}

//显示等待
void LCD_Show_Wait() {
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("IT CLUB SYSTEM");
  lcd.setCursor(4, 1);
  lcd.print("Waiting.");
}

//显示错误
void LCD_Show_Err(String Title, String Err) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Err:" + Title);
  lcd.setCursor(0, 1);
  lcd.print(Err);
}

//显示提示
void LCD_Show_Tip(String Title, String tip) {
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("Tip:" + Title);
  lcd.setCursor(1, 1);
  lcd.print(tip);
}

//显示主界面
void LCD_Show_Main() {
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("IT CLUB SYSTEM");
  lcd.setCursor(1, 1);
  lcd.print("Touch or Swing");
}

//显示管理员欢迎
void LCD_Show_Admin() {
  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.print("Welcome!");
  lcd.setCursor(5, 1);
  lcd.print("Admin");
}

//显示普通用户欢迎
void LCD_Show_User() {
  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.print("Welcome!");
  lcd.setCursor(6, 1);
  lcd.print("User");
}

// @ 蜂鸣器
// 接受反馈
void Buzzer_Show_Getting() {
  Buzzer_On(180, 3500);
}

//成功
void Buzzer_Show_Success() {
  Buzzer_On(100, 3500);
  Buzzer_On(100, 3500);
}

//失败
void Buzzer_Show_Fail() {
  Buzzer_On(1000, 3500);
}

//响蜂鸣器API
void Buzzer_On(int times, int diao) {
  tone(Buzzer, diao);
  delay(times);
  noTone(Buzzer);
}

// @ 指纹
//读指纹到Pic区
String get_Touch_ID_Moniter() {
  //定义局部变量
  char comdata[20];
  int i = 1;

  //清空串口缓存
  Serial.flush();
  Serial2.flush();

  delay(150);

  //定义指令码
  unsigned char hexdata[12] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x03, 0x01, 0x00, 0x05};

  //发出指令
  Serial2.write(hexdata, 12);

  delay(1000);

  // @!
  Serial.print("Get Touch:");

  //Serial.println(Serial2.available());
  while (Serial2.available() > 0) {
    //当收到返回时

    comdata[i] = Serial2.read();

    delay(2);

    // @!
    /*Serial.println(i);
      Serial.println("HEX:");
      Serial.println(comdata[i], DEC);*/

    i++;
  }
  //Serial.println("char:"+comdata);
  if (comdata[10] == 0) {
    //成功

    // @!
    Serial.println("Success");

    return "Success";
  } else if (comdata[10] == 01) {
    //串口错误

    // @!
    Serial.println("Serial_Err");

    return "Serial_Err";
  } else if (comdata[10] == 02) {
    //读取不到手指

    // @!
    Serial.println("No_Finger");

    return "No_Finger";
  } else {
    //其他错误

    // @!
    Serial.println("Err");

    return "Err";
  }
}

//添加指纹到Buff
// @ in bufferid
String add_buff_Touch_ID_Moniter(int bufferid) {
  //定义局部变量
  char comdata[20];
  int i = 1;

  //清空串口缓存
  Serial.flush();
  Serial2.flush();

  delay(150);

  //预置指令码并写入指令码
  if (bufferid == 1) {
    unsigned char hexdata[13] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x04, 0x02, 0x01, 0x00, 0x08};
    Serial2.write(hexdata, 13);
  } else {
    unsigned char hexdata[13] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x04, 0x02, 0x02, 0x00, 0x09};
    Serial2.write(hexdata, 13);
  }

  delay(1000);

  // @！
  Serial.print("Add Buffer:");
  Serial.println(Serial2.available());

  while (Serial2.available() > 0) {
    //当收到返回时
    comdata[i] = Serial2.read();
    delay(10);

    // @！
    /*Serial.println(i);
      Serial.println("HEX:");
      Serial.println(comdata[i], HEX);*/

    i++;
  }

  //Serial.println("char:"+comdata);
  if (comdata[10] == 0 and comdata[1] == -17) {
    //成功

    // @!
    Serial.println("Success");

    return "Success";
  } else if (comdata[10] == 01) {
    //串口错误

    // @!
    Serial.println("Serial_Err");

    return "Serial_Err";
  } else if (comdata[10] == 02) {
    //读取不到手指

    // @!
    Serial.println("No_Finger");

    return "No_Finger";
  } else {
    //其他错误

    // @!
    Serial.println("Err");

    return "Err";
  }

}

//比对指纹
boolean match_Touch_ID_Moniter() {
  //定义局部变量
  char comdata[20];
  int i = 1;

  //清空串口缓存
  Serial.flush();
  Serial2.flush();

  delay(150);

  //预置字段并写入
  unsigned char hexdata[12] = {0xEF , 0x01 , 0xFF , 0xFF , 0xFF , 0xFF , 0x01 , 0x00 , 0x03 , 0x03 , 0x00 , 0x07};
  Serial2.write(hexdata, 12);

  delay(1000);

  // @！
  Serial.print("Match:");
  //Serial.println(Serial2.available());
  while (Serial2.available() > 0) {
    //当收到反馈
    comdata[i] = Serial2.read();
    delay(2);

    // @！
    /*Serial.println(i);
      Serial.println("HEX:");
      Serial.println(comdata[i], DEC);*/

    i++;
  }
  //Serial.println("char:"+comdata);
  if (comdata[10] == 0) {
    //比对成功

    // @！
    Serial.println("Success");
    return true;
  } else {
    //失败

    // @！
    Serial.println("Err");
    return false;
  }
}

//搜索指纹
// @ in bufferid
char serach_Touch_ID_Moniter(int bufferid) {
  //定义局部变量
  char comdata[20];
  int i = 1;

  //清空串口缓存
  Serial.flush();
  Serial2.flush();

  delay(150);

  //预置指令并写入
  if (bufferid == 1) {
    unsigned char hexdata[17] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01 , 0x00 , 0x08 , 0x04 , 0x01 , 0x00 , 0x00 , 0x00 , 0x10 , 0x00 , 0x1E};
    Serial2.write(hexdata, 17);
  } else {
    unsigned char hexdata[17] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01 , 0x00 , 0x08 , 0x04 , 0x02 , 0x00 , 0x00 , 0x00 , 0x10 , 0x00 , 0x1F};
    Serial2.write(hexdata, 17);
  }

  delay(1000);

  // @!
  Serial.print("Serach:");
  //Serial.println(Serial2.available());
  while (Serial2.available() > 0) {
    //收到反馈
    comdata[i] = Serial2.read();

    delay(5);

    // @!
    /*Serial.println(i);
      Serial.println("HEX:");
      Serial.println(comdata[i], DEC);*/

    i++;
  }
  //Serial.println("char:"+comdata);
  if (comdata[10] == 0) {
    Serial.println("Serached");
    return comdata[12];
  } else {
    Serial.println("No_Sercah");
    return "Err";
  }
}

//将Flash内指纹添加到Buffer
boolean add_flash_Touch_ID_Moniter(int id, int bufferid) {
  //定义局部变量
  char comdata[20];
  int i = 1;

  //清空串口缓存
  Serial.flush();
  Serial2.flush();

  delay(150);

  //预置指令码并写入指令码
  if (bufferid == 1) {
    int to = 15 + id;
    unsigned char hexdata[15] = {0xEF , 0x01 , 0xFF , 0xFF , 0xFF , 0xFF , 0x01 , 0x00 , 0x06 , 0x07 , 0x01 , 0x00 , id , 0x00 , to};
    Serial2.write(hexdata, 17);
  } else {
    int to = 16 + id;
    unsigned char hexdata[15] = {0xEF , 0x01 , 0xFF , 0xFF , 0xFF , 0xFF , 0x01 , 0x00 , 0x06 , 0x07 , 0x02 , 0x00 , id , 0x00 , to};
    Serial2.write(hexdata, 17);
  }

  delay(1000);

  // @!
  Serial.print("Flash:");
  //Serial.println(Serial2.available());
  while (Serial2.available() > 0) {
    //当收到返回时
    comdata[i] = Serial2.read();

    delay(5);

    // @!
    /*Serial.println(i);
      Serial.println("HEX:");
      Serial.println(comdata[i], DEC);*/

    i++;
  }
  //Serial.println("char:"+comdata);


  if (comdata[10] == 0) {
    //添加成功

    // @!
    Serial.println("Added");

    return true;
  } else {
    //添加失败

    // @!
    Serial.println("Err");

    return false;
  }
}

//@ 功能
/*
   @ 监听
   return Card_ID or Touch_ID
*/
String Listen() {


  if (rfid.isCard()) {
    //如果读到卡

    //声明状态变量改变
    If_Card = true;

    // @！
    Serial.println("Find the card!");

    //读取卡序列号
    if (rfid.readCardSerial()) {

      // @！
      Serial.print("Read card's number is  : ");
      int q = 0;
      do {
        Get_ID = Get_ID + (String)rfid.serNum[q];
        q++;
      } while (q < 5);

      // @！
      Serial.println(Get_ID);

      //LCD提示等待
      LCD_Show_Wait();

      //蜂鸣器发声
      Buzzer_Show_Getting();

      //全局变量修改
      Card_ID = Get_ID;

      //蜂鸣器提示接受
      Buzzer_Show_Getting();

      //返回刷卡成功
      return "Card_Success";
    }

  } else if (digitalRead(Touch_ID_Moniter) == LOW) {
    //如果读到指纹

    //变量声明
    int Touch_ID_Searched;
    String Touch_State;

    //全局变量状态更新
    If_Card = true;

    // @！
    Serial.println("Touch!");

    //LCD提示等待
    LCD_Show_Wait();

    //读取指纹
    Touch_State = get_Touch_ID_Moniter();
    if (Touch_State == "Success") {
      //下一步验证

      //添加到ID为1的Buffer
      Touch_State = add_buff_Touch_ID_Moniter(1);
      if (Touch_State == "Success") {
        //下一步验证

        //搜索ID为1的Buffer的指纹是否存在并取出ID
        Touch_ID_Searched = serach_Touch_ID_Moniter(1);

        if (Touch_ID_Searched != char("Err")) {
          //如果搜索到ID

          // @!
          Serial.print("Touch_ID_Searched:");
          Serial.println(Touch_ID_Searched);

          //将取出ID的指纹放到ID为2的Buffer
          Touch_State = add_flash_Touch_ID_Moniter(Touch_ID_Searched, 2);

          if (Touch_ID_Searched != "Err") {
            //下一步验证
            Buzzer_Show_Getting();

            delay(100);

            //比对两个指纹
            if (match_Touch_ID_Moniter()) {

              // @!
              Serial.println("Pass");

              //更新全局变量
              Touch_ID = Touch_ID_Searched;

              //蜂鸣器提示接受
              Buzzer_Show_Getting();

              delay(1000);

              //返回指纹验证通过
              return "Touch_Success";
            } else {
              //比对不通过
              LCD_Show_Err("Match_T_F", "Flash Match Err");

              //蜂鸣器提示失败
              Buzzer_Show_Fail();

              delay(1000);

              return "Flash_Match_Err";
            }
          } else {
            //将ID取出到Buffer失败
            LCD_Show_Err("Add_F_B", Touch_State);

            //蜂鸣器提示失败
            Buzzer_Show_Fail();

            delay(1000);

            return "Touch_Fail";
          }
        } else {
          //没有搜索到ID
          LCD_Show_Err("Serach_T", "No Touch ID");

          //蜂鸣器提示失败
          Buzzer_Show_Fail();

          delay(1000);

          return "Touch_Fail";
        }

      } else {
        //添加到Buffer报错
        LCD_Show_Err("Add_T_B", Touch_State);

        //蜂鸣器提示失败
        Buzzer_Show_Fail();

        delay(1000);

        return "Touch_Fail";
      }

    } else {
      //读取报错
      LCD_Show_Err("Read_T", Touch_State);

      //蜂鸣器提示失败
      //考虑可能会出现接触不良，噪声影响，暂时关闭蜂鸣器
      Buzzer_Show_Fail();

      delay(1000);

      return "Touch_Fail";
    }

  } else {
    //没有动作

    return "No_Action";
  }

}

/*
   @ 二次验证
   in ID and Type
*/
bool Check_Again(String ID, String Type, int suffix) {
  //定义局部变量
  unsigned long Last_Time = millis();
  String Listen_State;
  while ((millis() - Last_Time) < 5000) {
    //5秒内有效
    Listen_State = Listen();

    if (Listen_State != "No_Action") {

      //有动作
      if (Listen_State == "Card_Success" and Type == "Touch_Success") {
        //原来是指纹后刷卡
        if (User_ID[suffix - 1] == Card_ID) {
          //验证通过
          return true;
        } else {
          //验证失败

          //显示错误
          LCD_Show_Err("It not match", "Card Touch Wrong");

          //蜂鸣器提示失败
          Buzzer_Show_Fail();

          delay(2000);

          return false;
        }
      } if (Listen_State == "Touch_Success" and Type == "Card_Success") {
        //原来是刷卡后指纹
        if (User_ID[suffix + 1] == Touch_ID) {
          //验证通过
          return true;
        } else {
          //验证失败

          //显示错误
          LCD_Show_Err("It not match", "Card Touch Wrong");

          //蜂鸣器提示失败
          Buzzer_Show_Fail();

          delay(2000);

          return false;
        }
      } else {

        //两次指纹或刷卡

        //显示错误
        LCD_Show_Err("Repeat", "Repeat ID");

        //蜂鸣器提示失败
        Buzzer_Show_Fail();

        delay(2000);

        return false;
      }
    }
  }

  //显示错误
  LCD_Show_Err("Time Out", "Time out!");

  //蜂鸣器提示失败
  Buzzer_Show_Fail();

  delay(2000);

  return false;

}

/*
  @ 开门
  开门10s
*/
void Open_Door() {
  digitalWrite(Doorpin, HIGH);
  Door_Opened = true;
  delay(10000);
  digitalWrite(Doorpin, LOW);
  Door_Opened = false;
}