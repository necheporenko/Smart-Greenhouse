//---------------------------------------------------
// 1. Librerias
//---------------------------------------------------
#include <Wire.h>
#include <LiquidCrystal_I2C.h> 

//----------------------------------------------------
// 2. Pines
//----------------------------------------------------
#define xPin     A1   
#define yPin     A0   
#define kPin      7   
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
//----------------------------------------------------
// S2. Objetos
//----------------------------------------------------
  lcd.init();
  lcd.backlight(); 
}

//====================================================
// LOOP
//====================================================
void loop() {
  controlJoystick();
  menu();
/*  if (millis()%50==0){
    tCount1++;}
  if (tCount1>1000){tCount1=0;}*/
}

//====================================================
// Menu
//====================================================
void menu(){
  switch (menuLevel1){
    case 0:
      if(menuLevel2==1){
          menu11();
        } else if (menuLevel2 == -1){
          menu12();
        } else {
          menu0();
        }
      break;
    case -1:
      if(menuLevel2 == 1){
        menu21();
      } else if (menuLevel2 == -1){
        menu22();
      } else {
        menu1();
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
  lcd.setCursor(0,0);
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
//-------------------------------------------------1.1
void menu11(){
  if (refresh){lcd.clear();refresh=0;}
  lcd.setCursor(0,0);
  lcd.print("Menu 1.1 TIME");
}
//-------------------------------------------------1.2
void menu12(){
  if (refresh){lcd.clear();refresh=0;}
  lcd.setCursor(0,0);
  lcd.print("Menu 1.2 WATER");
}
//-------------------------------------------------1.3
void menu13(){
  if (refresh){lcd.clear();refresh=0;}
  lcd.setCursor(0,0);
  lcd.print("Menu 1.3");
}
//----------------------------------------------------
// Menu 2
//----------------------------------------------------
void menu2(){
  if (refresh){lcd.clear();refresh=0;}
  lcd.setCursor(0,0);
  lcd.print("Menu 2 SHOW t/h");
}
//-------------------------------------------------2.1
void menu21(){
  if (refresh){lcd.clear();refresh=0;}
  lcd.setCursor(0,0);
  lcd.print("Menu 2.1 OUT");
}
//-------------------------------------------------2.2
void menu22(){
  if (refresh){lcd.clear();refresh=0;}
  lcd.setCursor(0,0);
  lcd.print("Menu 2.2 IN");
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
  lcd.print("Menu 4 ");
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
  if(((millis() - lastDebounceTime) > debounceDelay)&&(joyRead!=joyPos)){
    joyPos=joyRead;
    if(!PQCP){PQCP=1;}
    }
//  if(((millis() - lastDebounceTime) > (5*debounceDelay))&&(joyPos==3||joyPos==4)){
//    joyPos=joyRead;                     //repeat time only for UP/DOWN
//    if(!PQCP){PQCP=1;}
//    }
  lastJoyPos=joyRead;
}






