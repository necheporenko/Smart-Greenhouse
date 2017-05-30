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
#define HUMIDITY A2
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

//common prevMillis
unsigned long previousMillis = 0; 
unsigned long previousMillisInMenuWaterLevel = 0; 
unsigned long previousMillisInMenuTime = 0;
unsigned long previousMillisInMenuOutsideData = 0;
unsigned long previousMillisInMenuInsideData = 0;
unsigned long previousMillisFunctionWatering = 0;
unsigned long previousMillisEventListener = 0; 
unsigned long previousMillisIsWatering = 0;
unsigned long previousMillisIsPumping = 0; 
unsigned long previousMillisIsAeration = 0;
unsigned long previousMillisUpdateVars = 0; 

int intervalInMenu = 1000; // общий интервал 
int intervalEventListener = 2000; 
int intervalIsWatering = 4000; 
int intervalIsPumping = 4000; 
int intervalIsAeration = 4000;
int intervalFunctionWatering = 2000; 
int intervalFunctionPumping = 2000;
int intervalFunctionsAeration = 2000;
int intervalUpdateVars = 5000; 

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
int seconds, minutes, hours; 

//DHT
int outsideT;
int outsideH;

//ds18b20
int insideT;

//joystick
int btn = digitalRead(kPin);

//fc-28
int insideH;
unsigned int humidity = 0;
//state
bool isWatering = false;
bool isPumping = false;
bool isOpen = false;
bool buttonWasUp = true;  // была ли кнопка отпущена?

bool refresh;//lcd clear On/Off
//leerJoystick
int joyRead;
int joyPos; 
int lastJoyPos;
long lastDebounceTime = 0; 
long debounceDelay = 70;                 
//Control Joystick
bool PQCP;
// Рівні меню
int menuLevel1;  
int menuLevel2;  
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

  Watering(currentMillis);
  Pumping(currentMillis);
  Aeration(currentMillis);
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
//-------------------------------------------------1.1 TIME
void menu11(unsigned long &currentMillis){
  if (refresh){lcd.clear();refresh=0;}
  lcd.setCursor(0,0);

  if(currentMillis - previousMillisInMenuTime >= intervalInMenu) {
   previousMillisInMenuTime = currentMillis;
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
    }
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
   lcd.setCursor(10,1);
   lcd.print("H:");
   lcd.print(outsideH);
   lcd.print("%;");
  } 
}
//-------------------------------------------------2.2
void menu22(unsigned long &currentMillis){
  if (refresh){lcd.clear();refresh=0;}
  lcd.setCursor(5,0);
  lcd.print("INSIDE");
  lcd.setCursor(0,1);
  if(currentMillis - previousMillisInMenuInsideData >= intervalInMenu) {
   previousMillisInMenuInsideData = currentMillis;
   insideDataTemperature();
   insideDataHumidity();
   lcd.print("T:");
   lcd.print(insideT);
   lcd.print("C;");
   lcd.setCursor(10,1);
   if (insideH==200) {
    lcd.print("H:Not;");
   } else {
    lcd.print("H:");
    lcd.print(insideH);
    lcd.print("%;");
   }
  }
 } 
//----------------------------------------------------
// Menu 3
//----------------------------------------------------
void menu3(){
  if (refresh){lcd.clear();refresh=0;}
  lcd.setCursor(0,0);
  lcd.print("MANUAL SELECTION");
}
//-------------------------------------------------
void menu31(){
  if (refresh){lcd.clear();refresh=0;}
  lcd.setCursor(0,0);
  lcd.print("Watering:");
  bool buttonIsUp = digitalRead(kPin);
    if (buttonWasUp && !buttonIsUp) {
    delay(100);
    buttonIsUp = digitalRead(kPin);
    if (!buttonIsUp) {  
      isWatering = !isWatering;
      Serial.println("Press Button");
    }
  }
  lcd.setCursor(0,1);
  if(!isWatering) {
    lcd.print("OFF");
  } else {
    lcd.print("ON ");  
  }
}
//-------------------------------------------------
void menu32(){
  if (refresh){lcd.clear();refresh=0;}
  lcd.setCursor(0,0);
  lcd.print("Pumping:");
  bool buttonIsUp = digitalRead(kPin);
    if (buttonWasUp && !buttonIsUp) {
    delay(100);
    buttonIsUp = digitalRead(kPin);
    if (!buttonIsUp) {  
      isPumping = !isPumping;
      Serial.println("Press Button");
    }
  }
  lcd.setCursor(0,1);
  if(!isPumping) {
    lcd.print("OFF");
  } else {
    lcd.print("ON ");  
  }
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
    delay(100);
    buttonIsUp = digitalRead(kPin);
    if (!buttonIsUp) {  
      isOpen = !isOpen;
      Serial.println("Press Button");
    }
  }
  lcd.setCursor(0,1);
  if(!isOpen) {
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
  }
}
int leeJoystick(){                //считывание 
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
    seconds = tm.Second;
    minutes = tm.Minute;
    hours = tm.Hour; 
  }     
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
    showVariables();   
    if(currentMillis - previousMillisUpdateVars >= intervalUpdateVars) {
     previousMillisUpdateVars = currentMillis;
     waterLevel();
     outsideData();
     insideDataTemperature();
     insideDataHumidity();
     showTime();
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
  ds.reset();
  ds.write(0xCC);
  ds.write(0xBE);
  data[0] = ds.read(); 
  data[1] = ds.read();
  insideT = (data[1]<< 8)+data[0];
  insideT = insideT>>4;
}

void insideDataHumidity() {
humidity = analogRead(HUMIDITY);
  if (humidity > 950) {
    insideH = 200;
  } else if (100<humidity<950) {
    insideH = 100-(humidity-100)/8.5;
  }
}

void showVariables() {
  if (RTC.read(tm)) {
    Serial.print(hours);  
    Serial.print(":");
    Serial.print(minutes);
    Serial.print(":");
    Serial.print(seconds);
  }
  Serial.println();
  Serial.print("Variables: ");
  Serial.print("Water level:");
  Serial.print(distance);
  Serial.print(";  Out_T:");
  Serial.print(outsideT);
  Serial.print(";  Out_H:");
  Serial.print(outsideH);
  Serial.print(";  In_T:");
  Serial.print(insideT);
  Serial.print(";  In_H:");
  Serial.print(insideH);
  Serial.println();
  Serial.print("State: ");
  Serial.print("isWatering: ");
  Serial.print(isWatering);
  Serial.print(";  isPumping: ");
  Serial.print(isPumping);
  Serial.print(";  isOpen: ");
  Serial.print(isOpen);
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

void Aeration(unsigned long &currentMillis) {
  if(currentMillis - previousMillisIsAeration >= intervalIsAeration) {
    previousMillisIsAeration = currentMillis;
    if(insideT - outsideT > 20) {
      Serial.println("AERATION!");
      isOpen = true;
    } else if (insideT < 25) {
      Serial.println("STOP AERATION!");
      isOpen = false;
     }
  }
}
