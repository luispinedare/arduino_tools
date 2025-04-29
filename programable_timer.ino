//Library
#include "ShiftDisplay2.h"
#include "RTClib.h"

RTC_DS3231 rtc;

const int clock_pin = 17;
const int brightPin = 7;
const int LATCH_PIN = 6;
const int CLOCK_PIN = 5;
const int DATA_PIN = 8;
const DisplayType DISPLAY_TYPE = COMMON_ANODE; // COMMON_CATHODE or COMMON_ANODE
const int DISPLAY_SIZE = 4; // number of digits on display
ShiftDisplay2 display(LATCH_PIN, CLOCK_PIN, DATA_PIN, DISPLAY_TYPE, DISPLAY_SIZE);

// Rotary Encoder Inputs
#define CLK 2
#define DT 3
#define SW 4
#define relay 15
#define led 16

//Rotary Encoder Variables
int currentStateCLK;
int lastStateCLK;
int lastButtonPress = 0;
unsigned long startTime;
unsigned long endTime;
unsigned long duration;
byte timerRunning;

//Variables
int bright = 0;
int digit1;
int digit2;
int digit3;
int digit4;
String sec0 = "00";
String min0 = "00";
String sec1 = "de";
String min1 = "00";
String sec2 = "st";
String min2 = "00";
String sec3 = "fi";
String min3 = "00";
String currentTime;
int minutes, seconds, deciseconds = 0;
int program;
int setPoint = 1;

//Useful flags
boolean countFlag = false;
boolean setFlag = true;
boolean wait = false;
boolean blinkDisp = false;

int var = 0;
unsigned long pulse = 0;
// keep time
unsigned long  previousMillis = 0;
int interval = 99;

// led timer
unsigned long ledA;
unsigned long ledB = 0;
int ledTime;
int ledState = LOW;

void setup()
{
  // Set encoder pins as inputs
  pinMode(CLK, INPUT);
  pinMode(DT, INPUT);
  pinMode(SW, INPUT_PULLUP);

  pinMode(clock_pin, INPUT);

  // Setup Serial Monitor
  Serial.begin(57600);

  // Read the initial state of CLK
  lastStateCLK = digitalRead(CLK);

  // Output
  pinMode (relay, OUTPUT);
  pinMode (led, OUTPUT);

  pinMode(brightPin, OUTPUT);
  analogWrite(brightPin, bright);

  // Clock Setup!!!
#ifndef ESP8266
  while (!Serial); // wait for serial port to connect. Needed for native USB
#endif

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, let's set the time!");
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }

  // When time needs to be re-set on a previously configured device, the
  // following line sets the RTC to the date & time this sketch was compiled
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  // This line sets the RTC with an explicit date & time, for example to set
  // January 21, 2014 at 3am you would call:
  // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
}



void loop()
{
  if(digitalRead(clock_pin) > var)
  {
    var = 1;
    pulse++;
    if(pulse == 1024){
    pulse=0;  
    }
  }
//  if(digitalRead(clock_pin) == 0) {var = 0;}
//  Serial.println(pulse);
}

void dsadsad(){  
  if (timerRunning == 0 && digitalRead(SW) == LOW && program != 4) {  // button pressed & timer not running already
    startTime = millis();
    timerRunning = 1;
  }
  if (timerRunning == 1 && digitalRead(SW) == HIGH && program != 4) { // timer running, button released
    endTime = millis();
    timerRunning = 0;
    duration = endTime - startTime;
    if (duration <= 500 && !countFlag && !setFlag) {    //Start
      countFlag = true;
      if (program == 0) {
        seconds = sec0.toInt();
        minutes = min0.toInt();
      }
      else if (program == 1) {
        seconds = sec1.toInt();
        minutes = min1.toInt();
      }
      else if (program == 2) {
        seconds = sec2.toInt();
        minutes = min2.toInt();
      }
      else if (program == 3) {
        seconds = sec3.toInt();
        minutes = min3.toInt();
      }

      countdown();  //Start Countdown routine

    }
    else if (duration <= 500 && setFlag && !countFlag) {  //Set Time Position
      if (setPoint < 4) {
        setPoint++;
      }
      else {
        setPoint = 0;
        setFlag = false;
      }
    }
    else if (duration > 500 && !setFlag && !countFlag) {  //Enter Set Time
      setPoint = 1;
      setFlag = true;
    }
    else if (duration > 500 && setFlag && !countFlag) {   //Exit Set Time
      setPoint = 0;
      setFlag = false;
    }
  }

  else if (!setFlag && !countFlag) {  //Program Routines
    setProgram();
  }

  else if (setFlag && !countFlag) {   //Set Routines
    setDigit();
    blinkNumbers();
  }
  ledIndicator();
  display.update();
}



