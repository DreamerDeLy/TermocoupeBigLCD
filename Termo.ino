// Michael Pilcher
// Dale Gass
// Alex Gyver
// DeLy Dreamer
// Почтім память цих людей. 

#include <LiquidCrystal_I2C.h>
#include <max6675.h>
#include "GyverEncoder.h"

/////////////////////////////////////////////////////////////////
#define CLK 2
#define DT 8
#define SW 7

Encoder enc1(CLK, DT, SW);

#define thermoDO 4
#define thermoCS 5
#define thermoCLK 6

MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);

// Set the LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

#define relayPin 13

/////////////////////////////////////////////////////////////////
int screen = 1; // 1 - now, 2 - set, 3 - time
int setTemp = 40; //
unsigned int interval = 10000; // Інтервал між викл/вкл релє
unsigned int intervalM = round(interval / 1000);
unsigned long int timer = 30;
int timerUnit = 1; // 1 - хв, 2 - год.
bool relayS = true;
bool timerS = false;
bool hotS = true;
bool rendUnit = false; // Рендер одиниць таймера

unsigned long int timeConn = millis();
unsigned long int timeConn2 = millis();
unsigned long int timeConn3 = millis();

// Eight programmable character definitions
byte custom[8][8] = {
  { B11111,
    B11111,
    B11111,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000
  },

  { B11100,
    B11110,
    B11111,
    B11111,
    B11111,
    B11111,
    B11111,
    B11111
  },

  { B11111,
    B11111,
    B11111,
    B11111,
    B11111,
    B11111,
    B01111,
    B00111
  },

  { B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B11111,
    B11111,
    B11111
  },

  { B11111,
    B11111,
    B11111,
    B11111,
    B11111,
    B11111,
    B11110,
    B11100
  },

  { B11111,
    B11111,
    B11111,
    B00000,
    B00000,
    B00000,
    B11111,
    B11111
  },

  { B11111,
    B00000,
    B00000,
    B00000,
    B00000,
    B11111,
    B11111,
    B11111
  },

  { B00111,
    B01111,
    B11111,
    B11111,
    B11111,
    B11111,
    B11111,
    B11111
  }
};

// Characters, each with top and bottom half strings
// \nnn string encoding is octal, so:
// \010 = 8 decimal (8th programmable character)
// \024 = 20 decimal (space)
// \377 = 255 decimal (black square)

