#include <Wire.h> // Include the Arduino SPI library

const int  button0Pin = 8;
const int redPin = 10;
const int greenPin = 11;

int buttonPushCounter0 = 0;   // counter for the number of button presses
int buttonState0 = 0;         // current state of the button
int lastButtonState0 = 0;     // previous state of the button

enum AppState {ON, OFF};
enum BlinkState {SHOWING, HIDDEN};
enum PomState {POM, BRK};

AppState A_S = ON;
PomState currentPom = POM;
PomState prevPom = POM;
BlinkState currentBlink = SHOWING;

//The 7seg counter
const byte s7sAddress = 0x71;

int pomodoroLength = 25;
int shortBreak = 5;
int longBreak = 15;

int counter = pomodoroLength;  // This variable will count up to 65k
int counter2 = shortBreak;  // This variable will count up to 65k
int counter3 = longBreak;
int currentPomodoro = 1;
int pomodoros = 4;
char tempString[10];  // Will be used with sprintf to create strings

unsigned long previousMillis = 0;
unsigned long shortPreviousMillis = 0;
unsigned long flashPreviousMillis = 0;
const long interval = 1000;
const long shortInterval = 750;
const int flashInterval = 500;

void SetLeds()
{
  if(currentPom == POM)
  {
    digitalWrite(redPin, HIGH);
    digitalWrite(greenPin, LOW);
    Serial.println("RED");
  }
  else
  {
    Serial.println("GREEN");
    digitalWrite(redPin, LOW);
    digitalWrite(greenPin, HIGH);
  }
}

void SetPOM()
{
  if(currentPom != prevPom){
      prevPom = currentPom;
      SetLeds();
    }
}
void setup() {
  // initialize the button pin as a input:
  pinMode(button0Pin, INPUT);
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  SetLeds();
  
  Serial.begin(9600);
  Wire.begin();  // Initialize hardware I2C pins
  clearDisplayI2C(); 
  sprintf(tempString, "%4d", counter);
  s7sSendStringI2C(tempString);
  SetDecimals();
  counter--;
}

void loop() {
  // read the pushbutton input pin:
  buttonState0 = digitalRead(button0Pin);
  ButtonAction(0, &buttonState0, &lastButtonState0, &buttonPushCounter0);

  unsigned long currentMillis = millis();
  if (A_S == OFF && (currentMillis - flashPreviousMillis >= flashInterval)){
      flashPreviousMillis = currentMillis;
      if(A_S == OFF){
        BlinkDisplay();
      }
   }
  else if (A_S == ON && currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    if(counter > 0)
    {
     currentPom = POM;
      sprintf(tempString, "%4d", counter);
      counter--;
      s7sSendStringI2C(tempString);
      SetPOM();
    }
    else if(counter2 > 0){
      currentPom = BRK;
      
      sprintf(tempString, "%4d", counter2);
      counter2--;
      s7sSendStringI2C(tempString);
      SetPOM();
    }
    else if (A_S == ON)
    {
      clearDisplayI2C(); 
      delay(50);
        currentPomodoro++;
        if(currentPomodoro > pomodoros){
          currentPomodoro = 1;
        }
        SetDecimals();
        counter = pomodoroLength;
        currentPom = POM;
        SetPOM();
        sprintf(tempString, "%4d", counter);
        s7sSendStringI2C(tempString);

        if(currentPomodoro == 4)
          counter2 = longBreak;
        else
          counter2 = shortBreak;
        counter--;
    }
  }
}

void ButtonAction(int button, int *state, int* lastState, int* bpCounter){
  if ((int)*state != (int)*lastState) {
    if ((int)*state == HIGH) { //We're only checking for the press. Don't care about the release
      *bpCounter = *bpCounter + 1;
      if(A_S == ON)
        A_S = OFF;
      else {
        A_S = ON;
        if(currentPom == POM){
          counter = pomodoroLength;
          sprintf(tempString, "%4d", counter);
          s7sSendStringI2C(tempString);
          counter--;
        }
        else{
          counter2++;
          sprintf(tempString, "%4d", counter2);
          s7sSendStringI2C(tempString);
          counter2--;
        }
        previousMillis = millis();
        shortPreviousMillis = previousMillis;
        flashPreviousMillis = previousMillis;  
        SetDecimals();
      }
      Serial.println(A_S);
    } 
    // Delay a little bit to avoid bouncing
    delay(50);
  }
  *lastState = *state;
}

void BlinkDisplay(){
  if(currentBlink == SHOWING){
    clearDisplayI2C(); 
    currentBlink = HIDDEN;
  }
  else{
      if(currentPom == POM)
        sprintf(tempString, "%4d", counter + 1);
      else
        sprintf(tempString, "%4d", counter2 + 1);
      s7sSendStringI2C(tempString);
      SetDecimals();
      currentBlink = SHOWING;
  }
}

void SetDecimals(){
  if(currentPomodoro == 1)
    setDecimalsI2C(0b000001);
  else if(currentPomodoro == 2)
    setDecimalsI2C(0b000011);
  else if(currentPomodoro == 3)
    setDecimalsI2C(0b000111);
  else if(currentPomodoro == 4)
    setDecimalsI2C(0b001111);
}

void s7sSendStringI2C(String toSend)
{
  Wire.beginTransmission(s7sAddress);
  for (int i=0; i<4; i++)
  {
    Wire.write(toSend[i]);
  }
  Wire.endTransmission();
}

void clearDisplayI2C()
{
  Wire.beginTransmission(s7sAddress);
  Wire.write(0x76);  // Clear display command
  Wire.endTransmission();
}

void setDecimalsI2C(byte decimals)
{
  Wire.beginTransmission(s7sAddress);
  Wire.write(0x77);
  Wire.write(decimals);
  Wire.endTransmission();
}

