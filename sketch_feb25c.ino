#include <SPI.h>
#include <RFID.h>
#include <stdio.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>


/*
 * 定义引脚
 */

LiquidCrystal_I2C lcd(0x27, 16, 2);
//D10--读卡器SS引脚、D5--读卡器RST引脚
RFID rfid(53, 5); 
int Touch_ID_Moniter = 4;
int Buzzer = A8;

//预置超管
String AdminID[] = {"28118121122105", "20218013195176", "0","1", "2", "3", "4", "5","6", "7", "8", "9", "10"};

//定义服务器结构体数据类型
struct Data {
  String name;
  String englishname;
  String id;
  String Touch_ID_Moniter[5];
  String card_id;
  String allowed_time;
  String department;
  String post;
};

//定义卡号变量
String id;
String cardId;

//定义系统变量
bool if_Card = false;
bool Door_Opened;
bool Just_Door_Opened;


void setup()
{
  lcd.init();
  //初始化LCD屏幕
  lcd.backlight();
  lcd.setCursor(1, 0);
  lcd.print("IT CLUB SYSTEM");
  lcd.setCursor(4, 1);
  lcd.print("Starting");

  //初始化串口设置
  Serial.begin(9600);
  Serial3.begin(115200);
  Serial2.begin(115200);

  //初始化接口
  SPI.begin();
  rfid.init();
  pinMode(Buzzer, OUTPUT);

  //初始化系统变量
  Door_Opened = false;
  Just_Door_Opened = false;
  if_Card = false;
}

void Listen() {

  //如果读到卡
  if (rfid.isCard()) {

    //显示等待子程序
    LCD_Show_Wait();

    //声明状态变量改变
    if_Card = true;

    Serial.println("Find the card!");

    //读取卡序列号
    if (rfid.readCardSerial()) {
      Serial.println("Read card's number is  : ");
      int q = 0;
      do {
        id = id + (String)rfid.serNum[q];
        q++;
      } while (q < 5);

      Serial.println(id);

      //声明全局变量
      cardId = id;

      //LCD显示状态改变
      lcd.setCursor(1, 0);
      lcd.print("IT CLUB SYSTEM");
      lcd.setCursor(3, 1);
      lcd.print("Connecting");

      //蜂鸣器发声
      Buzzer_On(180, 3500);
    }
    //锁定读卡
    rfid.selectTag(rfid.serNum);
  }
  //休眠卡牌片
  rfid.halt();
  
  /*
    //如果读到指纹
    if (digitalRead(Touch_ID_Moniter) == LOW) {
      int got;
      if_Card = true;
      Serial.println("Touch!");
      LCD_Show_Wait();

      if (get_Touch_ID_Moniter() == "OK!") {
        Buzzer_On(150, 3500);
        if (add_buff_Touch_ID_Moniter(1) == "OK!") {
          Buzzer_On(150, 3500);
          got = serach_Touch_ID_Moniter(1);
          Serial.println("IIIIIIIIIID:");
          Serial.println(got);
          if (add_flash_Touch_ID_Moniter(got, 2)) {
            delay(100);
            if (match_Touch_ID_Moniter()) {
              Serial.println("Pass!!");
              lcd.setCursor(1, 0);
              lcd.print("IT CLUB SYSTEM");
              lcd.setCursor(3, 1);
              lcd.print("Connecting");
              id = got;
            }
          }
        } else {
          lcd.setCursor(1, 0);
          lcd.print("IT CLUB SYSTEM");
          lcd.setCursor(3, 1);
          lcd.print("Buffer Err");
          Buzzer_On(1000, 3500);
        }
      } else {
        Buzzer_On(1000, 3500);
        lcd.setCursor(1, 0);
        lcd.print("IT CLUB SYSTEM");
        lcd.setCursor(4, 1);
        lcd.print("No Finger");
      }
    }*/


}
void loop()
{
  id = "";
  cardId = "";
  if_Card = false;
  //lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("IT CLUB SYSTEM");
  lcd.setCursor(1, 1);
  lcd.print("Touch or Swing");
  /* while (if_Card == false) {
     listenin();
    }*/
  if (id != "") {
    delay(100);
    Data back;
    back = getServerData("1000100010003");
    Serial.println(back.name);
    if (back.name != "Err") {
      Serial.println(back.Touch_ID_Moniter[1]);
      //开启第二轮监听
      int starttime = millis();
      if_Card = false;
      while ((millis() - starttime) < 10000) {
        listenin();
      }
      if (  if_Card == true) {
        //取得第二轮的卡号或指纹ID后
      } else {
        //未刷卡或刷卡超时，验证不通过
      }
    } else  {
      //验证超管
      int p = 0;
      while (p < 15) {
        if (AdminID[p] == id) {
          //是超管，执行超管动作
          Serial.println("admin!");
          delay(300);
          Buzzer_On(100, 3500);
          delay(80);
          Buzzer_On(100, 3500);
          lcd.clear();
          lcd.setCursor(4, 0);
          lcd.print("Welcome!");
          lcd.setCursor(5, 1);
          lcd.print("Admin");
          delay(1000);
          Just_Door_Opened = true;
          break;
        }
        p++;
      }
    }
    //判断刷卡后是否开门了
    if (Just_Door_Opened == false) {
      lcd.clear();
      lcd.setCursor(5, 0);
      lcd.print("Sorry!");
      lcd.setCursor(2, 1);
      lcd.print("No Permission");
      delay(1000);
    }
  }
}

