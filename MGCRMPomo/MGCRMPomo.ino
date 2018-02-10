#include <Wire.h> // Include the Arduino SPI library

// this constant won't change:
const int  button0Pin = 8;
// the pin that the pushbutton is attached to
const int redPin = 10;
const int greenPin = 11;
// the pin that the LED is attached to

// Variables will change:
int buttonPushCounter0 = 0;   // counter for the number of button presses
int buttonState0 = 0;         // current state of the button
int lastButtonState0 = 0;     // previous state of the button

enum AppState {ON, OFF};

enum PomState {POM, BRK};

AppState A_S = ON;
PomState currentPom = POM;
PomState prevPom = POM;

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
const long interval = 1000;
const long shortInterval = 750;

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
  
  // initialize serial communication:
  Serial.begin(9600);
   Wire.begin();  // Initialize hardware I2C pins

   // Clear the display, and then turn on all segments and decimals
  clearDisplayI2C();  // Clears display, resets cursor
  sprintf(tempString, "%4d", counter);
  

  // Custom function to send four bytes via I2C
  //  The I2C.write function only allows sending of a single
  //  byte at a time.
  s7sSendStringI2C("25  ");
  delay(1500);
  clearDisplayI2C();
  delay(1000);
  s7sSendStringI2C(" 5  ");
  delay(1000);
  clearDisplayI2C();
}

void loop() {
  // read the pushbutton input pin:
  buttonState0 = digitalRead(button0Pin);
  ButtonAction(0, &buttonState0, &lastButtonState0, &buttonPushCounter0);

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;
    if(counter > 0)
    {
     currentPom = POM;
      sprintf(tempString, "%4d", counter);
      counter--;
      s7sSendStringI2C(tempString);
    }
    else if(counter2 > 0){
      currentPom = BRK;
      
      sprintf(tempString, "%4d", counter2);
      counter2--;
      s7sSendStringI2C(tempString);
    }
    else
    {
      clearDisplayI2C(); 
      delay(50);
        currentPomodoro++;
        if(currentPomodoro > pomodoros){
          currentPomodoro = 1;
        }
        if(currentPomodoro == 1)
          setDecimalsI2C(0b000001);
        else if(currentPomodoro == 2)
          setDecimalsI2C(0b000011);
        else if(currentPomodoro == 3)
          setDecimalsI2C(0b000111);
        else if(currentPomodoro == 4)
          setDecimalsI2C(0b001111);
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
  if (currentMillis - shortPreviousMillis >= shortInterval){
      shortPreviousMillis = currentMillis;
      SetPOM();
   }
}

void ButtonAction(int button, int *state, int* lastState, int* counter){
  // compare the buttonState to its previous state
  if ((int)*state != (int)*lastState) {
      //Serial.print("Button ");
      //Serial.print(button);
      //Serial.print(": ");
    // if the state has changed, increment the counter
    if ((int)*state == HIGH) {
      //Serial.println("HIGH !");
      // if the current state is HIGH then the button went from off to on:
      *counter = *counter + 1;
      //Serial.println("on");
      //Serial.print("number of button pushes: ");
      //Serial.println((int)*counter);
      if(A_S == ON)
        A_S = OFF;
      else
        A_S = ON;
      Serial.println(A_S);
    } else {
      // if the current state is LOW then the button went from on to off:
      //Serial.println("off");
    }
    // Delay a little bit to avoid bouncing
    delay(50);
    
  }
  
  // save the current state as the last state, for next time through the loop
  *lastState = *state;
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

