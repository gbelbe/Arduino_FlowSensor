/*
Liquid flow rate sensor -DIYhacking.com Arvind Sanjeev

Measure the liquid/water flow rate using this code. 
Connect Vcc and Gnd of sensor to arduino, and the 
signal line to arduino digital pin 2.
 
 */

// include the library code:
#include <LiquidCrystal.h>
#include <EEPROM.h>


//========= for Bluetooth test

int state = 0;
int flag = 0;        // make sure that you return the state only once





// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 10, 9, 8, 7);

// the 8 arrays that form each segment of the custom numbers
byte bar1[8] = 
{
        B11100,
        B11110,
        B11110,
        B11110,
        B11110,
        B11110,
        B11110,
        B11100
};
byte bar2[8] =
{
        B00111,
        B01111,
        B01111,
        B01111,
        B01111,
        B01111,
        B01111,
        B00111
};
byte bar3[8] =
{
        B11111,
        B11111,
        B00000,
        B00000,
        B00000,
        B00000,
        B11111,
        B11111
};
byte bar4[8] =
{
        B11110,
        B11100,
        B00000,
        B00000,
        B00000,
        B00000,
        B11000,
        B11100
};
byte bar5[8] =
{
        B01111,
        B00111,
        B00000,
        B00000,
        B00000,
        B00000,
        B00011,
        B00111
};
byte bar6[8] =
{
        B00000,
        B00000,
        B00000,
        B00000,
        B00000,
        B00000,
        B11111,
        B11111
};
byte bar7[8] =
{
        B00000,
        B00000,
        B00000,
        B00000,
        B00000,
        B00000,
        B00111,
        B01111
};
byte bar8[8] =
{
        B11111,
        B11111,
        B00000,
        B00000,
        B00000,
        B00000,
        B00000,
        B00000
};



byte statusLed    = 4;
byte beeper = 5;
byte sensorInterrupt = 0;  // 0 = digital pin 2
byte sensorPin       = 2;


// Quantity until alarm is triggered
#define quantityAlarm 3000


// The hall-effect flow sensor outputs approximately 4.5 pulses per second per
// litre/minute of flow.
float calibrationFactor = 7.5;

volatile byte pulseCount;  

float flowRate;
unsigned int flowMilliLitres;
unsigned long totalMilliLitres;
unsigned int totalLiters;
unsigned int lastScore;


unsigned long oldTime;

void setup()
{
  
  // Initialize a serial connection for reporting values to the host
  Serial.begin(9600);
   
  // Set up the status LED line as an output
  pinMode(statusLed, OUTPUT);
  pinMode(beeper, OUTPUT);
   
  
    // set up the LCD's number of columns and rows:
 
  
//Start Check-up, LED blinks
  
  digitalWrite(statusLed, HIGH);  // We have an active-low LED attached
  delay(1000);
  digitalWrite(statusLed, LOW);  // We have an active-low LED attached
  delay(1000);
  digitalWrite(statusLed, HIGH);  // We have an active-low LED attached
  delay(1000);
  digitalWrite(statusLed, LOW);  // We have an active-low LED attached


  
  pinMode(sensorPin, INPUT);
  digitalWrite(sensorPin, HIGH);

  pulseCount        = 0;
  flowRate          = 0.0;
  flowMilliLitres   = 0;
  totalMilliLitres  = 0;
  totalLiters       = 0;
  oldTime           = 0;
  lastScore         = 0;
  
  // The Hall-effect sensor is connected to pin 2 which uses interrupt 0.
  // Configured to trigger on a FALLING state change (transition from HIGH
  // state to LOW state)
  attachInterrupt(sensorInterrupt, pulseCounter, FALLING);


//initialize LCD screen
// sets the LCD's rows and colums:
  lcd.begin(16, 2);


//Welcome Screen
lcd.print("Welcome");
delay(1000);
 lcd.clear();

//check if there is a last score saved on the EEProm;
   lastScore = EEPROM.read(0);
 
 // debug serial
   Serial.println("Last Score:");
   Serial.print(lastScore);
  
  lcd.print("Last score");
  lcd.setCursor(0, 1);
  // print the number of seconds since reset:
 lcd.print(lastScore);
 lcd.print(" Liters");
  
 delay(2000); 
 lcd.clear();

  lcd.print("Try using less");
  
 delay(2000); 
 lcd.clear();


  // assignes each segment a write number
  lcd.createChar(1,bar1);
  lcd.createChar(2,bar2);
  lcd.createChar(3,bar3);
  lcd.createChar(4,bar4);
  lcd.createChar(5,bar5);
  lcd.createChar(6,bar6);
  lcd.createChar(7,bar7);
  lcd.createChar(8,bar8);





}