void LCD_Show_Wait() {
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("IT CLUB SYSTEM");
  lcd.setCursor(4, 1);
  lcd.print("Waiting.");
}

struct Data getServerData(String sign) {
  Serial3.flush();
  int randoms = random(100999, 999999);
  randoms = abs(randoms - random(111, 999) + random(122, 451));
  String putData = "sign_" + sign + ".code_" + randoms + ".appkey_s21S210erg785487GR4r8ri745th15R87";
  Serial3.println(putData);
  delay(2000);
  Data data;//声明一个返回的容器
  String comdata = "";
  int cutPosition;
  String info[20];
  int i = 1;
  while (Serial3.available() > 0) {
    comdata += (char)Serial3.read();
    delay(2);
  }
  Serial.println("Get:" + comdata);
  if (comdata.length() != 0) {
    //Serial.println("Get:" + comdata);
    do {
      cutPosition = comdata.indexOf("_");
      if (cutPosition > 0) {
        info[i] = comdata.substring(0, cutPosition);
        comdata = comdata.substring(cutPosition + 1, comdata.length());
      } else {
        if (comdata.length() > 0) {
          info[i] = comdata;
        }
      }
      i++;
    } while (cutPosition >= 0);
    //Serial.println(info[6]);
    //char token;
    i = 1;
    //token=((randoms + 2017) / 3) + ((randoms + 2017) % 3);
    //Serial.println(token,DEC);
    //if (info[9] == token) {
    String touchidget[5];
    do {
      cutPosition = info[4].indexOf(",");
      if (cutPosition > 0) {
        touchidget[i] = info[4].substring(0, cutPosition);
        info[4] = info[4].substring(cutPosition + 1, comdata.length());
      } else {
        if (comdata.length() > 0) {
          touchidget[i] = info[4];
        }
      }
      i++;
    } while (cutPosition >= 0);
    if (info[2] != "Err") {
      data.name = info[2];
      data.englishname = info[3];
      data.id = info[1];
      i = 1;
      while (i <= 5) {
        data.Touch_ID_Moniter[i] = touchidget[i];
        i++;
      }
      data.card_id = info[5];
      data.allowed_time = info[8];
      data.department = info[6];
      data.post = info[7];
    } else {
      data.name = "Err";
    }
    /*} else {
      data.name = "Err";
      }*/
  } else {
    data.name = "Err";
  }

