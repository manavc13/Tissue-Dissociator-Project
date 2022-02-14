volatile byte  count = 0;
byte numCount = 8; //number of pulse intervals to measure

volatile unsigned long startTime;
volatile unsigned long endTime;
unsigned long copy_startTime;
unsigned long copy_endTime;

volatile boolean finishCount = false;
float period;
unsigned int rpm = 0;
bool CW = true;

bool autoMode = false;

int relay1 = 3;
int relay2 = 4;
int relayState = LOW;   // the current state of relay

const int buttonA = 5;     // the number of the pushbutton pin
const int buttonB = 6;
const int buttonC = 7;
const int buttonD = 8;
const int ledPin =  13;      // the number of the LED pin


// variables will change:
int buttonAState = 0;         // variable for reading the pushbutton status
int buttonBState = 0;  
int buttonCState = 0;
int buttonDState = 0;  

unsigned long lastSwitchTime = 0;
int forwardTime;
int reverseTime;    
bool directionFlag = true;

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x3F,20,4);

void(* resetFunc) (void) = 0;

//=============================================================

void setup(){
  Serial.begin(9600);

  lcd.init();
  lcd.backlight();
  
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  
  //digitalWrite(relay1, LOW);// turn relay 1 OFF
  //digitalWrite(relay2, LOW);// turn relay 2 OFF

  // initialize the LED pin as an output:
  pinMode(ledPin, OUTPUT);

  
  // initialize the pushbutton pin as an input:
  pinMode(buttonA, INPUT_PULLUP);
  pinMode(buttonB, INPUT_PULLUP);
  pinMode(buttonC, INPUT_PULLUP);
  pinMode(buttonD, INPUT_PULLUP);

  // select desired mode of control
  Serial.println("Red   - Manual");
  Serial.println("Green - Auto");

   lcd.setCursor(0,0);
   lcd.print("TD");
   lcd.setCursor(0,1);
   lcd.print("Version 1.0.0");
   delay(3000);

   lcd.clear();
   lcd.setCursor(0,0);
   lcd.print("Red   - Manual");
   lcd.setCursor(0,1);
   lcd.print("Green - Auto");

  // determine desired mode
  while(true){
    buttonAState = digitalRead(buttonA);
    buttonBState = digitalRead(buttonB);
    if(buttonAState == LOW){
      Serial.println("Manual Control Selected");
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Manual Control");
      lcd.setCursor(0,1);
      lcd.print("Selected");
      
      digitalWrite(ledPin, HIGH);
      delay(1000);
      digitalWrite(ledPin, LOW);
      break;
    }
    if(buttonBState == LOW){
      Serial.println("Automatic Control Selected");
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Auto Control");
      lcd.setCursor(0,1);
      lcd.print("Selected");
      
      digitalWrite(ledPin, HIGH);
      delay(1000);
      digitalWrite(ledPin, LOW);
      autoMode = true;
      delay(1000);

      // pick type of auto mode
      Serial.println("Pick AutoMode (1-4)");
      
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Pick Auto Mode:");
      lcd.setCursor(0,1);
      lcd.print("R G B");
      
      while(true){
        buttonAState = digitalRead(buttonA);
        buttonBState = digitalRead(buttonB);
        buttonCState = digitalRead(buttonC);
        buttonDState = digitalRead(buttonD);
        if(buttonAState == LOW){
          Serial.println("Auto 1 Selected");

          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Auto - R");
          lcd.setCursor(0,1);
          lcd.print("Selected");
          delay(1000);
          
          forwardTime = 30000;    // miliseconds  (30 s)
          reverseTime = 5000;     // miliseconds  (5 s)
          break;
        }
        if(buttonBState == LOW){
          Serial.println("Auto 2 Selected");

          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Auto - G");
          lcd.setCursor(0,1);
          lcd.print("Selected");
          delay(1000);
          
          forwardTime = 2000;
          reverseTime = 2000;    
          break;
        }
        if(buttonCState == LOW){
          Serial.println("Auto 3 Selected");

          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Auto - B");
          lcd.setCursor(0,1);
          lcd.print("Selected");
          delay(1000);
          
          forwardTime = 3000;
          reverseTime = 3000;    
          break;
        }
        /*
        if(buttonDState == LOW){
          Serial.println("Auto 4 Selected");

          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Auto - Y");
          lcd.setCursor(0,1);
          lcd.print("Selected");
          delay(1000);
          
          forwardTime = 4000;
          reverseTime = 4000;    
          break;
        } */
      }
      break;
    }
  }

  //attach interrupt to pin 2 for reading RPMs
  attachInterrupt(digitalPinToInterrupt(2), isrCount, FALLING);
  
}