void countdown()
{
  unsigned long currentMillis;
  int i = 0;
  int deciseconds = 0;

  while (countFlag) {
    ledIndicator();
    if (minutes == 0 && seconds == 0 && deciseconds == 0) {
      digitalWrite(relay, LOW);
      digitalWrite(led, LOW);
      countFlag = false;
      if (program != 0) {
        program ++;
        if (program > 3) {
          program = 1;
        }
      }
      for (int i = 0; i < 3; i++) {
        display.set("0000");
        display.show(500);
        display.set("    ");
        display.show(500);
        display.update();
      }
    }
    else {
      if (program == 0) {
        digitalWrite(relay, HIGH);
      }
      currentMillis = millis();
      if (currentMillis - previousMillis > interval) {
        previousMillis = currentMillis;
        deciseconds --;
        if (deciseconds < 0) {
          deciseconds = 9;
          seconds --;
          if (seconds < 0) {
            seconds = 59;
            minutes --;
            if (minutes < 0) {
              minutes = 0;
            }
          }
        }
        if (seconds >= 10 ) {
          display.set(String(minutes) + String(seconds) + String(deciseconds));
        }
        else if (seconds < 10) {
          display.set(String(minutes) + String("0") + String(seconds) + String(deciseconds));
        }
        if (minutes >=10){
          display.changeDot(1);
          if (digitalRead(clock_pin) == HIGH){
            display.changeDot(3);
          }
        }
        else if (minutes < 10){
          display.changeDot(0);
          if (digitalRead(clock_pin) == HIGH){
            display.changeDot(2);
          }
        }
      }
      if (digitalRead(SW) == LOW && timerRunning == 0) { // button pressed & timer not running already
        startTime = millis();
        timerRunning = 1;
      }
      if (digitalRead(SW) == HIGH && timerRunning == 1) { // timer running, button released
        endTime = millis();
        timerRunning = 0;
        duration = endTime - startTime;
        digitalWrite(relay, LOW);
        if (duration <= 500) { //Stop
          wait = true;
          while (wait) {
            ledIndicator();
            if (timerRunning == 0 && digitalRead(SW) == LOW) {
              startTime = millis();
              timerRunning = 1;
            }
            if (timerRunning == 1 && digitalRead(SW) == HIGH) {
              endTime = millis();
              timerRunning = 0;
              duration = endTime - startTime;
              if (duration <= 500) { //Start
                wait = false;
              }
              else if (duration > 500) { //Cancel CountDown
                wait = false;
                countFlag = false;
              }
            }
            display.update();
          }
        }
        else if (duration > 500) { //Cancel CountDown
          countFlag = false;
        }
      }
    }
    display.update();
  }
}



void setProgram()
{
  DateTime now = rtc.now();
  
  currentStateCLK = digitalRead(CLK);
  if (currentStateCLK != lastStateCLK  && currentStateCLK == 1) {
    program = rotaryFunction(program, 4);
  }
  // Remember last CLK state
  lastStateCLK = currentStateCLK;
  if (program == 0) {
    resetDigit(sec0, min0);
    display.set(String(min0) + String(sec0));
  }
  else if (program == 1) {
    resetDigit(sec1, min1);
    display.set(String(min1) + String(sec1));
  }
  else if (program == 2) {
    resetDigit(sec2, min2);
    display.set(String(min2) + String(sec2));
  }
  else if (program == 3) {
    resetDigit(sec3, min3);
    display.set(String(min3) + String(sec3));
  }
  else if (program == 4) {
    char buf1[] = "mmss";
    currentTime = (String(now.toString(buf1)));
    display.set(currentTime, ALIGN_RIGHT);
    if (pulse < 51){
      display.changeDot(3, true);
    }
    if (pulse > 51){
      display.changeDot(3, false);
    }
    // brightProgram();
  }
}