const char *bigChars[][2] = {
  {"\024\024\024", "\024\024\024"}, // Space
  {"\377", "\007"}, // !
  {"\005\005", "\024\024"}, // "
  {"\004\377\004\377\004", "\001\377\001\377\001"}, // #
  {"\010\377\006", "\007\377\005"}, // $
  {"\001\024\004\001", "\004\001\024\004"}, // %
  {"\010\006\002\024", "\003\007\002\004"}, // &
  {"\005", "\024"}, // '
  {"\010\001", "\003\004"}, // (
  {"\001\002", "\004\005"}, // )
  {"\001\004\004\001", "\004\001\001\004"}, // *
  {"\004\377\004", "\001\377\001"}, // +
  {"\024", "\005"}, // ,
  {"\004\004\004", "\024\024\024"}, // -
  {"\024", "\004"}, // .
  {"\024\024\004\001", "\004\001\024\024"}, // /
  {"\010\001\002", "\003\004\005"}, // 0
  {"\001\002\024", "\024\377\024"}, // 1
  {"\006\006\002", "\003\007\007"}, // 2
  {"\006\006\002", "\007\007\005"}, // 3
  {"\003\004\002", "\024\024\377"}, // 4
  {"\377\006\006", "\007\007\005"}, // 5
  {"\010\006\006", "\003\007\005"}, // 6
  {"\001\001\002", "\024\010\024"}, // 7
  {"\010\006\002", "\003\007\005"}, // 8
  {"\010\006\002", "\024\024\377"}, // 9
  {"\004", "\001"}, // :
  {"\004", "\005"}, // ;
  {"\024\004\001", "\001\001\004"}, // <
  {"\004\004\004", "\001\001\001"}, // =
  {"\001\004\024", "\004\001\001"}, // >
  {"\001\006\002", "\024\007\024"}, // ?
  {"\010\006\002", "\003\004\004"}, // @
  {"\010\006\002", "\377\024\377"}, // A
  {"\377\006\005", "\377\007\002"}, // B
  {"\010\001\001", "\003\004\004"}, // C
  {"\377\001\002", "\377\004\005"}, // D
  {"\377\006\006", "\377\007\007"}, // E
  {"\377\006\006", "\377\024\024"}, // F
  {"\010\001\001", "\003\004\002"}, // G
  {"\377\004\377", "\377\024\377"}, // H
  {"\001\377\001", "\004\377\004"}, // I
  {"\024\024\377", "\004\004\005"}, // J
  {"\377\004\005", "\377\024\002"}, // K
  {"\377\024\024", "\377\004\004"}, // L
  {"\010\003\005\002", "\377\024\024\377"}, // M
  {"\010\002\024\377", "\377\024\003\005"}, // N
  {"\010\001\002", "\003\004\005"}, // 0/0
  {"\377\006\002", "\377\024\024"}, // P
  {"\010\001\002\024", "\003\004\377\004"}, // Q
  {"\377\006\002", "\377\024\002"}, // R
  {"\010\006\006", "\007\007\005"}, // S
  {"\001\377\001", "\024\377\024"}, // T
  {"\377\024\377", "\003\004\005"}, // U
  {"\003\024\024\005", "\024\002\010\024"}, // V
  {"\377\024\024\377", "\003\010\002\005"}, // W
  {"\003\004\005", "\010\024\002"}, // X
  {"\003\004\005", "\024\377\024"}, // Y
  {"\001\006\005", "\010\007\004"}, // Z
  {"\377\001", "\377\004"}, // [
  {"\001\004\024\024", "\024\024\001\004"}, // Backslash
  {"\001\377", "\004\377"}, // ]
  {"\010\002", "\024\024"}, // ^
  {"\024\024\024", "\004\004\004"}, // _
};

int writeBigChar(char ch, int x, int y) {
  const char *(*blocks)[2] = NULL; // Pointer to an array of two strings (character pointers)

  if (ch < ' ' || ch > '_') // If outside our table range, do nothing
    return 0;

  blocks = &bigChars[ch - ' ']; // Look up the definition

  for (int half = 0; half <= 1; half++) {
    int t = x; // Write out top or bottom string, byte at a time
    for (const char *cp = (*blocks)[half]; *cp; cp++) {
      lcd.setCursor(t, y + half);
      lcd.write(*cp);
      t = (t + 1) % 40; // Circular scroll buffer of 40 characters, loop back at 40
    }
    lcd.setCursor(t, y + half);
    lcd.write(' '); // Make space between letters, in case overwriting
  }
  return strlen((*blocks)[0]); // Return char width
}

void writeBigString(char *str, int x, int y) {
  char c;
  while ((c = *str++))
    x += writeBigChar(c, x, y) + 1;
}


/*---------------------------------------------------------------
  Настройка в початку. Шо тут ше можна сказати.
  -------------------------------------------------------------*/
void setup() {
  Serial.begin(9600);

  Serial.println("Thermocouple test");

  pinMode(relayPin, OUTPUT);

  //enc1.setTickMode(AUTO);
  attachInterrupt(0, isr, CHANGE);    // прерывание на 2 пине! CLK у энка
  enc1.setType(TYPE2);

  lcd.begin();

  for (int i = 0; i < 8; i++)
    lcd.createChar(i + 1, custom[i]);

  //lcd.backlight();// Включаем подсветку дисплея
  lcd.setCursor(0, 0);
  lcd.print("Thermocouple");
  lcd.setCursor(6, 1);
  lcd.print("Loading...");
  lcd.setCursor(0, 0);

  // wait for MAX chip to stabilize
  delay(2000);

  lcd.clear();

  relayS = true;
  digitalWrite(relayPin, HIGH);

  //writeBigString("TEST", 0, 0);
  //delay(2000);

  lcd.clear();
}

