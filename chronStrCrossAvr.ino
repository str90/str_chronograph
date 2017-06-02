#include <TM1637.h>

#define CLK 5 // display clk port
#define DIO 4 // display dio port
#define intFirst 1 // entry sensor interrupt (refer to your avr board documents for this)
#define intSecond 0 // out sensor interrupt (refer to your avr board documents for this)
#define sensorDistance 0.12 // distance beetwen sensors in cm
#define showDelayTime 3000 // result showing time
#define waitDelay 20000 // wait timeout
#define screenBrightness 5 // seven segment indicator brightness level 0-6
#define ratePin 6 // digital pin of rate of fire mode

TM1637 segmentIndicator(CLK, DIO);

void setSegments(byte addr, byte data)
{
  segmentIndicator.start();
  segmentIndicator.writeByte(ADDR_FIXED);
  segmentIndicator.stop();
  segmentIndicator.start();
  segmentIndicator.writeByte(addr|0xc0);
  segmentIndicator.writeByte(data);
  segmentIndicator.stop();
  segmentIndicator.start();
  segmentIndicator.writeByte(segmentIndicator.Cmd_DispCtrl);
  segmentIndicator.stop();
}

void showReady()
{
  setSegments(0, 73);
  setSegments(1, 73);
  setSegments(2, 73);
  setSegments(3, 73);
  delay(100);
}

void showError()
{
  setSegments(0, 121);
  setSegments(1, 80);
  setSegments(2, 80);
  setSegments(3, 0);
  delay(showDelayTime);
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
  segmentIndicator.set(screenBrightness);
}

void loop() {
  showReady();
  if(digitalRead(ratePin) == LOW) {
    if(rateWelcomeFlag == false) {
      segmentIndicator.display(0, 1);
      delay(700);
      segmentIndicator.display(1, 4);
      delay(700);
      segmentIndicator.display(2, 8);
      delay(700);
      segmentIndicator.display(3, 8);
      delay(700);
      showReady();
      rateWelcomeFlag = true;
    }
    rateBeginning:
    segmentIndicator.display(0, (int)shotsPerMinute / 1000);
    segmentIndicator.display(1, (int)shotsPerMinute / 100 % 10);
    segmentIndicator.display(2, (int)shotsPerMinute / 10 % 10);
    segmentIndicator.display(3, (int)shotsPerMinute % 10);
    delay(showDelayTime);
    timeEntry = 0;
    timeOut = 0;
    if(digitalRead(ratePin) == LOW) goto rateBeginning;
  }
  else {
    rateWelcomeFlag = false;
    shotsPerMinute = 0;
  }
  if(timeEntry > 0) if(timeOut - timeEntry < waitDelay) {
    bulletSpeed = ((float)sensorDistance * 1000000 / (timeOut - timeEntry));
    segmentIndicator.display(0, (int)bulletSpeed / 1000);
    segmentIndicator.display(1, (int)bulletSpeed / 100 % 10);
    segmentIndicator.display(2, (int)bulletSpeed / 10 % 10);
    segmentIndicator.display(3, (int)bulletSpeed % 10);
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