void setDigit()   // Clicks Does
{
  currentStateCLK = digitalRead(CLK);
  if (currentStateCLK != lastStateCLK  && currentStateCLK == 1) {
    if (setPoint == 1) {
      digit1 = rotaryFunction(digit1, 9);
    }
    if (setPoint == 2) {
      digit2 = rotaryFunction(digit2, 5);
    }
    if (setPoint == 3) {
      digit3 = rotaryFunction(digit3, 9);
    }
    if (setPoint == 4) {
      digit4 = rotaryFunction(digit4, 9);
    }
  }
  if (program == 0) {
    sec0 = String(digit2) + String(digit1);
    min0 = String(digit4) + String(digit3);
  }
  else if (program == 1) {
    sec1 = String(digit2) + String(digit1);
    min1 = String(digit4) + String(digit3);
  }
  else if (program == 2) {
    sec2 = String(digit2) + String(digit1);
    min2 = String(digit4) + String(digit3);
  }
  else if (program == 3) {
    sec3 = String(digit2) + String(digit1);
    min3 = String(digit4) + String(digit3);
  }
  // Remember last CLK state
  lastStateCLK = currentStateCLK;
}



void blinkNumbers()
{
  if (pulse < 102)
  {
    display.set(String(digit4) + String(digit3) + String(digit2) + String(digit1));
  }    
  if(pulse >= 102) {
    if (setPoint == 1) {
      display.set(String(" ") + String(" ") + String(" ") + String(digit1));
    }
    else if (setPoint == 2) {
      display.set(String(" ") + String(" ") + String(digit2) + String(" "));
    }
    else if (setPoint == 3) {
      display.set(String(" ") + String(digit3) + String(" ") + String(" "));
    }
    else if (setPoint == 4) {
      display.set(String(digit4) + String(" ") + String(" ") + String(" "));
    }  
  }
}



void ledIndicator()
{
  ledA = millis();
  if (ledA - ledB >= 50) {
    ledB = ledA;
    ledTime ++;
  }
  if (program == 0) {
    ledState = HIGH;
  }
  else if (program == 1) {
    blinkLedIndicator(1, HIGH);
    blinkLedIndicator(2, LOW);
  }
  else if (program == 2) {
    blinkLedIndicator(1, HIGH);
    blinkLedIndicator(2, LOW);
    blinkLedIndicator(7, HIGH);
    blinkLedIndicator(8, LOW);
  }
  else if (program == 3) {
    blinkLedIndicator(1, HIGH);
    blinkLedIndicator(2, LOW);
    blinkLedIndicator(7, HIGH);
    blinkLedIndicator(8, LOW);
    blinkLedIndicator(13, HIGH);
    blinkLedIndicator(14, LOW);
  }
  else if (program == 4) {
    ledState = LOW;
  }
  digitalWrite(led, ledState);
  if (ledTime == 40)  {
    ledTime = 0;
  }
}



void blinkLedIndicator(int x, byte y)
{
  if (ledTime == x)  {
    ledState = y;
  }
}



void brightProgram()
{
  if (timerRunning == 0 && digitalRead(SW) == LOW && program == 4) {
    startTime = millis();
    timerRunning = 1;
  }
  if (timerRunning == 1 && digitalRead(SW) == HIGH && program == 4) {
    endTime = millis();
    timerRunning = 0;
    duration = endTime - startTime;
    if (duration > 50) {
      wait = true;
      while (wait) {
        if (setPoint == 0 && !countFlag) {
          currentStateCLK = digitalRead(CLK);
          if (currentStateCLK != lastStateCLK  && currentStateCLK == 1) {
            if (digitalRead(DT) != currentStateCLK) {
              bright = bright - 10;
              if (bright < 0) {
                bright = 0;
              }
            } else {
              bright = bright + 10;
              if (bright > 250) {
                bright = 250;
              }
            }
            analogWrite(brightPin, bright);
          }
          // Remember last CLK state
          lastStateCLK = currentStateCLK;
        }
        if (timerRunning == 0 && digitalRead(SW) == LOW) {
          startTime = millis();
          timerRunning = 1;
        }
        if (timerRunning == 1 && digitalRead(SW) == HIGH) {
          endTime = millis();
          timerRunning = 0;
          duration = endTime - startTime;
          if (duration > 50) {
            wait = false;
          }
        }
        display.update();
      }
    }
  }
}



int rotaryFunction(int x, int y)
{
  if (digitalRead(DT) != currentStateCLK) {
    x ++;
    if (x > y) {
      x = 0;
    }
  }
  else {
    x --;
    if (x < 0) {
      x = y;
    }
  }
  return x;
}



int resetDigit(String x, String y)
{
  String a;
  String b;
  String c;
  String d;

  a = x[1];
  b = x[0];
  c = y[1];
  d = y[0];

  digit1 = a.toInt();
  digit2 = b.toInt();
  digit3 = c.toInt();
  digit4 = d.toInt();

  return digit1;
  return digit2;
  return digit3;
  return digit4;
}
