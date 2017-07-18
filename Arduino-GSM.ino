#include <DHT.h>

#include <avr/wdt.h> 
#include <string.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <CyberLib.h>

  // Set the LCD address to 0x27 for a 16 chars and 2 line display
  LiquidCrystal_I2C lcd(0x27, 16, 2); 


#define DHTPIN 2     // вывод, к которому подключается датчик
#define DHTTYPE DHT22   // DHT 22  (AM2302)

DHT dht(DHTPIN, DHTTYPE);


  char command_in[20];
  char data_in[200];
  char unitID_in[10]; 

  boolean Enter0=false;
  boolean echo0=true;
  boolean CommandExist=false;
  int buf_i0=0;
  char buff_all0[250];
  char buf0=0;
  unsigned long TimerSerial0 = 0;

  boolean Enter1=false;
  boolean echo1=true;
  boolean CommandExist1=false;
  int buf_i1=0;
  char GSM_buf[250];
  char buf1=0;
  unsigned long TimerSerial1 = 0;


  unsigned long TimerZamer = 0;

  float Balance=0.0;
  float LastBalance=0.0;





// Таймеры
  unsigned long TimerBalance = 10000;             // Первый, стартовый толчек (отсчет) такта
  unsigned long TimerEverySecond = 1000;
  unsigned long TimerTemperature = 1000;
  unsigned long TimerDateTime = 10000;
  unsigned long TimerModemStatus = 500;
  unsigned long TimerLifeUpTime = 0;
  unsigned long TimerDHT22 = 500;
  unsigned long TimerModemPower=0;
  unsigned long TimerSerialEvents1=0;
  unsigned long TimerAtExecute=0;

// Интервалы
  unsigned long IntervalGetBalance = 60000;       // Интервал запроса баланса
  unsigned long IntervalGetTemperature  = 30000;  // Интервал запроса температуры
  unsigned long IntervalGetDateTime=900;          // Интервал запроса Даты и Время
  unsigned long IntervalGetModemStatus=500;       // Интервал запроса статуса модема
  unsigned long IntervalLifeUpTime=300;           // Интервал жизни Arduino
  unsigned long IntervalDHT22=30000;              // Интервал опроса датчика DHT22
  unsigned long IntervalModemPower=100;
  unsigned long IntervalSerialEvents1=1500;
  
// Таймауты
  unsigned long TimeOutGetAlgoritm=0;             // Предельное время выполнения такта
  unsigned long TimeOutGetBalance=0;
  unsigned long TimeOutGetDateTime=0;
  unsigned long TimeOutGetModemStatus=0;
  unsigned long TimeOutSerialEvents1=0;


  
  unsigned long GetAlgoritmSteps[4]={0, 0, 0, 0};
  int GetBalanceSteps[2]={0, 0};
  int GetDateTimeSteps[2]={0, 0};

  int MaxGetAlgoritmSteps=15;
  int ForceToNextStep=0;
  
  String WaintSerialResult1 = "";
  String WaintSerialAlternativeResult1 = "";
  String WaintSerialResult2 = "";
  String WaintSerialResult3 = "";

  String ReturnOKResult = "";


  String strSerial1="";
  
  String DateTime = "";
  int ModemStatus = 2;
        // Информация о состояние модуля
        // 0 – готов к работе
        // 2 – неизвестно
        // 3 – входящий звонок
        // 4 – голосовое соединение
  int ModemPowerStatus = 0;
  //boolean ModemPowerON = false;


                
  
  String BufferCommandToSerial1="";

  boolean StopAll=true;
  
  
  // Переменные на все случаи жизни
  int tmp_n1=0;
  int tmp_n2=0;
  int tmp_n3=0;
  String tmp_s1="";
  String tmp_s2="";
  String tmp_s3="";



  
  String LastSteps="";
  String TekSteps="";
  char Steps[3]={'0', '0', '0'};

  String input = "";
  

  int LOG_Rotation=4;
  //int LOG_Rotation=0;
  // 0 - Debug
  // 1 - Full
  // 2 - Information
  // 3 - Warning
  // 4 - Error
  // 5 - Critical, Fatal


  char vDateYY[3]={'1', '7', '\0'};
  char vDateMM[3]={'0', '1', '\0'};
  char vDateDD[3]={'0', '1', '\0'};
  char vTimeHH[3]={'0', '0', '\0'};
  char vTimeMM[3]={'0', '0', '\0'};
  char vTimeSS[3]={'0', '0', '\0'};


  boolean EnableSerialEvents0=true;
  boolean EnableSerialEvents1=true;




  
 