/*---------------------------------------------------------------
  Основна програма. 
  Тут происходит магия:
   - Прийняття команд з СОМ
   - Опрацювання таймерів
   - Опрацювання енкодера
  -------------------------------------------------------------*/
void loop() {
  if (Serial.available()) { /////////////////////////////////////
    String Comm = Serial.readString();

    if (Comm == "1") {
      screen = 1;
      lcd.clear();
    }
    else if (Comm == "2") {
      screen = 2;
      lcd.clear();
    }
    else if (Comm == "3") {
      screen = 3;
      lcd.clear();
    } 
    else if (Comm == "4") {
      screen = 4;
      lcd.clear();
    }
    Serial.println("Screen - " + String(screen));
    Serial.println("Comm - " + Comm);
  }

  enc1.tick();  // отработка

  if (screen == 1) { ////////////////////////////////////////////
    if (enc1.isClick()) {
      AllTurn(true);
    }
    if (enc1.isRight()) {
    }
    if (enc1.isLeft()) {
    }
    if (enc1.isRightH()) {
      screen++;
      lcd.clear();
    }
    if (enc1.isLeftH()) {
      //
    }
  } else if (screen == 2) { /////////////////////////////////////
    if (enc1.isRight()) {
      setTemp++;
      Render(screen);
    }
    if (enc1.isLeft()) {
      if (setTemp > 0) setTemp--;
      Render(screen);
    }
    if (enc1.isRightH()) {
      screen++;
      lcd.clear();
    }
    if (enc1.isLeftH()) {
      screen--;
      lcd.clear();
    }
  } else if (screen == 3) { /////////////////////////////////////
    if (enc1.isRight()) {
      intervalM = round(interval / 1000);
      intervalM++;
      interval = intervalM * 1000;
      Render(screen);
    }
    if (enc1.isLeft()) {
      intervalM = round(interval / 1000);
      if (intervalM > 0) intervalM--;
      interval = intervalM * 1000;
      Render(screen);
    }
    if (enc1.isRightH()) {
      screen++;
      lcd.clear();
    }
    if (enc1.isLeftH()) {
      screen--;
      lcd.clear();
    }
  }else if (screen == 4) { //////////////////////////////////////
    if (enc1.isTurn()){
      //if (!rendUnit) rendUnit = true;
      //else rendUnit = false;
      rendUnit = false;
    }
    if (enc1.isRight()) {
      timer++;
      Render(screen);
    }
    if (enc1.isLeft()) {
      if (timer > 0) timer--;
      Render(screen);
    }

    if (enc1.isLeftH()) {
      screen--;
      lcd.clear();
    }

    enc1.tick();

    if (enc1.isClick()) {
      timeConn3 = millis();
      if (timerS){
        timerS = false;
      }else if (!timerS){
        timerS = true;
      }
      Render(screen);
    }
    if (enc1.isHolded()) {
      if (timerUnit == 1) timerUnit = 2;
      else timerUnit = 1;

      rendUnit = true;
      Render(screen);

      Serial.println(" > HOLD - "+String(timerUnit)+" - "+String(rendUnit));
    }
    /*if (enc1.isRelease()) {
      Serial.println(" > RELEASE - "+String(timerUnit)+" - "+String(rendUnit));
      rendUnit = false;
      lcd.clear();
      Render(screen);
    }*/
  }

  if (millis() - timeConn > 1000) { // Upd screen
    //Serial.println("C = " + String(thermocouple.readCelsius()));
    //Serial.println("  F = "+ String(thermocouple.readFahrenheit()));

    Render(screen);

    timeConn = millis();
  }

  if (millis() - timeConn2 > interval && hotS == true) {
    int t = String(thermocouple.readCelsius()).toInt();

    if (relayS == true && t > setTemp) {
      relayS = false;
      digitalWrite(relayPin, LOW);
      if (screen == 1) Render(screen);
    } else if (relayS == false && t < setTemp) {
      relayS = true;
      digitalWrite(relayPin, HIGH);
      if (screen == 1) Render(screen);
    }

    /*if (screen == 1){
      Render(screen);
    }*/

    Serial.println(" > Relay - " + String(relayS));
    timeConn2 = millis();
  }

  if (timerS == true) {
    if (timerUnit == 1){
      if (millis() - timeConn3 > timer*60000){
        AllTurn(false);
      } 
  } else {
      if (millis() - timeConn3 > timer*3600000){
        AllTurn(false);
      }
    }
  }
  

  //delay(10);
  //lcd.clear();
}

