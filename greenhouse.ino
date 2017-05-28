//---------------------------------------------------
// 1. Librerias
//---------------------------------------------------
#include <Wire.h>
#include <LiquidCrystal_I2C.h> 
#include <Time.h>
#include <DS1307RTC.h>
#include <ds3231.h>
#include <DHT.h>
#include <OneWire.h>

//----------------------------------------------------
// 2. Pines
//----------------------------------------------------
#define xPin     A1   
#define yPin     A0   
#define kPin      7   
#define Trig      8
#define Echo      9
#define DHTPIN    4     
#define DHTTYPE DHT11 
DHT dht(DHTPIN, DHTTYPE);
OneWire ds(10);
//SDA            A4
//SCL            A5

//----------------------------------------------------
// 3. Variables y Comandos
//----------------------------------------------------
int tCount1;
bool refresh;//lcd clear On/Off
//leerJoystick
int joyRead;
int joyPos; 
int lastJoyPos;
long lastDebounceTime = 0; 
long debounceDelay = 70;                 //user define
//Control Joystick
bool PQCP;
bool editMode;
//sistema de menus
int menuLevel1;  
int menuLevel2;  
//editmode
byte n[19];
int lastN;
int lcdX;
//int lcdY;
bool exiT;

//common prevMillis
unsigned long previousMillisInMenuWaterLevel = 0; 
unsigned long previousMillisInMenuTime = 0;
unsigned long previousMillisInMenuOutsideData = 0;
unsigned long previousMillisInMenuInsideDataTemperature = 0;
unsigned long previousMillisEventListener = 0; 
unsigned long previousMillisIsWatering = 0; 
unsigned long previousMillisFunctionWatering = 0; 
unsigned long previousMillisIsPumping = 0; 
unsigned long previousMillisFunctionPumping = 0; 
unsigned long previousMillisUpdateVars = 0; 

long intervalInMenu = 1000; // общий интервал 
long intervalEventListener = 2000; 
long intervalIsWatering = 2000; 
long intervalIsPumping = 2000; 
long intervalFunctionWatering = 4000; 
long intervalFunctionPumping = 4000;
long intervalUpdateVars = 5000; 

unsigned long previousMillis = 0; 
//long intervalUpdateVars = 2000; // половина периода мигания (в миллисекундах)



// water level
int duration, distance;  