void setup() {
  // открываем порт 
  Serial.begin(57600); 
  Serial1.begin(19200);
  
  D13_Out;
  D13_Low;
  
  D6_In;
  
  dht.begin();


  lcd.init(); 
  lcd.backlight();
  lcd.print ("Hello!!");
}

    // Подключение к пину Reset GSM-шилда
    boolean ModemPowerON(){
        if (D6_Read==1){
            return true;
        } else {
            return false;
        }
    }

    // Замена подстроки, всех вхождений
    void ReplaceString (String &SourceString, String subString1, String subString2){
        //  & - ссылка
        //  * - указатель
        if (SourceString.length()>0){
            while (SourceString.indexOf(subString1)>=0){
                SourceString.replace(subString1, subString2);
            }
        }
    }

    // Выполнить AT комманду, и дождаться ответ
    String AtExecute (String Command="", unsigned long TimeOutAtExecute=1500, boolean OnlyRead = false){
        if (ModemPowerON()==true){

            //Serial.println("************************");
            //Serial.print("Execute AT Command: ");Serial.println(Command);
            //Serial.print("Execute AT TimeOutAtExecute: ");Serial.println(TimeOutAtExecute);
            //Serial.print("Execute AT OnlyRead: ");Serial.println(OnlyRead,DEC);


            
            if (LOG_Rotation < 4){
                Serial.println("************************");
                Serial.print("Execute AT Command: ");Serial.println(Command);
            }

            if (OnlyRead==false){
                Serial1.flush();
                Serial1.println(Command);
            }

            tmp_s1="";
            tmp_s2="";

            // Установка таймаута
            TimerAtExecute = millis()+TimeOutAtExecute;
            
            while (TimerAtExecute > millis())
            {
                if (LOG_Rotation < 4){
                  //Serial.print("TimerAtExecute > millis(); ");Serial.print(TimerAtExecute);Serial.print(" > ");Serial.println(millis());
                }
                
                while (Serial1.available() > 0)
                {
                    tmp_s1 = Serial1.readStringUntil('\n');
                    
                    if (tmp_s1.length()>0){
                        for(tmp_n1=0;tmp_n1<tmp_s1.length();tmp_n1++){
                            if ((tmp_s1[tmp_n1]<1) || (tmp_s1[tmp_n1]==13)){
                              tmp_s1[tmp_n1]='|';
                            }
                        }
                    }
                    
                    if (tmp_s1.length()>0){
                        if (LOG_Rotation < 4){
                            Serial.print("FULL_BUF: ");Serial.println(tmp_s1);
                            int tmp_n2=0;
                            Serial.print("FULL_BUF(2): ");
                            for(tmp_n1=0;tmp_n1<tmp_s1.length();tmp_n1++){
                              tmp_n2=tmp_s1[tmp_n1];
                              Serial.print(tmp_s1[tmp_n1]);Serial.print("(");Serial.print(tmp_n2);Serial.print("), ");
                            }
                            Serial.println();
                        }
          
                        tmp_s2 += tmp_s1;
                        tmp_s2 += "|";
                        tmp_s1="";
                    }
                    
                    // Сброс таймаута, до тех пор пока поступают данные, или пока не придет OK, или ERROR
                    TimerAtExecute = millis()+TimeOutAtExecute;
                }



                if (tmp_s2.length()>0){
                    ReplaceString(tmp_s2, "||", "|");
                    
                    if ((tmp_s2.indexOf("OK") >= 0) || (tmp_s2.indexOf("ERROR") >= 0)) {

                        if (LOG_Rotation < 4){
                            Serial.println("OK or ERROR Execute AT Command ");
                            Serial.println("*#*#*#*#*#*#*#*#*#*#*#*#");
                        }
                        
                        return tmp_s2;
                        tmp_s2 = "";

                        break;

                    } else if (millis() >= TimerAtExecute) {
                        if (LOG_Rotation < 4){
                            Serial.println("TimeOut Execute AT Command");
                            Serial.println("*#*#*#*#*#*#*#*#*#*#*#*#");
                        }
                        
                        return "(t)" + tmp_s2;
                        tmp_s2 = "";  
                        

                        break;
                        
                                      
                    }
                }



            } // END TimerAtExecute


            
            if (LOG_Rotation < 4){
                //Serial.print("TimerAtExecute > millis(); ");Serial.print(TimerAtExecute);Serial.print(" > ");Serial.println(millis());
                Serial.println("Flush Execute AT Command");
                Serial.println("*#*#*#*#*#*#*#*#*#*#*#*#");
            }
            return "";


            
        } // END - ModemPowerON
    } // END -Void AtExecute

    

int SerialEvents0 (){
//если есть данные - читаем
  if(Serial.available()){
     delay(10);
      
     //загоняем прочитанное в буфер
     while( Serial.available() && buf_i0< 249) {
        buf0=Serial.read();

        if (('\n' != buf0) && ('\r' != buf0)) {
          buff_all0[buf_i0++] = buf0;
          if (echo0){Serial.print(buf0);};
         } else {
            buff_all0[buf_i0++]='\0';
            Enter0=true;
            Serial.println("");
        }
        delay(1);
     }
     //закрываем массив
     
  }
 
//если буфер наполнен
  if ((buf_i0>0) && (Enter0==true)){  

    CommandExist=true;
    
    memset(&command_in, 0, sizeof(command_in));
    memset(&data_in, 0, sizeof(data_in));
    memset(&unitID_in, 0, sizeof(unitID_in));
    
    //разбераем его на части отделенные запятой
    sscanf(buff_all0, "%[^';'];%[^';'];%s", &command_in, &data_in, &unitID_in);

    Serial.println("~~~~~Serial0~~~~");
    Serial.print ((String)command_in);Serial.print(";");
    Serial.print ((String)data_in);Serial.print(";");
    Serial.print ((String)unitID_in);Serial.println(";");
    Serial.println("################");
    

  if ((String)command_in == "on"){
      digitalWrite(13, HIGH);
      Serial.println("ON LED;");
      lcd.backlight();
      CommandExist=false;
  } else if ((String)command_in == "off"){
        digitalWrite(13, LOW);
        lcd.noBacklight();
        Serial.println("OFF LED;");
        CommandExist=false;
  } else if ((String)command_in == "echo"){
      CommandExist=false;
      if ((String)data_in == "on"){
        echo0=true;
        Serial.println("Echo On;");
      } else if ((String)data_in == "off"){
        echo0=false;
        Serial.println("Echo Off;");
      } else {
        Serial.print("Echo=");Serial.println(echo0);
      }
  } else if ((String)command_in == "help"){
      CommandExist=false;  
      Serial.println("Help Doccumentation;");
      Serial.println("~~~~~~~~~~~~~~~~~;");
      Serial.println(" log;");
      Serial.println(" ping;");
      Serial.println(" echo;");
  } else if ((String)command_in == "log"){
      CommandExist=false;  
      Serial.print("LogLevel=");Serial.println(LOG_Rotation);
      Serial.println("~~~~~~~~~~~~~~~~~;");
      Serial.println(" 0 - Debug;");
      Serial.println(" 1 - Full;");
      Serial.println(" 2 - Information;");
      Serial.println(" 3 - Warning;");
      Serial.println(" 4 - Error;");
      Serial.println(" 5 - Critical, Fatal;");
 
  } else if ((String)command_in == "log0"){
      CommandExist=false;  
      LOG_Rotation=0;
      Serial.print("SET LogLevel=");Serial.println(LOG_Rotation);
  } else if ((String)command_in == "log4"){
      CommandExist=false;  
      LOG_Rotation=4;
      Serial.print("SET LogLevel=");Serial.println(LOG_Rotation);
  
  } else if ((String)command_in == "ping"){
      CommandExist=false;  
      Serial.println("pong");
  } else if ((String)command_in == "datetime"){
      CommandExist=false;  
      Serial.print(vDateYY);Serial.print("-");Serial.print(vDateMM);Serial.print("-");Serial.print(vDateDD);Serial.print(", ");
      Serial.print(vTimeHH);Serial.print(":");Serial.print(vTimeMM);Serial.print(":");Serial.print(vTimeSS);Serial.println("");              
            
  } else if (((String)command_in == "balans") || ((String)command_in == "balanc")){
      CommandExist=false;  
      //Serial1.println("AT+CUSD=1,\"#100#\"");
      GetBalance();

  } else if ((String)command_in == "stop"){
      CommandExist=false;  
      StopAll=true;
      Serial.println("Stop all process!");
  } else if ((String)command_in == "start"){
      CommandExist=false;  
      StopAll=false;      
      Serial.println("Start all process!");

      

  } 

  
    


    if (CommandExist){


      //if ((EnableSerialEvents1==true) && (TekSteps=="")){
      if (TekSteps==""){
          Serial.print("Send Command: "); Serial.print(buff_all0);Serial.println();
          //Serial1.println(buff_all0);
          Serial.println(AtExecute(buff_all0));
      } else {
          BufferCommandToSerial1=buff_all0;
      }
    }







    memset(&buff_all0, 0, sizeof(buff_all0));
    buf_i0=0;
    Enter0=false;
  }

    //if ((EnableSerialEvents1==true) && (TekSteps=="")){
    if (TekSteps==""){
        if (BufferCommandToSerial1.length()>0){
            Serial.print("Send Command: "); Serial.print(BufferCommandToSerial1);Serial.println();
            //Serial1.println(BufferCommandToSerial1);
            Serial.println(AtExecute(BufferCommandToSerial1));
            BufferCommandToSerial1="";
        }
    }
    
}

 int InStr(const char *text, const char *pattern) {
    if (strlen(text) < strlen(pattern)) return -1;//проверка длины
    if (*text == '\0' && *pattern == '\0') return 0; // если пустые строки  
    else if (*pattern != '\0'&& *text == '\0') return -1; 
    else if (*text != '\0' && *pattern == '\0') return 0;
    bool temp = true;
    const char *t = text;
    const char*p = pattern;
    int i = 0;
    for (; *text; text++) {
        t = text; p = pattern; i++;
        while (*p && *t) {
            temp = true;
            if (*p != *t) { temp = false;  break; }
            p++; t++;
        }
        if (temp) return i-1;
    }

    return -1;
 }