  return data;
}
//读指纹到Pic区
String get_Touch_ID_Moniter() {
  Serial.flush();
  delay(150);
  unsigned char hexdata[12] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x03, 0x01, 0x00, 0x05};
  Serial2.write(hexdata, 12);
  delay(100);
  char comdata[20];
  int i = 1;
  Serial.println("Get Touch:");
  //Serial.println(Serial2.available());
  while (Serial2.available() > 0) {
    comdata[i] = Serial2.read();
    delay(2);
    //Serial.println(i);
    //Serial.println("HEX:");
    //Serial.println(comdata[i], DEC);
    i++;
  }
  //Serial.println("char:"+comdata);

  if (comdata[10] == 0) {
    Serial.println("OK!");
    return "OK!";
  } else if (comdata[10] == 01) {
    Serial.println("Serial Err");
    return "Serial Err";
  } else if (comdata[10] == 02) {
    Serial.println("No finger");
    return "No finger";
  } else {
    Serial.println("Err");
    return "Err";
  }
}
//添加指纹到buff
String add_buff_Touch_ID_Moniter(int bufferid) {
  Serial.flush();
  delay(150);
  if (bufferid == 1) {
    unsigned char hexdata[13] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x04, 0x02, 0x01, 0x00, 0x08};
    Serial2.write(hexdata, 13);
  } else {
    unsigned char hexdata[13] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x04, 0x02, 0x02, 0x00, 0x09};
    Serial2.write(hexdata, 13);
  }
  delay(500);
  char comdata[20];
  int i = 1;
  Serial.println("Add Buffer:");
  Serial.println(Serial2.available());
  while (Serial2.available() > 0) {
    comdata[i] = Serial2.read();
    delay(2);
    Serial.println(i);
    Serial.println("HEX:");
    Serial.println(comdata[i], DEC);
    i++;
  }
  //Serial.println("char:"+comdata);

  if (comdata[10] == 0 and comdata[1] == -17) {
    Serial.println("OK!");
    return "OK!";
  } else if (comdata[10] == 01) {
    Serial.println("Serial Err");
    return "Serial Err";
  } else if (comdata[10] == 02) {
    Serial.println("No finger2");
    return "No finger";
  } else {
    Serial.println("finger Err");
    return "finger Err";
  }
}
//比对指纹
boolean match_Touch_ID_Moniter() {
  Serial.flush();
  unsigned char hexdata[12] = {0xEF , 0x01 , 0xFF , 0xFF , 0xFF , 0xFF , 0x01 , 0x00 , 0x03 , 0x03 , 0x00 , 0x07};
  Serial2.write(hexdata, 12);
  delay(100);
  char comdata[20];
  int i = 1;
  Serial.println("Match:");
  Serial.println(Serial2.available());
  while (Serial2.available() > 0) {
    comdata[i] = Serial2.read();
    delay(2);
    Serial.println(i);
    Serial.println("HEX:");
    Serial.println(comdata[i], DEC);
    i++;
  }
  //Serial.println("char:"+comdata);

  if (comdata[10] == 0) {
    Serial.println("OK!");
    return true;
  } else {
    Serial.println("No");
    return false;
  }
}
//搜索指纹
char serach_Touch_ID_Moniter(int bufferid) {
  Serial.flush();
  delay(150);
  if (bufferid == 1) {
    unsigned char hexdata[17] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01 , 0x00 , 0x08 , 0x04 , 0x01 , 0x00 , 0x00 , 0x00 , 0x10 , 0x00 , 0x1E};
    Serial2.write(hexdata, 17);
  } else {
    unsigned char hexdata[17] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01 , 0x00 , 0x08 , 0x04 , 0x02 , 0x00 , 0x00 , 0x00 , 0x10 , 0x00 , 0x1F};
    Serial2.write(hexdata, 17);
  }
  delay(500);
  char comdata[20];
  int i = 1;
  Serial.println("Serach:");
  Serial.println(Serial2.available());
  while (Serial2.available() > 0) {
    comdata[i] = Serial2.read();
    delay(5);
    Serial.println(i);
    Serial.println("HEX:");
    Serial.println(comdata[i], DEC);
    i++;
  }
  //Serial.println("char:"+comdata);

  if (comdata[10] == 0) {
    Serial.println("Serached!");
    return comdata[12];
  } else {
    Serial.println("No Sercah");
    return "No Sercah";
  }
}
boolean add_flash_Touch_ID_Moniter(int id, int bufferid) {
  Serial.flush();
  delay(150);
  if (bufferid == 1) {
    int to = 15 + id;
    unsigned char hexdata[15] = {0xEF , 0x01 , 0xFF , 0xFF , 0xFF , 0xFF , 0x01 , 0x00 , 0x06 , 0x07 , 0x01 , 0x00 , id , 0x00 , to};
    Serial2.write(hexdata, 17);
  } else {
    int to = 16 + id;
    unsigned char hexdata[15] = {0xEF , 0x01 , 0xFF , 0xFF , 0xFF , 0xFF , 0x01 , 0x00 , 0x06 , 0x07 , 0x02 , 0x00 , id , 0x00 , to};
    Serial2.write(hexdata, 17);
  }
  delay(500);
  char comdata[20];
  int i = 1;
  Serial.println("Flash:");
  Serial.println(Serial2.available());
  while (Serial2.available() > 0) {
    comdata[i] = Serial2.read();
    delay(5);
    Serial.println(i);
    Serial.println("HEX:");
    Serial.println(comdata[i], DEC);
    i++;
  }
  //Serial.println("char:"+comdata);

  if (comdata[10] == 0) {
    Serial.println("Added!");
    return true;
  } else {
    Serial.println("No Add!");
    return false;
  }
}
void Buzzer_On(int times, int diao) {
  tone(Buzzer, diao);
  delay(times);
  noTone(Buzzer);
}
