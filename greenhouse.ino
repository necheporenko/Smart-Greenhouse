//---------------------------------------------------
// 1. Підключені бібліотеки
//---------------------------------------------------
#include <Wire.h>
#include <LiquidCrystal_I2C.h> 
#include <Time.h>
#include <DS1307RTC.h>
#include <ds3231.h>
#include <DHT.h>
#include <OneWire.h>
//----------------------------------------------------
// 2. Піни
//----------------------------------------------------
#define HUMIDITY A2         //внутр. датчик вологості
#define xPin     A1         //джойстик
#define yPin     A0   
#define kPin      7   
#define Trig      8         //датчик виміру відстані
#define Echo      9
#define DHTPIN    4         //зовн. датчик температури та вологості   
#define DHTTYPE DHT11 
DHT dht(DHTPIN, DHTTYPE);
OneWire ds(10);             //внутр. датчик температури
//SDA            A4
//SCL            A5
//----------------------------------------------------
// 3. Змінні
//----------------------------------------------------
unsigned long previousMillis = 0;      //поперердній час спрацювання
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

int intervalInMenu = 1000;          // інтервали спрацювання функуцій
int intervalEventListener = 2000; 
int intervalIsWatering = 4000; 
int intervalIsPumping = 4000; 
int intervalIsAeration = 4000;
int intervalUpdateVars = 5000; 

// HC-SR04
int duration, distance;  