char* Mid(const char* src, size_t start, size_t len)
{
   //char* dest = (char*)calloc(len + 1, sizeof(char));
    char *dest = (char *) malloc(len + 1);

    strncpy(dest, src + start, len);
    dest[len] = '\0';
    return dest;
}

void trim(char *s)
{
     // удаляем пробелы и табы с начала строки:
     int i=0,j;
     while((s[i]==' ')||(s[i]=='\t')) 
     {
           i++;
     }
     if(i>0) 
     {
          for(j=0;j<strlen(s);j++) 
          {
              s[j]=s[j+i];
          }
          s[j]='\0';
     }

     // удаляем пробелы и табы с конца строки:
     i=strlen(s)-1;
     while((s[i]==' ')||(s[i]=='\t')) 
     {
          i--;
     }
     if(i<(strlen(s)-1)) 
     {
          s[i+1]='\0';
     }
}

// Еще набор функций, использующий оператор &&:
void rtrim( char * string, char * trim )
{
     // делаем трим справа
     int i;
     for( i = strlen (string) - 1; 
           i >= 0 && strchr (trim, string[i]) != NULL;
           i-- )
     {   
         // переставляем терминатор строки 
         string[i] = '\0';
     }
}

void ltrim( char * string, char * trim )
{
     // делаем трим слева
     while ( string[0] != '\0' && strchr ( trim, string[0] ) != NULL )
     {
         memmove( &string[0], &string[1], strlen(string) );
     }
}