void AllTurn(bool State) {
  if (!State){
    if (timerS == true){
      Serial.println(" > TIMER OFF");
      timerS = false;
      relayS = false;
      hotS = false;
      digitalWrite(relayPin, LOW);
      Render(screen);
    }
  } else {
    relayS = true;
    hotS = true;
    digitalWrite(relayPin, HIGH);
    Render(screen);
  }
  
}

/*---------------------------------------------------------------
  Рендер тексту на LCD.
  -------------------------------------------------------------*/
void Render(int screen) {

  char x[5];

  if (screen == 1) { ////////////////////////////////////////////
    dtostrf(thermocouple.readCelsius(), 5, 0, x);

    writeBigString(x, -4, 0);
    //writeBigString('', 0, 0);

    lcd.setCursor(0, 0);
    lcd.print("Now");
    lcd.setCursor(0, 1);
    if (relayS == true) lcd.print("ON");
    else lcd.print("OFF");
    lcd.setCursor(0, 0);

    lcd.setCursor(15, 0);
    lcd.print("o");
    lcd.setCursor(0, 0);

  } else if (screen == 2) { /////////////////////////////////////
    char y[5];
    String str;
    str = String(setTemp);
    for(int i = str.length(); i < 3; i++){
      str = ' '+str;
    }
    str.toCharArray(y, 5);

    writeBigString(y, 4, 0);


    lcd.setCursor(0, 0);
    lcd.print("Set");
    lcd.setCursor(15, 0);
    lcd.print("o");

    lcd.setCursor(0, 0);
  } else if (screen == 3) { /////////////////////////////////////
    //Serial.println(" > SCREEN 3 RENDER " + String(intervalM));
    lcd.setCursor(0, 0);

    intervalM = round(interval / 1000);

    char z[5];
    String str2;
    str2 = String(round(interval / 1000));
    for(int i = str2.length(); i < 3; i++){
      str2 = ' '+str2;
    }
    str2.toCharArray(z, 5);

    writeBigString(z, 4, 0);

    lcd.setCursor(0, 0);
    lcd.print("Sec");
    lcd.setCursor(15, 0);
    lcd.print("'");
  } else if (screen == 4){ //////////////////////////////////////
    lcd.setCursor(0, 0);

    char a[5];
    String str3;
    str3 = String(timer);
    for(int i = str3.length(); i < 3; i++){
      str3 = ' '+str3;
    }
    str3.toCharArray(a, 5);

    writeBigString(a, 5, 0);

    lcd.setCursor(0, 0);
    lcd.print("Timer");
    lcd.setCursor(0, 1);

    if (rendUnit == true){
      if (timerUnit == 1) lcd.print("Min ");
      else lcd.print("Hour");
    } else {
      if (timerS == true) lcd.print("ON  ");
      else lcd.print("OFF ");
    }

    //lcd.setCursor(0, 0);
    //lcd.print(timerM);
  }
  
}

void isr() {
  enc1.tick();  // отработка в прерывании
}

/*
  When you feel my heat
  Look into my eyes
  It’s where my demons hide
  It’s where my demons hide
  Don’t get too close
  It’s dark inside
  It’s where my demons hide
  It’s where my demons hide
*/