//DS3231
const char *monthName[12] = {
  "Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};
tmElements_t tm;
bool parse=false;
bool config=false;
int seconds, minutes, hours; 

//DHT
int outsideT;
int outsideH;

//ds18b20
int insideT;

//fc-28
int insideH;
unsigned int humidity = 0;

//Стан програми
bool isWatering = false; //полив
bool isPumping = false;  //закачування води 
bool isOpen = false;     //провітрювання
bool buttonWasUp = true; // чи була ли кнопка відпущена?
bool refresh;            // очистка lcd On/Off

//джойстик
int btn = digitalRead(kPin);
int joyRead;
int joyPos; 
int lastJoyPos;
long lastDebounceTime = 0; 
long debounceDelay = 70;                 
bool PQCP;

// Меню
int menuLevel1;  
int menuLevel2;  
//----------------------------------------------------
// 4. Об'єкти
//----------------------------------------------------
LiquidCrystal_I2C lcd(0x3f,16,2);  //встановлюємо адресу LCD в 0x3f для дисплею з 16 символами в 2 рядках
//====================================================
// SETUP
//====================================================
void setup() {
//----------------------------------------------------
// S1. Піни
//----------------------------------------------------
  pinMode(xPin, INPUT);
  pinMode(yPin, INPUT);
  pinMode(kPin, INPUT_PULLUP);
  pinMode(Trig, OUTPUT); 
  pinMode(Echo, INPUT); 
//----------------------------------------------------
// S2. Об'єкти
//----------------------------------------------------
  //отримання дати і часу під час компіляції
  if (getDate(__DATE__) && getTime(__TIME__)) {
    parse = true;
    // запис в RTC 
    if (RTC.write(tm)) {
      config = true;
    }
  }
  Serial.begin(9600); // ініціалізація монітору порта
  lcd.init();         // ініціалізація LCD
  lcd.backlight();    // вмикання підсвічування
  dht.begin();
}
//====================================================
// LOOP
//====================================================
void loop() {
  unsigned long currentMillis = millis(); // поточний час в мілісекундах
  controlJoystick();
  menu(currentMillis);
  eventListener(currentMillis);
  Watering(currentMillis);
  Pumping(currentMillis);
  Aeration(currentMillis);
}
//====================================================
// Меню
//====================================================
void menu(unsigned long &currentMillis){
  switch (menuLevel1){                    //вибір меню
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
  }
}
//----------------------------------------------------
// Smart Greenhouse
//----------------------------------------------------
void menu0(){
  if (refresh){lcd.clear();refresh=0;}  //очистка екрану
  lcd.setCursor(3,0);                   //встановлюємо курсор почати з 4 символа 1 рядка
  lcd.print("Welcome to");              //вивід тексту на екран
  lcd.setCursor(0,1);                   //встановлюємо курсор почати з 1 символа 2 рядка
  lcd.print("Smart Greenhouse");        //вивід тексту на екран
}
//-------------------------------------------------1.1 TIME
void menu11(unsigned long &currentMillis){  //передача ссилки на змінну 
  if (refresh){lcd.clear();refresh=0;}
  lcd.setCursor(0,0);

  if(currentMillis - previousMillisInMenuTime >= intervalInMenu) { //перевірка чи прийшов час для виконання
   previousMillisInMenuTime = currentMillis;                       //записуємо час спрацювання
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
  bool buttonIsUp = digitalRead(kPin); //перевірка чи відпущена кнопка зараз
    if (buttonWasUp && !buttonIsUp) {  //спрацює тільки в разі, коли кнопка натиснута тільки що
    delay(100);
    buttonIsUp = digitalRead(kPin);    //зчитуємо сигнал знову
    if (!buttonIsUp) {                 //якщо вона все ще натиснута ...
      isWatering = !isWatering;        //змінюємо стан
      intervalIsWatering = 15000;
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
      intervalIsPumping = 15000;
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
      intervalIsAeration = 15000;
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
// Керування джойстиком
//====================================================
void controlJoystick(){
  leeJoystick();
  if(PQCP) {
    PQCP=0;
      if (menuLevel1<2&&joyPos==3){menuLevel1++;    //вниз
        refresh=1;
        menuLevel2=0;}
      if (menuLevel1>-1&&joyPos==4){menuLevel1--;   //вгору
        menuLevel2=0;
        refresh=1;}
      if (menuLevel2<1&&joyPos==1){menuLevel2++;    //вправо
        refresh=1;}
      if (menuLevel2>-1&&joyPos==2){menuLevel2--;   //вліво
       refresh=1;}
  }
}
int leeJoystick(){                //зчитування положення джойстика 
  int x = analogRead(xPin);
  int y = analogRead(yPin);
  int k = digitalRead(kPin);
    if(x>900){joyRead=1;        //x+
    }else if(x<100){joyRead=2;  //x-
    }else if(y>900){joyRead=3;  //y+
    }else if(y<100){joyRead=4;  //y-
    }else if(!k){joyRead=5;     //кнопка
    }else{joyRead=0;}
  if (joyRead != lastJoyPos){lastDebounceTime = millis();}   
  if(((millis() - lastDebounceTime) >= debounceDelay)&&(joyRead!=joyPos)){
    joyPos=joyRead;
    if(!PQCP){PQCP=1;}
    }
  lastJoyPos=joyRead;
}
//====================================================
// Час
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

void showTime() {           //функція для зчитування часу з RTC
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

void eventListener(unsigned long &currentMillis) {   //функція, яка визиває функції опитування датчиків
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

void waterLevel() {      //функція для отримування показників з датчика виміру рівня води
  digitalWrite(Trig, LOW); 
  delayMicroseconds(2); 
  digitalWrite(Trig, HIGH); //подаємо на вхід Trig імпульс тривалістю 10 мкс 
  delayMicroseconds(10); 
  digitalWrite(Trig, LOW); 
  duration = pulseIn(Echo, HIGH); //заміряємо довжину імпульсу 
  distance = duration / 58;      
}

void outsideData() {                //функція для отримування показників з зовн. датчика
  outsideT = dht.readTemperature(); //температури
  outsideH = dht.readHumidity();    //вологості
}

void insideDataTemperature() { //функція для отримування показників з внутр. датчика температути
  byte data[2];
  ds.reset(); 
  ds.write(0xCC); //skip rom(не читати пам'ять с адресою), так як підкл. 1 датчик
  ds.write(0x44); //провести замір температури
  ds.reset();
  ds.write(0xCC);
  ds.write(0xBE); //зчитати послідно 9байт 
  data[0] = ds.read(); //в перших 2 байтах  знаходиться інформація о температурі
  data[1] = ds.read();
  insideT = (data[1]<< 8)+data[0];
  insideT = insideT>>4;
}

void insideDataHumidity() {  //функція для отримування показників з внутр. датчика вологості
humidity = analogRead(HUMIDITY);
  if (humidity > 950) {
    insideH = 200;
  } else if (100<humidity<950) {
    insideH = 100-(humidity-100)/8.5;
  }
}

void showVariables() {          //функція для відображення показників з датчиків
  if (RTC.read(tm)) {           //та станів програми в моніторі порта  
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

void Watering(unsigned long &currentMillis) {  //функція для встановлення стану поливу
 if(currentMillis - previousMillisIsWatering >= intervalIsWatering) {
    previousMillisIsWatering = currentMillis;
    intervalIsWatering = 4000;
    if(outsideH) {
     if(insideH < 40) {
      Serial.println("WATERING!");
      isWatering = true;
     } else if (insideH > 80) {
      Serial.println("STOP WATERING!");
      isWatering = false;
     }
    }
  }
}

void Pumping(unsigned long &currentMillis) { //функція для встановлення стану закачки води
   if(currentMillis - previousMillisIsPumping >= intervalIsPumping) {
    previousMillisIsPumping = currentMillis;
    intervalIsPumping = 4000;
    if(distance) {
      if(distance < 40) {
        Serial.println("PUMPING!");
        isPumping = true;
      } else if (distance > 100) {
        Serial.println("STOP PUMPING!");
        isPumping = false;
       }
    }
  }
}

void Aeration(unsigned long &currentMillis) { //функція для встановлення стану провітрювання
  if(currentMillis - previousMillisIsAeration >= intervalIsAeration) {
    previousMillisIsAeration = currentMillis;
    intervalIsAeration = 4000;
    if(insideT - outsideT > 20) {
      Serial.println("AERATION!");
      isOpen = true;
    } else if (insideT < 25) {
      Serial.println("STOP AERATION!");
      isOpen = false;
     }
  }
}