int SerialEvents1_rab (boolean ForceEvents=false){

      if ((TekSteps=="") || (ForceEvents==true)){
      //если есть данные - читаем
      delay(50);
      if(Serial1.available()){
         delay(100);
         //delay(1);
         
         //загоняем прочитанное в буфер
         while( Serial1.available() && buf_i1< 249) {
            buf1=Serial1.read();
            //buf1=Serial1.peek();
            
            //if (echo1){Serial.print(buf1);};
    
            if (('\n' == buf1) || ('\r' == buf1)) {
                buf1='|';
            }
            
            //if (('\n' != buf1) && ('\r' != buf1) && ('\0' != buf1)) {
            //if ('\0' != buf1) {
              if (buf1=='|'){
                if (!(GSM_buf[buf_i1-1] == buf1)) {
                  GSM_buf[buf_i1++] = buf1;
                }
              }else{
                GSM_buf[buf_i1++] = buf1;
              }
             //} else {
                //GSM_buf[buf_i1++]='\0';
                Enter1=true;
            //}
            //delay(1);
         }
         //закрываем массив
    
    
    
          //если буфер наполнен
          if ((buf_i1>0) && (Enter1==true)){ 
              CommandExist1=true;
    
    
              char *istr;
    
               //Поиск строки
               //istr = strstr (GSM_buf,"+CCLK:");
    
               
               
              istr = "";


              /*
              if (strstr (GSM_buf,"+CCLK:") != NULL) {
                  istr = strstr (GSM_buf,"+CCLK:");
                  if (!strncmp("+CCLK:",istr,6)) {  //Делаем все то же самое с последовательностью символов начинающейся на +CCLK
                      CommandExist1=false;
                      //Serial.println(msg);
                      //+CCLK: "17/05/20,22:05:34+06"
        
                      strncpy(vDateYY, istr+8, 2);vDateYY[2]='\0';
                      strncpy(vDateMM, istr+11, 2);vDateMM[2]='\0';
                      strncpy(vDateDD, istr+14, 2);vDateDD[2]='\0';
                      
                      strncpy(vTimeHH, istr+17, 2);vTimeHH[2]='\0';
                      strncpy(vTimeMM, istr+20, 2);vTimeMM[2]='\0';
                      strncpy(vTimeSS, istr+23, 2);vTimeSS[2]='\0';
                      
                      //Serial.print(vDateYY);Serial.print("-");Serial.print(vDateMM);Serial.print("-");Serial.print(vDateDD);Serial.print(", ");
                      //Serial.print(vTimeHH);Serial.print(":");Serial.print(vTimeMM);Serial.print(":");Serial.print(vTimeSS);Serial.println("");              
    
    
                      
                  } else {
                    Serial.print("GSM_buf CCLK ERROR:");Serial.print(istr);
                  }

              } else if (strstr (GSM_buf,"+CUSD:") != NULL) {
                  istr = strstr (GSM_buf,"+CUSD:");
                  if (!strncmp("+CUSD:",istr,6)) {  //Делаем все то же самое с последовательностью символов начинающейся на +CCLK
                      CommandExist1=false;
                      
                      tmp_n1=0;
                      tmp_n1=InStr(istr,"balans ");
                      
                      if (tmp_n1>0){
                          tmp_n1+=strlen("balans ");
                          tmp_n2=0;
                          tmp_n2=InStr(istr,"rub.");
                          if (tmp_n2>0){
                              //LastBalance=
    
                              Balance=0.0;
                              Balance=atof(Mid(istr,tmp_n1,tmp_n2-tmp_n1)),2;
                              LastBalance=Balance;
    
                             
                             
                              Serial.print("Balance: ");Serial.print(Balance);Serial.println("rub.");
                              lcd.setCursor(7, 0);
                              lcd.print (Balance);lcd.print ("rub");
                          }
    
    
                        
                      }
                      
    
                      
                      
                      //Serial.print("InStr(balans ):");Serial.print(InStr(istr,"balans "));
                      //Serial.print("InStr(rub.):");Serial.print(InStr(istr,"rub."));
                      
    
                      
                  } else {
                    Serial.print("GSM_buf CUSD ERROR:");Serial.print(istr);
                  }


                  
  
                  
    //~~~~~CUSD~~~~
    //istr: +CUSD: 0,"Vash balans -8.33 rub. Bonus: 0.00 rub. Besplatnyh sekund: 0. Dopolnitelnye balansy: #106#",64|
    //****************
    
    
    
    
                
              }
    
      */          




//GSM_buf: |RING|+CLIP: "+7903XXXXXXX",145,"",,"",0|

      
    
              if (CommandExist1){
                Serial.println("~~~~~Serial1~~~~");
                //Serial.print("Enter1: ");Serial.println(Enter1);
                //Serial.print("buf_i1: ");Serial.println(buf_i1);
                Serial.print("GSM_buf: ");Serial.println(GSM_buf);
                //Serial.print("Found2: ");Serial.println(strstr (GSM_buf,"+CCLK:"));
                Serial.println("****************");
              }
              
    
            }
    
         
              //Serial.println("");
              memset(&GSM_buf, 0, sizeof(GSM_buf));
              buf_i1=0;
              Enter1=false;
      
          }
  }
  }  

















        


    int SerialEvents1 (boolean ForceEvents=false){
      if (ModemPowerON()==true){
          if ((TekSteps=="") || (ForceEvents==true)){
              strSerial1="";
              strSerial1 = AtExecute("SerialEvents1",100,true);
              if (strSerial1.length()>0){
                  Serial.println("~~~~~Serial1(1)~~~~~");
                  Serial.print("GSM_buf: ");Serial.println(strSerial1);
                  Serial.println("********************");
                  strSerial1 = "";
              }
          
            } 
        } // END - ModemPowerON
    } // END - Void SerialEvents1



































































    





    // Получаем с модема все подрят, заменяя символы 10 и 13 на вертикальную черту ( | ), для удобного представления
    boolean ReadSerialData(boolean IsFlush=true)
    {
        boolean Return=false;
        delay(100);
        if(Serial1.available()){
            Return=true;
            boolean StopRead = false;
        
            while ((Serial1.available()) && (StopRead==false)) {
                buf1=Serial1.read();
                if (('\n' == buf1) || ('\r' == buf1)) buf1='|';
                input += buf1;
                if (buf1=="/0") StopRead=true;
            }
        
            Serial1.flush();

            if (input.indexOf("OK")>0)
                {
                    Return=true; 
                }
            else if (input.indexOf("ERROR")>0)
                {
                    Return=true;
                }
            else if (input.length()>0)
                {
                  Return=true;
                }
           
        } else {
            Return=false;  
        }

        if (IsFlush==true){
            Serial1.flush();
        }

        return Return;
    } // End - ReadSerialData





    // Вывести текущий шаг TekSteps
    void PrintAlgoritmSteps(){
        Serial.print (TekSteps);Serial.print(GetAlgoritmSteps[0]);
        Serial.print("(");Serial.print(GetAlgoritmSteps[1]);Serial.print("): ");
    }


    // Процедура перехода на следуюющий, конкретный, или вовсе прекраитить все шаги
    void ResetAlgoritmSteps (boolean Continue, int Steps = 0){
        
        if (TekSteps!="GetModemStatus"){
            PrintAlgoritmSteps();
            Serial.println (input);
            Serial.println ("###############################");
        }

        
        if (Continue==true){
            if (Steps>0){
                GetAlgoritmSteps[0]=Steps;
            } else {
                GetAlgoritmSteps[0]++;
            }
        } else {
            GetAlgoritmSteps[0]=0;
            GetAlgoritmSteps[1]=0;
            TekSteps="";
        }

        GetAlgoritmSteps[1]=0;
        GetAlgoritmSteps[3]=0;

        SerialEvents1(true);
        input="";
    } // End - ResetAlgoritmSteps





    // Понять, принятые данные содержат ОК-информацию
    boolean GetLetsOK(){
        boolean ReturnOK=false;

// ReturnOKResult показывает только WaintSerialResult1 или WaintSerialAlternativeResult1, обычно это OK или ERROR

        if (ReadSerialData()==true){
            //delay(20);
            // Проверяем на наличие OK или альтернативы - ERROR
            if (input.indexOf(WaintSerialResult1)>0){
                ReturnOK=true;
                ReturnOKResult=WaintSerialResult1;
            } else {
              if (WaintSerialAlternativeResult1.length()>0){
                if (input.indexOf(WaintSerialAlternativeResult1)>0){
                  ReturnOK=true;
                  ReturnOKResult=WaintSerialAlternativeResult1;
                }
              }
            }
            
            
            if (ReturnOK==true){  // "OK"
              if (WaintSerialResult2.length()>0){
                if (input.indexOf(WaintSerialResult2)>0){
                  if (WaintSerialResult3.length()>0){
                    if (input.indexOf(WaintSerialResult3)==0){
                      ReturnOK=false;
                    }
                  }
                } else {
                    ReturnOK=false;
                }
              }
            }

        }
    
        if (ReturnOK==true){
            ReadSerialData(true);

            if (LOG_Rotation < 4){
              PrintAlgoritmSteps();
              Serial.print ("OK GOOD!:"); Serial.println (input.indexOf("OK"));
            }
            
        } else {
            //GetAlgoritmSteps[1]++;

            if (LOG_Rotation < 4){
                Serial.println ("~~~~~~~~~~~~~~");
                Serial.print ("input:"); Serial.println (input);
                Serial.print ("WaintSerialResult2:"); Serial.print (WaintSerialResult2);
                Serial.print(" (");Serial.print(input.indexOf(WaintSerialResult2));Serial.println(")");                         
                Serial.print ("WaintSerialResult3:"); Serial.print (WaintSerialResult3);
                Serial.print(" (");Serial.print(input.indexOf(WaintSerialResult3));Serial.println(")");
                Serial.print ("ReturnOK:"); Serial.println (ReturnOK);
                Serial.println ("**************");
            }
        }

        return ReturnOK;
    
    }


