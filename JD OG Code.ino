volatile byte  count = 0;
byte numCount = 8; //number of pulse intervals to measure

volatile unsigned long startTime;
volatile unsigned long endTime;
unsigned long copy_startTime;
unsigned long copy_endTime;

unsigned long lastSwitchTime = 0;

volatile boolean finishCount = false;
float period;
unsigned int rpm = 0;
bool CW = true;

bool autoMode = false;

int relay1 = 3;
int relay2 = 4;

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

int forwardTime = 20000;    //miliseconds
int reverseTime = 20000;    //miliseconds
int number = 2000;         //miliseconds

//=============================================================

void calculateRPM(){
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

//=============================================================

void setup(){
  Serial.begin(9600);

  //pinMode(relay1, OUTPUT);
  //pinMode(relay2, OUTPUT);
  
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
  Serial.println("Button 1 - Manual");
  Serial.println("Button 2 - Automatic");

  // determine desired mode
  while(true){
    buttonAState = digitalRead(buttonA);
    buttonBState = digitalRead(buttonB);
    if(buttonAState == LOW){
      Serial.println("Manual Control Selected");
      digitalWrite(ledPin, HIGH);
      delay(1000);
      digitalWrite(ledPin, LOW);
      break;
    }
    if(buttonBState == LOW){
      Serial.println("Automatic Control Selected");
      digitalWrite(ledPin, HIGH);
      delay(1000);
      digitalWrite(ledPin, LOW);
      autoMode = true;
      break;
    }
  }

  //attach interrupt to pin 2 for reading RPMs
  attachInterrupt(digitalPinToInterrupt(2), isrCount, FALLING);
  
}

//=============================================================

void loop(){
  if(autoMode == true){
    if(millis() - lastSwitchTime > number || lastSwitchTime == 0){
      lastSwitchTime = millis;
      if(CW == true){
        digitalWrite(relay1, LOW);// turn relay 1 ON
        digitalWrite(relay2, HIGH);// turn relay 2 OFF  
        Serial.println("Rotating CCW"); // Rotate in CCW direction
        CW = false;
      }
      else{
        digitalWrite(relay1, HIGH);// turn relay 1 OFF
        digitalWrite(relay2, LOW);// turn relay 2 ON 
        Serial.println("Rotating CW"); // Rotate in CW direction
        CW = true;
      } 
    }
    
    calculateRPM();
  }
  else if(autoMode == false){
    //manually control relays with buttons here
    
    calculateRPM();
  }
}

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