//DS3231
const char *monthName[12] = {
  "Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};
tmElements_t tm;
bool parse=false;
bool config=false;
int zxc = 0;
int seconds; 

//DHT
float outsideT;
float outsideH;

//ds18b20
int insideT;

//joystick
int btn = digitalRead(kPin);

//state
bool isWatering = false;
bool isPumping = false;
bool isPress = false;
bool buttonWasUp = true;  // была ли кнопка отпущена?

//----------------------------------------------------
// 4. Objetos
//----------------------------------------------------
LiquidCrystal_I2C lcd(0x3f,16,2); 

//====================================================
// SETUP
//====================================================
void setup() {
//----------------------------------------------------
// S1. Pines
//----------------------------------------------------
  pinMode(xPin, INPUT);
  pinMode(yPin, INPUT);
  pinMode(kPin, INPUT_PULLUP);
  pinMode(Trig, OUTPUT); 
  pinMode(Echo, INPUT); 
//----------------------------------------------------
// S2. Objetos
//----------------------------------------------------
  // get the date and time the compiler was run
  if (getDate(__DATE__) && getTime(__TIME__)) {
    parse = true;
    // and configure the RTC with this info
    if (RTC.write(tm)) {
      config = true;
    }
  }
  Serial.begin(9600);
  lcd.init();
  lcd.backlight(); 
  dht.begin();
}

//====================================================
// LOOP
//====================================================
void loop() {
  unsigned long currentMillis = millis(); // текущее время в миллисекундах
  controlJoystick();
  menu(currentMillis);
  eventListener(currentMillis);
   if(currentMillis - previousMillisFunctionWatering >= intervalFunctionWatering) {
    previousMillisFunctionWatering = currentMillis;
      Watering(currentMillis);
   }

  Pumping(currentMillis);
}

//====================================================
// Menu
//====================================================
void menu(unsigned long &currentMillis){
  switch (menuLevel1){
    case 0:
      if(menuLevel2==1){
          menu11(currentMillis);
        } else if (menuLevel2 == -1){
          menu12(currentMillis);
        } else {
          menu0();
        }
      break;
    case -1:
      if(menuLevel2 == 1){
        menu21(currentMillis);
      } else if (menuLevel2 == -1){
        menu22(currentMillis);
      } else {
        menu2();
      }
     break;
    case 1:
      if(menuLevel2 == 1){
        menu31();
      } else if (menuLevel2 == -1){
        menu32();
      } else {
        menu3();
      }
     break;
    case 2:
      menu4(); 
     break;
  }//switch
}
//----------------------------------------------------
// Smart Greenhouse
//----------------------------------------------------
void menu0(){
  if (refresh){lcd.clear();refresh=0;}
  lcd.setCursor(3,0);
  lcd.print("Welcome to");
  lcd.setCursor(0,1);
  lcd.print("Smart Greenhouse");
}
//----------------------------------------------------
// Menu 1
//----------------------------------------------------
void menu1(){
  if (refresh){lcd.clear();refresh=0;}
  lcd.setCursor(0,0);
  lcd.print("Menu 1 TH");
  lcd.setCursor(0,1);
//++++++++++++++++++++
  while(editMode){
    controlJoystick();
    lcd.setCursor(lcdX,1);
    if(n[lcdX]!=lastN){
      lcd.print(n[lcdX]);
      lastN=n[lcdX];
    }
  }
}
//-------------------------------------------------1.1 TIME
void menu11(unsigned long &currentMillis){
  if (refresh){lcd.clear();refresh=0;}
  lcd.setCursor(0,0);

  if(currentMillis - previousMillisInMenuTime >= intervalInMenu) {
   previousMillisInMenuTime = currentMillis;
   showTime();
  }
}
//-------------------------------------------------1.2 WATER LEVEL
void menu12(unsigned long &currentMillis){
  if (refresh){lcd.clear();refresh=0;}
  lcd.setCursor(3,0);
  lcd.print("WATER LEVEL ");
  lcd.setCursor(0,1);
  lcd.print("D: ");
  if(currentMillis - previousMillisFunctionWatering >= intervalInMenu) {
   previousMillisFunctionWatering = currentMillis;
   waterLevel();
   lcd.print(distance); 
   lcd.print(" cm"); 
  } 
}

//----------------------------------------------------
// Menu 2
//----------------------------------------------------
void menu2(){
  if (refresh){lcd.clear();refresh=0;}
  lcd.setCursor(0,0);
  lcd.print("<< Outside");
  lcd.setCursor(8,1);
  lcd.print("Inside>>");
}
//-------------------------------------------------2.1
void menu21(unsigned long &currentMillis){
  if (refresh){lcd.clear();refresh=0;}
  lcd.setCursor(5,0);
  lcd.print("OUTSIDE");
  lcd.setCursor(0,1);
  if(currentMillis - previousMillisInMenuOutsideData >= intervalInMenu) {
   previousMillisInMenuOutsideData = currentMillis;
   outsideData();
   lcd.print("T:");
   lcd.print(outsideT);
   lcd.print("C;");
   lcd.print("H:");
   lcd.print(outsideH);
   lcd.print("%;");
  } 
  Serial.print(outsideT);
  Serial.print(outsideH); 
}
//-------------------------------------------------2.2
void menu22(unsigned long &currentMillis){
  if (refresh){lcd.clear();refresh=0;}
  lcd.setCursor(5,0);
  lcd.print("INSIDE");
  lcd.setCursor(0,1);
  if(currentMillis - previousMillisInMenuInsideDataTemperature >= intervalInMenu) {
   previousMillisInMenuInsideDataTemperature = currentMillis;
   insideDataTemperature();
   lcd.print("T:");
   lcd.print(insideT);
   lcd.print(".00");
   lcd.print("C;");
  } 
}

//----------------------------------------------------
// Menu 3
//----------------------------------------------------
void menu3(){
  if (refresh){lcd.clear();refresh=0;}
  lcd.setCursor(0,0);
  lcd.print("Menu 3 SHOW EXTRA");
}
//-------------------------------------------------2.1
void menu31(){
  if (refresh){lcd.clear();refresh=0;}
  lcd.setCursor(0,0);
  lcd.print("Menu 3.1 extra1");
}
//-------------------------------------------------2.2
void menu32(){
  if (refresh){lcd.clear();refresh=0;}
  lcd.setCursor(0,0);
  lcd.print("Menu 3.2 extra2");
}

//----------------------------------------------------
// Menu 4
//----------------------------------------------------
void menu4(){
  if (refresh){lcd.clear();refresh=0;}
  lcd.setCursor(0,0);
  lcd.print("Window:");
  bool buttonIsUp = digitalRead(kPin);
    if (buttonWasUp && !buttonIsUp) {
    // ...может это «клик», а может и ложный сигнал (дребезг),
    // возникающий в момент замыкания/размыкания пластин кнопки,
    // поэтому даём кнопке полностью «успокоиться»...
    delay(100);
    // ...и считываем сигнал снова
    buttonIsUp = digitalRead(kPin);
    if (!buttonIsUp) {  // если она всё ещё нажата...
      // ...это клик! 
      isPress = !isPress;
      Serial.println("Press Button");
    }
  }
  lcd.setCursor(0,1);
  if(!isPress) {
    lcd.print("Close");
  } else {
    lcd.print("Open ");  
  }
}

//====================================================
// Control Joystic
//====================================================
void controlJoystick(){
  leeJoystick();
  if(PQCP) {
    PQCP=0;
    if (joyPos==5){editMode=!editMode;}
    switch (editMode){
//       case 1: 
//          lcd.blink(); //мигание курсора, для меню 1 
//          if (joyPos==4&&n[lcdX]<9){n[lcdX]++;   //arriba - вверх
//              refresh=0;}
//          if (joyPos==3&&n[lcdX]>0){n[lcdX]--;   //abajo - вниз 
//              refresh=0;} 
//          if (joyPos==1&&lcdX<19){lcdX++;        //derecha - вправо
//            refresh=0;}
//          if (joyPos==2&&lcdX>0){lcdX--;         //izq - влево
//           refresh=0;}
//        break;
        case 0:
          lcd.noBlink();
          if (menuLevel1<2&&joyPos==3){menuLevel1++;    //abajo
            refresh=1;
            menuLevel2=0;}
          if (menuLevel1>-1&&joyPos==4){menuLevel1--;    //arriba
            menuLevel2=0;
            refresh=1;}
          if (menuLevel2<1&&joyPos==1){menuLevel2++;   //derecha
            refresh=1;}
          if (menuLevel2>-1&&joyPos==2){menuLevel2--;    //izq
           refresh=1;}
        }//!edit
  }//PQCP
}
int leeJoystick(){ //считывание 
  int x = analogRead(xPin);
  int y = analogRead(yPin);
  int k = digitalRead(kPin);
    if(x>900){joyRead=1;        //x+
    }else if(x<100){joyRead=2;  //x-
    }else if(y>900){joyRead=3;  //y+
    }else if(y<100){joyRead=4;  //y-
    }else if(!k){joyRead=5;
    }else{joyRead=0;}

  if (joyRead != lastJoyPos){lastDebounceTime = millis();}     // отчет с последнего нажатия
  if(((millis() - lastDebounceTime) >= debounceDelay)&&(joyRead!=joyPos)){
    joyPos=joyRead;
    if(!PQCP){PQCP=1;}
    }
//  if(((millis() - lastDebounceTime) > (5*debounceDelay))&&(joyPos==3||joyPos==4)){
//    joyPos=joyRead;                     //repeat time only for UP/DOWN
//    if(!PQCP){PQCP=1;}
//    }
  lastJoyPos=joyRead;
}

//====================================================
// Get time
//====================================================
bool getTime(const char *str)
{
  int Hour, Min, Sec;

  if (sscanf(str, "%d:%d:%d", &Hour, &Min, &Sec) != 3) return false;
  tm.Hour = Hour;
  tm.Minute = Min;
  tm.Second = Sec;
  return true;
}

bool getDate(const char *str)
{
  char Month[12];
  int Day, Year;
  uint8_t monthIndex;

  if (sscanf(str, "%s %d %d", Month, &Day, &Year) != 3) return false;
  for (monthIndex = 0; monthIndex < 12; monthIndex++) {
    if (strcmp(Month, monthName[monthIndex]) == 0) break;
  }
  if (monthIndex >= 12) return false;
  tm.Day = Day;
  tm.Month = monthIndex + 1;
  tm.Year = CalendarYrToTm(Year);
  return true;
}

void showTime() {
  if (RTC.read(tm)) {
    lcd.print("Time: ");
    print2digits(tm.Hour);
    lcd.write(':');
    print2digits(tm.Minute);
    lcd.write(':');
    print2digits(tm.Second);
    lcd.setCursor(0,1);
    lcd.print("Date: ");
    lcd.print(tm.Day);
    lcd.write('/');
    lcd.print(tm.Month); 
    lcd.write('/');
    lcd.print(tmYearToCalendar(tm.Year));
    seconds = tm.Second;
  }     
    Serial.println("hi");
}

void print2digits(int number) {
  if (number >= 0 && number < 10) {
    lcd.write('0');
  }
  lcd.print(number);
}

void eventListener(unsigned long &currentMillis) {
  if(currentMillis - previousMillisEventListener >= intervalEventListener) {
    previousMillisEventListener = currentMillis;
//     if (RTC.read(tm)) {
//      seconds = tm.Second;
////      Serial.println(seconds);   
//     }
     showVariables();
     
      if(currentMillis - previousMillisUpdateVars >= intervalUpdateVars) {
       previousMillisUpdateVars = currentMillis;
       waterLevel();
       outsideData();
       insideDataTemperature();
       Serial.println("Action");
      }    
  }
}

void waterLevel() {
  digitalWrite(Trig, LOW); 
  delayMicroseconds(2); 
  digitalWrite(Trig, HIGH); 
  delayMicroseconds(10); 
  digitalWrite(Trig, LOW); 
  duration = pulseIn(Echo, HIGH); 
  distance = duration / 58;

}

void outsideData() {
  outsideT = dht.readTemperature();
  outsideH = dht.readHumidity();
}

void insideDataTemperature() {
  byte data[2];
  ds.reset(); 
  ds.write(0xCC);
  ds.write(0x44);
  //delay(750);
  ds.reset();
  ds.write(0xCC);
  ds.write(0xBE);
  data[0] = ds.read(); 
  data[1] = ds.read();
  insideT = (data[1]<< 8)+data[0];
  insideT = insideT>>4;
}

void showVariables() {
  Serial.print("Variables: ");
  Serial.print("Water level:");
  Serial.print(distance);
  Serial.print(";  Out_T:");
  Serial.print(outsideT);
  Serial.print(";  Out_H:");
  Serial.print(outsideH);
  Serial.print(";  In_T:");
  Serial.print(insideT);
  Serial.println();
  Serial.print("State: ");
  Serial.print("isWatering: ");
  Serial.print(isWatering);
  Serial.print(";  isPumping: ");
  Serial.print(isPumping);
  Serial.print(";  isPress: ");
  Serial.print(isPress);
  Serial.println();
}

void Watering(unsigned long &currentMillis) {
 if(currentMillis - previousMillisIsWatering >= intervalIsWatering) {
    previousMillisIsWatering = currentMillis;
    if(outsideH) {
     if(outsideH < 40) {
      Serial.println("WATERING!");
      isWatering = true;
     } else if (outsideH > 80) {
      Serial.println("STOP WATERING!");
      isWatering = false;
     }
    }
  }
}

void Pumping(unsigned long &currentMillis) {
   if(currentMillis - previousMillisIsPumping >= intervalIsPumping) {
    previousMillisIsPumping = currentMillis;
    if(distance) {
      if(distance < 40) {
        Serial.println("PUMPING!");
        isPumping = true;
      } else if (distance > 80) {
        Serial.println("STOP PUMPING!");
        isPumping = false;
       }
    }
  }
}