// Проверка что делать следующим шагом конвеера
    void CheckNextStep(){
        if (ForceToNextStep==-1){
            ResetAlgoritmSteps(false); 
        } else if (ForceToNextStep==0){
            ResetAlgoritmSteps(true);
        } else {
            ResetAlgoritmSteps(true,ForceToNextStep);
        }
    }



    int GetModemStatusParser(){
        int Return=2;
        tmp_s1="";
        tmp_n1=0;
        tmp_n2=0;
        tmp_n3=0;
        
      
        tmp_s1="+CPAS:";   // AT+CPAS|+CPAS: 0|OK|
        tmp_n1=input.indexOf(tmp_s1);
        tmp_n2=tmp_s1.length();
    
        if (tmp_n1>0){
            tmp_n1+=tmp_n2;
            
            tmp_n3=input.indexOf("|", tmp_n1+1);
            if (tmp_n3>0){
                tmp_s2=input.substring(tmp_n1,tmp_n3);
                tmp_s2.trim();  
    
                
                ModemStatus=tmp_s2.toInt();
                Return=tmp_s2.toInt();
    
                //Serial.print ("ModemStatus:");Serial.print (ModemStatus); Serial.println(";");
    
                // Информация о состояние модуля
                // 0 – готов к работе
                // 2 – неизвестно
                // 3 – входящий звонок
                // 4 – голосовое соединение
    
                //lcd.setCursor(15,1);
                //lcd.print (tmp_s2);
            }
        }

        return Return;
    }


    void SettingsBeginAlgoritmSerial(){
      // Общие настройки
      SerialEvents1(true);
      input="";
      
      MaxGetAlgoritmSteps=20;             // Стандартное ограничение шагов
      TimeOutGetAlgoritm=millis()+5000;   // Стандарт Timout 5 сек
      WaintSerialResult1="OK";            // Положительный Ответ №1 по умолчанию (Обезательный)
      WaintSerialAlternativeResult1="";   // Альтернативный Ответ №1 по умолчанию (работает как ИЛИ)
      WaintSerialResult2="";              // Положительный Ответ №2 (работает как И к Ответу №1 )
      WaintSerialResult3="";              // Положительный Ответ №3 (работает как И к Ответу №1 и к Ответу №2 )
      ForceToNextStep=0;                  // Принудительно Перейти к указанному шагу      
    }






