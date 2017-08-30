#include <TM1637Display.h>

#define CLK 5 // display clk port
#define DIO 4 // display dio port
#define intFirst 1 // entry sensor interrupt (refer to your avr board documents for this)
#define intSecond 0 // out sensor interrupt (refer to your avr board documents for this)
#define sensorDistance 0.12 // distance beetwen sensors in cm
#define showDelayTime 3000 // result showing time
#define waitDelay 20000 // wait timeout
#define screenBrightness 5 // seven segment indicator brightness level 0-6
#define ratePin 6 // digital pin of rate of fire mode

TM1637Display display(CLK, DIO);

void showReady()
{
  display.showNumberDec(0);
  delay(1000);
}

void showError() {
  display.showNumberDec(9999);
  delay(100);
}

// variables block
float bulletSpeed;
volatile long timeEntry = 0;
volatile long timeOut = 0;
volatile int shotsPerMinute = 0;
volatile long lastShot = 0;
boolean rateFlag = false;
boolean rateWelcomeFlag = false;
// end variables block

void isrEntry() { // first interrupt handler
  timeEntry = micros();
  shotsPerMinute = 60/((float)(timeEntry-lastShot)/1000000);
  lastShot = timeEntry;
}

void isrOut() { // second interrupt handler
  timeOut = micros();                                                                        
}

void setup() {
  attachInterrupt(intFirst, isrEntry, RISING);
  attachInterrupt(intSecond, isrOut, RISING);
  
  display.setBrightness(screenBrightness);

  Serial.begin(9600);
  Serial.println("chron_avail");
}

void loop() {
  showReady();
  if(digitalRead(ratePin) == HIGH) {
    if(rateWelcomeFlag == false) {
      display.showNumberDec(1);
      delay(500);
      display.showNumberDec(14);
      delay(500);
      display.showNumberDec(148);
      delay(500);
      display.showNumberDec(1488);
      delay(500);
      Serial.println("rate_mode");
      showReady();
      rateWelcomeFlag = true;
    }
    rateBeginning:
    display.showNumberDec(shotsPerMinute);
    Serial.println(shotsPerMinute);
    delay(showDelayTime);
    timeEntry = 0;
    timeOut = 0;
    if(digitalRead(ratePin) == HIGH) goto rateBeginning;
  }
  else {
    if(rateWelcomeFlag == true) Serial.println("regular_mode");
    rateWelcomeFlag = false;
    shotsPerMinute = 0;
  }
  if(timeEntry > 0) if(timeOut - timeEntry < waitDelay) {
    bulletSpeed = ((float)sensorDistance * 1000000 / (timeOut - timeEntry));
    display.showNumberDec(bulletSpeed);
    Serial.println(bulletSpeed);
    timeEntry = 0;
    timeOut = 0;
    delay(showDelayTime);
  }
  else {
    showError();
    timeEntry = 0;
    timeOut = 0;
    delay(showDelayTime);
  }
}