//=============================================================

void loop(){
  // Auto Control Code
  if(autoMode == true){
    
    //start rotating in a direction
    digitalWrite(relay1, relayState);
    digitalWrite(relay2, !relayState);
     
    
    if(millis()-lastSwitchTime >= forwardTime && directionFlag == false){
      lastSwitchTime = millis();
      relayState = !relayState;
      directionFlag = !directionFlag;
      digitalWrite(relay1, relayState);
      digitalWrite(relay2, !relayState);
    }
    else if(millis()-lastSwitchTime >= reverseTime && directionFlag == true){
      lastSwitchTime = millis();
      relayState = !relayState;
      directionFlag = !directionFlag;
      digitalWrite(relay1, relayState);
      digitalWrite(relay2, !relayState);
    }
    
      if (finishCount == true){
      finishCount = false;//reset flag
      // disable interrupts, make protected copy of time values
      noInterrupts();
      copy_startTime = startTime;
      copy_endTime = endTime;
      count = 0;
      interrupts();
  
      period = (copy_endTime - copy_startTime);
      
      //debug prints
      //Serial.print(period); //total time for numCount
      //Serial.print('\t');
      //Serial.println(period/numCount);//time between individual pulses
  
      rpm = 1/((period/numCount)/60000.0); //rotation (rev) per time (min)
      //rpm = numCount * 60.0 * (1000.0 / period);//one count per revolution  (if we were to use 1 magnet -- same output as above equation)
      //rpm = numCount * 30.0 * (1000.0 / period);//two counts per revolution (if we were to use 2 magnets)
  
      //UPDATE RPM Here
      Serial.print("RPM = ");
      Serial.println(rpm);

      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("RPM = ");
      lcd.setCursor(0,1);
      lcd.print(rpm);
      
    }
     
  }
  
  // Manual Control Code
  else if(autoMode == false){
    
    // get button states
    buttonAState = digitalRead(buttonA);
    buttonBState = digitalRead(buttonB);
    buttonCState = digitalRead(buttonC);
    buttonDState = digitalRead(buttonD);

    // if button A pressed, rotate CCW
    if(buttonAState == LOW){
      digitalWrite(relay1, LOW);// turn relay 1 ON
      digitalWrite(relay2, HIGH);// turn relay 2 OFF  
      Serial.println("Rotating CCW"); // Rotate in CCW direction
    }
    
    // if button B pressed, rotate CW
    else if(buttonBState == LOW){
      digitalWrite(relay1, HIGH);// turn relay 1 ON
      digitalWrite(relay2, LOW);// turn relay 2 OFF  
      Serial.println("Rotating CW"); // Rotate in CCW direction
    }
    
    // if button C pressed, OFF
    else if(buttonCState == LOW){
      digitalWrite(relay1, LOW);// turn relay 1 ON
      digitalWrite(relay2, LOW);// turn relay 2 OFF  
      Serial.println("OFF"); // Rotate in CCW direction
    }

    else if(buttonDState == LOW) {
      resetFunc();
    }

      if (finishCount == true){
      finishCount = false;//reset flag
      // disable interrupts, make protected copy of time values
      noInterrupts();
      copy_startTime = startTime;
      copy_endTime = endTime;
      count = 0;
      interrupts();
  
      period = (copy_endTime - copy_startTime);
      
      //debug prints
      //Serial.print(period); //total time for numCount
      //Serial.print('\t');
      //Serial.println(period/numCount);//time between individual pulses
  
      rpm = 1/((period/numCount)/60000.0); //rotation (rev) per time (min)
      //rpm = numCount * 60.0 * (1000.0 / period);//one count per revolution  (if we were to use 1 magnet -- same output as above equation)
      //rpm = numCount * 30.0 * (1000.0 / period);//two counts per revolution (if we were to use 2 magnets)
  
      //UPDATE RPM Here
      Serial.print("RPM = ");
      Serial.println(rpm);
    }
  }

  
} // end of void loop

//=============================================================

void isrCount()
{
  if (count == 0)//first entry to isr
  {
    startTime = millis();
  }

  if (count == numCount)
  {
    endTime = millis();
    finishCount = true;    
  }
  count++; //increment after test for numCount
}