// Главный цикл/двигатель общения с модемом
    void BeginAlgoritmSerial(){

        if ((LOG_Rotation < 4) && (GetAlgoritmSteps[0]>0)){

          Serial.print ("TekSteps: ");
          Serial.print (TekSteps);
          
          Serial.print (" (");
          Serial.print (GetAlgoritmSteps[0]); 
          
          Serial.print (", ");
          Serial.print (GetAlgoritmSteps[1]); 

          Serial.print (", ");
          Serial.print (GetAlgoritmSteps[2]); 
          Serial.print ("); ");
          
          Serial.println ("");
                    
        }




        if ((ModemPowerON()==true) && (GetAlgoritmSteps[0]>0)){
        //if (TekSteps!=""){

            GetAlgoritmSteps[1]++;
            //if ((GetAlgoritmSteps[1]==0) && (EnableSerialEvents1==true)){


            // Настройки на первом шаге
            if (GetAlgoritmSteps[1]==1){
                
                // Общие настройки
                SettingsBeginAlgoritmSerial();
                
                // Проверка на зацикливание одного Шага
                if (LastSteps==TekSteps){
                    GetAlgoritmSteps[2]--;
                } else {
                    GetAlgoritmSteps[2]=MaxGetAlgoritmSteps;
                }
               
                LastSteps=TekSteps;


                // Проверка статуса модема, через каждый шаг
                if ((TekSteps!="GetModemStatus") && (GetAlgoritmSteps[3]==0)){

                    //TimerZamer=millis();
                
                    WaintSerialResult2="+CPAS:";
                    Serial1.println("AT+CPAS");    // AT+CPAS|+CPAS: 0|OK|

                    for (; ; )
                    {
                        if (GetLetsOK()==true){
                            ModemStatus=GetModemStatusParser();
                            if (ModemStatus!=0){
                                Serial.print ("ModemStatus:");Serial.print (ModemStatus); Serial.println(";");
                            }
                            break;
                        }
                        
                        if (millis() >= TimeOutGetAlgoritm) {
                            break;
                        }
                    }
                    GetAlgoritmSteps[1]=0;
                    GetAlgoritmSteps[3]=1;

                    // Входящий звонок
                    if (ModemStatus==3){
                        //Serial1.println("ATA");
                        //String strRING=AtExecute("ATA");
                        
                        Serial.println (AtExecute("ATA"));
                    }

                    //TimerZamer=millis();
                    //Serial.print ("ZAMER TIMER MODEM Status:");Serial.print (millis()-TimerZamer); Serial.println(";");

                    
                }
            }

            // Проверка на зацикливание одного Шага
            if (GetAlgoritmSteps[2]<=0){
                GetAlgoritmSteps[2]=MaxGetAlgoritmSteps;
                LastSteps="";                  
                ResetAlgoritmSteps(false); // Завершить дальнейшие такты
            }            



            if (TekSteps=="GetTemperature"){
                if (GetAlgoritmSteps[1]==1){
                    switch(GetAlgoritmSteps[0]){ // Отправляем запросы
                      case 1:  // Запрос: Уровень сигнала
                      {                 
                          WaintSerialResult2="+CSQ:";
                          Serial1.println("AT+CSQ");    // GSM_buf: AT+CSQ |+CSQ: 27,0|OK|
                          break;
                      }
                      case 2:  // Статус подключения И есть ли IP
                      {                 
                          WaintSerialResult2="+SAPBR:";
                          Serial1.println("AT+SAPBR=2,1");    // GSM_buf: AT+SAPBR=2,1|+SAPBR: 1,1,"10.217.146.5"|OK| 
                                                              // GSM_buf: AT+SAPBR=2,1|+SAPBR: 1,3,"0.0.0.0"|OK|                              
                          
                          break;
                      }
                      case 3:   // Далее несколько шагов прописываем настройки подключения  
                      {
                          Serial1.println("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"");   // GSM_buf: AT+SAPBR=3,1,"CONTYPE","GPRS"|OK|
                          break;
                      }
                      case 4:   // 
                      {                                       
                          Serial1.println("AT+SAPBR=3,1,\"APN\",\"internet.beeline.ru\"");      // GSM_buf: AT+SAPBR=3,1,"APN","internet.beeline.ru"|OK|
                          break;
                      }
                      case 5:   //           
                      {
                          Serial1.println("AT+SAPBR=3,1,\"USER\",\"beeline\"");      // GSM_buf: AT+SAPBR=3,1,"USER","beeline"|OK|
                          break;
                      }
                      case 6:   //           
                      {
                          Serial1.println("AT+SAPBR=3,1,\"PWD\",\"beeline\"");      //GSM_buf: AT+SAPBR=3,1,"PWD","beeline"|OK|
                          break;
                      }
                      case 7:   //           
                      {
                          Serial1.println("AT+SAPBR=3,1,\"PHONENUM\",\"*99#\"");      // GSM_buf: AT+SAPBR=3,1,"PHONENUM","*99#"|OK|
                          break;
                      }
                      case 8:   // Подключение  AT+SAPBR=1,1          
                      {
                          TimeOutGetAlgoritm=millis()+15000;
                          WaintSerialAlternativeResult1="ERROR";
                          Serial1.println("AT+SAPBR=1,1");      // |OK|
                          break;
                      }   
                      case 9:  // Запрос: 
                      {                 
                          WaintSerialAlternativeResult1="ERROR";
                          Serial1.println("AT+HTTPTERM");       // GSM_buf: AT+HTTPTERM|OK|
                                                                // GSM_buf: AT+HTTPTERM|ERROR|
                          break;
                      }   
                      case 10:    //  Инициализация HTTP            
                      {
                          WaintSerialAlternativeResult1="ERROR";
                          Serial1.println("AT+HTTPINIT");     // GSM_buf: AT+HTTPINIT|OK|
                                                              // GSM_buf: AT+HTTPINIT|ERROR|
                          break;
                      }
                      case 11:   //           
                      {
                          Serial1.println("AT+HTTPPARA=\"URL\",\"http://dove.omsk.otpbank.ru/files/weather.js\"");      // GSM_buf: AT+HTTPPARA="URL","http://dove.omsk.otpbank.ru/files/weather.js"|OK|
                          break;
                      }
                      case 12:   //           
                      {
                          WaintSerialResult2="+HTTPACTION:";
                          TimeOutGetAlgoritm=millis()+15000;
                          Serial1.println("AT+HTTPACTION=0");       // GSM_buf: AT+HTTPACTION=0|OK|
                                                                        // GSM_buf: |+HTTPACTION:0,200,19|
                          break;
                      }
                      case 13:   //           
                      {
                          WaintSerialResult2="+HTTPREAD:";
                          TimeOutGetAlgoritm=millis()+15000;
                          Serial1.println("AT+HTTPREAD");       // GSM_buf: AT+HTTPREAD|+HTTPREAD:19|var Therm = "25.0";|OK|
                          break;
                      }
                 
                    } // End Switch
                } else if (GetAlgoritmSteps[1]>1){ // Первый шаг пройден, переходим ко второму
                    if (GetLetsOK()==true){
                        switch(GetAlgoritmSteps[0]){ // Разбор данных, после успешного ответа
                              case 2:  // Статус подключения
                              {                 
                                  tmp_s1="+SAPBR: 1,";
                                  tmp_n1=input.indexOf(tmp_s1);
                                  tmp_n2=tmp_s1.length();
      
                                  boolean GoToConnect=true;
          
                                  if (tmp_n1>0){
                                      tmp_n1+=tmp_n2;
                                      tmp_s2=input.substring(tmp_n1,tmp_n1+1);
                                      
                                      if (LOG_Rotation < 4){
                                          PrintAlgoritmSteps();                               
                                          Serial.print ("Connection status:");Serial.println (tmp_s2);
                                      }
          
                                      /*
                                      0 – соединение устанавливается
                                      1 – соединение установлено
                                      2 – соединение закрывается
                                      3 – нет соединения
                                      */
          
                                      tmp_n2=input.indexOf("\"", tmp_n1);
                                      tmp_n3=input.indexOf("\"", tmp_n2+1);
                                      tmp_s3=input.substring(tmp_n2+1,tmp_n3);
      
                                      if (LOG_Rotation < 4){
                                          PrintAlgoritmSteps();
                                          Serial.print ("IP address:");Serial.println(tmp_s3);
                                      }
                                      
                                      if ((tmp_s3=="0.0.0.0") || (!tmp_s2=="1")){
                                          if (LOG_Rotation < 4){
                                              PrintAlgoritmSteps();
                                              Serial.println ("Bad Connection");
                                          }

                                          lcd.setCursor(0, 0);
                                          lcd.print ("no_IP  ");
                                      } else {
                                          if (LOG_Rotation < 4){
                                              PrintAlgoritmSteps();
                                              Serial.println ("GOOD Connection");
                                          }
                                          ForceToNextStep=10; // Переход к шагу 10
                                      }
                                  }

                                  break;
                              } // End Case 2 (Статус подключения)

                              case 8:  // Этап подключения
                              {  



                                  if (ReturnOKResult!="OK"){
                                      GetAlgoritmSteps[2]--;
                                      ForceToNextStep=2; // Переход к шагу 2 

                                      if (LOG_Rotation < 4){
                                          PrintAlgoritmSteps();                               
                                          Serial.print ("RECheck Connection"); Serial.println(";");
                                      }
                                  
                                  }

                                  


                                  break;
                              } // End Case 8 (Этап подключения)

                              
                              case 13:  // AT+HTTPREAD|+HTTPREAD:19|var Therm = "25.0";|OK|
                              {  
                                  tmp_s1="var Therm = \"";
                                  tmp_n1=input.indexOf(tmp_s1);
                                  tmp_n2=tmp_s1.length();
      
          
                                  if (tmp_n1>0){
                                      tmp_n1+=tmp_n2;
                                      tmp_n3=input.indexOf("\"", tmp_n1+1);
                                      if (tmp_n3>0){
                                          tmp_s2=input.substring(tmp_n1,tmp_n3);
                                          
                                          if (LOG_Rotation < 4){
                                              PrintAlgoritmSteps();                               
                                              Serial.print ("Temperature:");Serial.print (tmp_s2); Serial.println(";");
                                          }
      
                                          lcd.setCursor(0, 0);
                                          lcd.print (tmp_s2);lcd.print ("C  ");
                                      }
          
                                  }

                                  // Успешно получили температуру
                                  TimerTemperature = millis()+IntervalGetTemperature;
                                  ForceToNextStep=-1; // Завершить дальнейшие такты
                                  break;
                              } // End Case 13
                        } // End Switch
                        CheckNextStep();
                    } // End GetLetsOK
                }
            } // End GetTemperature

            if (TekSteps=="GetBalance"){
                if (GetAlgoritmSteps[1]==1){
                    switch(GetAlgoritmSteps[0]){ // Отправляем запросы
                          case 1:  // Запрос: Уровень сигнала
                          {                 
                              WaintSerialResult2="+CSQ:";
                              Serial1.println("AT+CSQ");    // GSM_buf: AT+CSQ |+CSQ: 27,0|OK|
                              break;
                          }
                          case 2:  // Запрос баланса
                          {                 
                              WaintSerialResult2="+CUSD:";
                              WaintSerialResult3="Vash balans";
                              //MaxGetAlgoritmSteps=50;
                              TimeOutGetAlgoritm=millis()+15000;
                              Serial1.println("AT+CUSD=1,\"#100#\""); // +CUSD: 0,"Vash balans -8.33 rub. Bonus: 0.00 rub. Besplatnyh sekund: 0. Dopolnitelnye balansy: #106#",64|
                              break;
                          }                          
                    } // End Switch
                } else if (GetAlgoritmSteps[1]>1){ // Первый шаг пройден, переходим ко второму
                    if (GetLetsOK()==true){
                        switch(GetAlgoritmSteps[0]){ // Разбор данных, после успешного ответа
                              case 2:  // Статус подключения
                              { 
                                  tmp_s1="Vash balans ";
                                  tmp_n1=input.indexOf(tmp_s1);
                                  tmp_n2=tmp_s1.length();
          
                                  if (tmp_n1>0){
                                      tmp_n1+=tmp_n2;
                                      tmp_n3=input.indexOf("rub.", tmp_n1+1);
                                      if (tmp_n3>0){
                                          tmp_s2=input.substring(tmp_n1,tmp_n3);
                                          tmp_s2.trim();
                                          tmp_n1=tmp_s2.toInt(); 
                                          
                                          tmp_s2=tmp_n1;
                                          tmp_n2=tmp_s2.length();
                                          Serial.print ("Balanc:");Serial.print (tmp_n1); Serial.println(";");
          
                                          lcd.setCursor(9, 0);lcd.print("       ");
                                          lcd.setCursor(13-tmp_n2, 0);
                                          
                                          lcd.print (tmp_n1);lcd.print("rub");
                                      } else {
                                        Serial.print ("FAIL Balanc2: ");Serial.print (input); Serial.println(";");
                                      }
                                      
            
                                  } else {
                                    Serial.print ("FAIL Balanc2: ");Serial.print (input); Serial.println(";");
                                  }
      
                                  TimerBalance = millis()+IntervalGetBalance;
                                  ForceToNextStep=-1; // Завершить дальнейшие такты
                                  
                                  break;
                              } // End Case 2
                        } // End Switch
                        CheckNextStep();
                    } // End GetLetsOK
                }                        
            } // End GetBalance

            if (TekSteps=="GetModemStatus"){
                if (GetAlgoritmSteps[1]==1){
                    switch(GetAlgoritmSteps[0]){ // Отправляем запросы
                          case 1:  // Запрос Статуса модема
                          {                 
                              WaintSerialResult2="+CPAS:";
                              Serial1.println("AT+CPAS");    // AT+CPAS|+CPAS: 0|OK|
                              break;
                          }                          
                    } // End Switch
  
                } else if (GetAlgoritmSteps[1]>1){ // Первый шаг пройден, переходим ко второму
                    if (GetLetsOK()==true){

                        switch(GetAlgoritmSteps[0]){ // Разбор данных, после успешного ответа
                            case 1:  // Статус модема
                            { 
                                ModemStatus=GetModemStatusParser();

                                if (ModemStatus!=0){
                                    Serial.print ("ModemStatus:");Serial.print (ModemStatus); Serial.println(";");
                                }

                                // Входящий звонок
                                if (ModemStatus==3){
                                    //Serial1.println("ATA");
                                    Serial.println (AtExecute("ATA"));
                                  
                                }                                

                                TimerModemStatus = millis()+IntervalGetModemStatus;
                                ForceToNextStep=-1; // Завершить дальнейшие такты
                                
                                break;
                            } // End Case 1
                      } // End Switch
            
            
            
                        CheckNextStep();
                    } // End GetLetsOK
                }                  
            } // End GetModemStatus

            if (TekSteps=="DEMO"){
                if (GetAlgoritmSteps[1]==1){
                    switch(GetAlgoritmSteps[0]){ // Отправляем запросы
                          case 1:  // Запрос
                          {                 

                              break;
                          }                          
                    } // End Switch
  
                } else if (GetAlgoritmSteps[1]>1){ // Первый шаг пройден, переходим ко второму
                    if (GetLetsOK()==true){
                        switch(GetAlgoritmSteps[0]){ // Разбор данных, после успешного ответа
                            case 1:  //  
                            { 
   
                                break;
                            } // End Case 1
                      } // End Switch
            
            
            
                        CheckNextStep();
                    } // End GetLetsOK
                }                  
            } // End DEMO

            if (TekSteps=="GetDateTime"){
                if (GetAlgoritmSteps[1]==1){
                    switch(GetAlgoritmSteps[0]){ // Отправляем запросы
                          case 1:  // Запрос Даты и Время
                          {                 
                              WaintSerialResult2="+CCLK:";
                              Serial1.println("AT+CCLK?");    // AT+CCLK?||||+CCLK: "17/06/22,20:38:51+06"||||OK||
                              break;
                          }                          
                    } // End Switch
                } else if (GetAlgoritmSteps[1]>1){ // Первый шаг пройден, переходим ко второму
                    if (GetLetsOK()==true){

                        switch(GetAlgoritmSteps[0]){ // Разбор данных, после успешного ответа
                              case 1:  // Дата и Время
                              { 
                                  tmp_s1="+CCLK: \"";   // AT+CCLK?||||+CCLK: "17/06/22,20:38:51+06"||||OK||
                                  tmp_n1=input.indexOf(tmp_s1);
                                  tmp_n2=tmp_s1.length();
          
                                  if (tmp_n1>0){
                                      tmp_n1+=tmp_n2;
                                      tmp_n1+=3;
                                      tmp_n3=input.indexOf("\"", tmp_n1+1);
                                      if (tmp_n3>0){
                                          tmp_s2=input.substring(tmp_n1,tmp_n3-3);
                                          tmp_s2.trim();  
        
                                          DateTime=tmp_s2;
        
                                          Serial.print ("DateTime:");Serial.print (DateTime); Serial.println(";");
          
                                          lcd.setCursor(0,1);
                                          lcd.print (tmp_s2);
                                      }
            
                                  }                        
        
        
                                  TimerDateTime = millis()+IntervalGetDateTime;
                                  ForceToNextStep=-1; // Завершить дальнейшие такты
                                  
                                  break;
                              } // End Case 1
                        } // End Switch

                        CheckNextStep();
                    } // End GetLetsOK
                }  
                
            } // End GetDateTime - Запрос   




            if (GetAlgoritmSteps[1]>1){
                // Если долго засиделист в цикле (по таймауту), идем на выход
                if (millis() >= TimeOutGetAlgoritm) {
                    PrintAlgoritmSteps();
                    Serial.println("FAIL!!! TimeOut");
                    ResetAlgoritmSteps(false);
                }   // End - Выход из зацикливания               
            }



        
        } // End - Если выполняется хотябы какой то алгоритм
    } // End - BeginAlgoritmSerial















    void GetTemperature(){
        if ((ModemStatus==0) && ((TekSteps=="GetTemperature") || (TekSteps==""))){
        //if (((TekSteps=="GetTemperature") || (TekSteps==""))){
            if (millis() >= TimerTemperature) {
                TekSteps="GetTemperature";
                TimerTemperature = millis()+IntervalGetTemperature;
                
                GetAlgoritmSteps[0]=1; // Этап конвеера. Череда комманд модему.
                GetAlgoritmSteps[1]=0; // Колличество тактов в пределах одного этапа, сбрасывается в 0 при каждом таймауте
            }
        }
        BeginAlgoritmSerial();
    }

    void GetBalance(){
        if ((ModemStatus==0) && ((TekSteps=="GetBalance") || (TekSteps==""))){
        //if (((TekSteps=="GetBalance") || (TekSteps==""))){
            if (millis() >= TimerBalance) {
                TekSteps="GetBalance";
                TimerBalance = millis()+IntervalGetBalance;

                GetAlgoritmSteps[0]=1; // Этапы
                GetAlgoritmSteps[1]=0; // Колличество попыток получить ответ, в пределах одного таймаута сбрасывается в 0 при каждом таймауте
            }
        }
        BeginAlgoritmSerial();
    }

    void GetDateTime(){
        if (ModemPowerON()==true){
            if ((TekSteps=="GetDateTime") || (TekSteps=="")){
                if (millis() >= TimerDateTime) {
                    TekSteps="GetDateTime";
                    TimerDateTime = millis()+IntervalGetDateTime;
    
                    GetAlgoritmSteps[0]=1; // Этапы
                    GetAlgoritmSteps[1]=0; // Колличество попыток получить ответ, в пределах одного таймаута сбрасывается в 0 при каждом таймауте
                }
            }
        }
        BeginAlgoritmSerial();
    }

    void GetModemStatus(){
        if (ModemPowerON()==true){
            if ((TekSteps=="GetModemStatus") || (TekSteps=="")){
                if (millis() >= TimerModemStatus) {
                    TekSteps="GetModemStatus";
    
                    // Информация о состояние модуля
                    // 0 – готов к работе
                    // 2 – неизвестно
                    // 3 – входящий звонок
                    // 4 – голосовое соединение
                    // 5 - Выключен
                    
                    ModemStatus = 2;
                    
                    TimerModemStatus = millis()+IntervalGetModemStatus;
    
                    GetAlgoritmSteps[0]=1; // Этапы. Запустит конвеер с самого начала.
                    GetAlgoritmSteps[1]=0; // Колличество попыток получить ответ, в пределах одного таймаута сбрасывается в 0 при каждом таймауте
                }
            }
        }
        
        BeginAlgoritmSerial();
    }


    void EverySecond(){
        if (millis() >= TimerEverySecond) {
            TimerEverySecond = millis()+1000; 

// Выполняется каждую секунду
    
            
            /*
            long rrr=(TimerModemStatus-millis())/1000;
              
            lcd.setCursor(14, 1);
            lcd.print ("  ");
            lcd.setCursor(14, 1);
            lcd.print (rrr);
                   



            Serial.print ("TimerModemStatus: ");
            Serial.println (rrr); 

            if (LOG_Rotation < 4){
              Serial.print ("GetAlgoritmSteps: ");
              Serial.println (GetAlgoritmSteps[0]); 
            }
*/

            //Serial.print ("GetAlgoritmSteps[0]: ");
            //Serial.println (GetAlgoritmSteps[0]); 

                
            
        }
      
    }




    
    
    


    void LifeUpTime(){
      if (millis() >= TimerLifeUpTime) {
        D13_Inv;
        TimerLifeUpTime = millis()+IntervalLifeUpTime;
        //TimerLifeUpTime = millis()+500;
      
   
  
            if (ModemPowerON()==true){
                lcd.setCursor(0, 1);
    
                if (TekSteps==""){
                    lcd.print (DateTime);
                    lcd.print ("                ");
                } else {
                    lcd.print ("                ");
                    lcd.setCursor(0, 1);
                    
                    if (D13_Read==HIGH){
                      lcd.print (TekSteps);
                      lcd.print ("                ");
                    }
                }
            }

            

            if (TekSteps=="timeout"){
                TekSteps="";
                Serial.println ("TimeOut!!!");
                Serial.println ("###############################");                 
            }
      }


      if (millis() >= TimerModemPower) {
        TimerModemPower = millis()+IntervalModemPower;

        

        if (ModemPowerON()==true){
          ModemPowerStatus=1;
        } else {
          ModemStatus = 5;
          TekSteps="";
          //ResetAlgoritmSteps(false); // Завершить дальнейшие такты
          GetAlgoritmSteps[0]=0; // Этапы. Выключить конвеер.
          GetAlgoritmSteps[1]=0; // Колличество попыток получить ответ, в пределах одного таймаута сбрасывается в 0 при каждом таймауте
          
          if (ModemPowerStatus!=-1){
              lcd.setCursor(0, 0);
              lcd.print ("                ");
              
              lcd.setCursor(0, 1);
              lcd.print ("Modem PowerOff  ");

              

              Serial.println("Modem PowerOff;");
          }

          ModemPowerStatus=-1;

          
        }
  
        lcd.setCursor(7, 0);

        if (ModemPowerON()==true)
        {
            lcd.print ("1 ");
        } else {
            lcd.print ("0 ");
        }
      }
      
    }



    void BeginDHT22() {

      if (millis() >= TimerDHT22) {
          TimerDHT22 = millis()+IntervalDHT22;



           // считывание температуры или влажности занимает примерно 250 мс!
            // считанные показания могут отличаться от актуальных примерно на 2 секунды (это очень медленный датчик)
            float h = dht.readHumidity();
            // Считывание температуры в цельсиях
            float t = dht.readTemperature();
          
            // проверяем, были ли ошибки при считывании и, если были, начинаем заново
            if (isnan(h) || isnan(t)) {
              Serial.println("Failed to read from DHT sensor!");
              return;
            }
          
            //выводим информацию в Монитор последовательного порта
            Serial.print("Humidity: ");
            Serial.print(h);
            Serial.print(" %\t");
            Serial.print("Temperature: ");
            Serial.print(t);
            Serial.println(" *C ");




          
      }
  
    }



 
    void loop() {         
        LifeUpTime();
        EverySecond();
        
        GetModemStatus();

        if (StopAll==false){

            GetDateTime();
            GetBalance();
            GetTemperature();
        }
        
        SerialEvents0();
        SerialEvents1();
        
        //BeginDHT22();
    }