/**
 * Main program loop
 */
void loop()
{
   
  // test bluetooth connection:
  //===================
  
  
   //if some data is sent, read it and save it in the state variable
    if(Serial.available() > 0){
      state = Serial.read();
      flag=0;
    }
    // if the state is 0 the led will turn off
    if (state == '0') {
       digitalWrite( beeper, LOW);
       digitalWrite(statusLed, LOW);  // We have an active-low LED attached      
        if(flag == 0){
          
          Serial.println("LED: off");
          flag = 1;
        }
    }
    // if the state is 1 the led will turn on
    else if (state == '1') {
         digitalWrite( beeper, HIGH);
         digitalWrite(statusLed, HIGH);  // We have an active-low LED attached      
        if(flag == 0){
          Serial.println("LED: on");
          flag = 1;
        }
    }
  
  
  
  
  
  
  
  //==================
  
   if((millis() - oldTime) > 1000)    // Only process counters once per second
  { 
    // Disable the interrupt while calculating flow rate and sending the value to
    // the host
    detachInterrupt(sensorInterrupt);
        
    // Because this loop may not complete in exactly 1 second intervals we calculate
    // the number of milliseconds that have passed since the last execution and use
    // that to scale the output. We also apply the calibrationFactor to scale the output
    // based on the number of pulses per second per units of measure (litres/minute in
    // this case) coming from the sensor.
    flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / calibrationFactor;
    
    // Note the time this processing pass was executed. Note that because we've
    // disabled interrupts the millis() function won't actually be incrementing right
    // at this point, but it will still return the value it was set to just before
    // interrupts went away.
    oldTime = millis();
    
    // Divide the flow rate in litres/minute by 60 to determine how many litres have
    // passed through the sensor in this 1 second interval, then multiply by 1000 to
    // convert to millilitres.
    flowMilliLitres = (flowRate / 60) * 1000;
    
    // Add the millilitres passed in this second to the cumulative total
    totalMilliLitres += flowMilliLitres;
      
      
   
   
   
   //    lcd.begin(16, 2);
  // Print a message to the LCD.
 
   totalLiters = totalMilliLitres / 1000;
   
   
   //print totalLiters on the LCD Display
   
   LCDprintNumber(totalLiters);
 
    
 
 //========Alarm 5 liters
      
    if (totalMilliLitres > quantityAlarm){
       
       digitalWrite( beeper, HIGH);
       digitalWrite(statusLed, HIGH);  // We have an active-low LED attached

    //write last score na EEprom
       EEPROM.write(0, totalLiters);

    } 
    
    if (totalMilliLitres > quantityAlarm+1000){
       digitalWrite( beeper, LOW);
       digitalWrite(statusLed, LOW);  // We have an active-low LED attached  
       
       
       
    }
      
 //========Alarm 10 liters
    if (totalMilliLitres > quantityAlarm*2){
       
       digitalWrite( beeper, HIGH);
       digitalWrite(statusLed, HIGH);  // We have an active-low LED attached
    } 
    
    if (totalMilliLitres > quantityAlarm*2+1000){
       digitalWrite( beeper, LOW);
       digitalWrite(statusLed, LOW);  // We have an active-low LED attached      
    }
  
   //========Alarm 15 liters
    if (totalMilliLitres > quantityAlarm*3){
       
       digitalWrite( beeper, HIGH);
       
    } 
    
    if (totalMilliLitres > quantityAlarm*3+5000){
       digitalWrite( beeper, LOW);
      
    }
  
      
      
      
    unsigned int frac;
    
    // Print the flow rate for this second in litres / minute
    Serial.print("Flow rate: ");
    Serial.print(int(flowRate));  // Print the integer part of the variable
    Serial.print(".");             // Print the decimal point
    // Determine the fractional part. The 10 multiplier gives us 1 decimal place.
    frac = (flowRate - int(flowRate)) * 10;
    Serial.print(frac, DEC) ;      // Print the fractional part of the variable
    Serial.print("L/min");
    // Print the number of litres flowed in this second
    Serial.print("  Current Liquid Flowing: ");             // Output separator
    Serial.print(flowMilliLitres);
    Serial.print("mL/Sec");

    // Print the cumulative total of litres flowed since starting
    Serial.print("  Output Liquid Quantity: ");             // Output separator
    Serial.print(totalMilliLitres);
    Serial.println("mL"); 

    // Reset the pulse counter so we can start incrementing again
    pulseCount = 0;
    
    
    // Enable the interrupt again now that we've finished sending output
    attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
  }
}

/*
Insterrupt Service Routine
 */
void pulseCounter()
{
  // Increment the pulse counter
  pulseCount++;
}
