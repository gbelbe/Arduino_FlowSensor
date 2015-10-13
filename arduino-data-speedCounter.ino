#include <Ethernet.h>
#include <SPI.h>
#include <EEPROM.h> 

////////////////////////////////////////////////////////////////////////
//CONFIGURE
////////////////////////////////////////////////////////////////////////

// Motor pin definitions
// ! Attention, there is a potential conflict between ethernet shield and step motor if using step 10 
// (as it is used for communication between arduino board and ethernet shield )
// it is preferred to use pins 6 to 9 to connect to the stepper motor

#define motorPin1  6     // IN1 on the ULN2003 driver 1
#define motorPin2  7     // IN2 on the ULN2003 driver 1
#define motorPin3  8     // IN3 on the ULN2003 driver 1
#define motorPin4  9   // IN4 on the ULN2003 driver 1

#define beeperPin  12


//Each time the motor is driven to a certain angle (0 to 180 degrees) we will write to the EEPROM this angle so that we can reset it whenever the arduino is unplugged
const int addr = 0;  // the current address in the EEPROM (i.e. which byte we're going to write to next)


//************************************************************************
// Motor config
//************************************************************************

const int motorSpeed = 1200;//1200;  //variable to set stepper speed

int stepsPerRev = 512; // number of steps per full revolution

int lookup[8] = {B01000, B01100, B00100, B00110, B00010, B00011, B00001, B01001};


//************************************************************************
// Ethernet  connection definition
//************************************************************************
byte server[] = { 104,131,116,128}; //ip Address of the server you will connect to
//byte server[] = { 192,168,0,105}; //ip Address of the server you will connect to


int port = 80; // server port
String location = "/index.php HTTP/1.0";

//byte server[] = { 192,168,0,14};      //ip Address of the server you will connect to
///byte server[] = "arduino.gaetonweb.com";
//byte gateway[] = {192,168,0,1};
//byte subnet[] = {255,255,255,0};

//int port = 8888;

// if need to change the MAC address (Very Rare)
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
//byte ip[] = { 192, 168, 0, 245 }; // case we want to use fixed IP

EthernetClient client;

char inString[32]; // string for incoming serial data
int stringPos = 0; // string index counter
boolean startRead = false; // is reading?

// variable used to reset the counter after desconnection
int lastPosition = 0; //Angle of the last position, we need to substract it with the new position to check if we need the motor to go forward or backwards

//////////////////////////////////////////////////////////////////////////////
// SETUP
//////////////////////////////////////////////////////////////////////////////

// setup the stepper parameters

void setup() {
 
  //declare the motor pins as outputs
  pinMode(motorPin1, OUTPUT);
  pinMode(motorPin2, OUTPUT);
  pinMode(motorPin3, OUTPUT);
  pinMode(motorPin4, OUTPUT);
  
  Serial.begin(9600);
 
  //int value = EEPROM.read(addr);
 
  //Serial.println("EEPROM Value");
  //Serial.println(value);
  // antiClockwise(value);
 
  Serial.println("starting communication");
 	
  //Connect the Ethernet Shield by specifying the board's mac and ip address
 
 	
  Ethernet.begin(mac);//, ip , gateway, subnet);
  
  Serial.println("Ethernet shield connected & obtained IP address : ");
  Serial.println(Ethernet.localIP());

 }
   		
 
//////////////////////////////////////////////////////////////////////////////
// MAIN LOOP
//////////////////////////////////////////////////////////////////////////////
         
  

void loop(){

       //Shield already connected during setup phase, now connect to the webpage and retrieve the value

       //Connect to the server
       //delay(1000);
       
       
         
       
 	       
 //Add a conditional to check if the arduino was able to read a value/.
 //If not, should reset the position to 0 to indicate a malfuntion (instead of being stuck on the last position)e
 
               String traf = connectAndRead();
               int traffic = traf.toInt();
               Serial.println(traf);
               //Read the output value written on the webpage
 
              //int traffic = readTrafic().toInt();
              Serial.println("traffic from webpage");
              Serial.println(traffic); 
      	
              int angle = trafficToAngle(traffic);
              Serial.println("Angle converted from traffic");
              Serial.println(angle); 
              Serial.println("lastPosition");
              Serial.println(lastPosition); 

              int diffAngle = angle - lastPosition;
              
              Serial.println("diffAngle");
              Serial.println(diffAngle); 

              
              lastPosition = angle;
            
              if(diffAngle >0){            
                clockwise(diffAngle);
              }else{
                antiClockwise(diffAngle);
              }
              
              //save to eeprom in case the Arduino disconnects we can reset the motor to the initial position by substracting this angle
      	      //EEPROM.put(addr, lastPosition);
              //int test = EEPROM.read(addr);
              //Serial.println("eeprom"); 
            
              delay(3000);

      //end of success case connection  
      }	



//////////////////////////////////////////////////////////////////////////////
// FUNCTIONS
//////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////////
//  0. Convert Analytics Value to an angle from 0 to 180
////////////////////////////////////////////////////////////////////////////////////////////

int trafficToAngle(int traffic)
{

  const int maxSpeed = 3000 ;
  // set the highest traffic of the speedCounter to 4000 realTime visitors 
  
  const int speedCounterAngle = 180;
  // assume a speed counter from 0 to 180 degrees. (almos one full circle)

  int angle = traffic / (maxSpeed / speedCounterAngle);
  return angle;
 
}




///////////////////////////////////////////////////////////////////////////////////////////
//  1. Connect to a web page 
////////////////////////////////////////////////////////////////////////////////////////////


String connectAndRead(){
  //connect to the server

  Serial.println("connecting...");

  //port 80 is typical of a www page
  if (client.connect(server, port)) {
    Serial.println("connected");
    client.print("GET ");
    client.println(location);
    client.println();

    //Connected - Read the page
    return readTrafic();//, readOrders(); //go and read the output

  }else{
    return "connection failed";
  }

}




///////////////////////////////////////////////////////////////////////////////////////////
//  2. Parse a value located between a < and a >
////////////////////////////////////////////////////////////////////////////////////////////



String readTrafic(){
  //read the page, and capture & return everything between '<' and '>'

  stringPos = 0;
  memset( &inString, 0, 32 ); //clear inString memory

  while(true){

    if (client.available()) {
      char c = client.read();

      if (c == '<' ) { //'<' is our begining character
        startRead = true; //Ready to start reading the part 
      }else if(startRead){

        if(c != '>'){ //'>' is our ending character
          inString[stringPos] = c;
          stringPos ++;
        }else{
          //got what we need here! We can disconnect now
          startRead = false;
          client.stop();
          client.flush();
          Serial.println("disconnecting.");
          return inString;

        }

      }
    }

  }

}



String readOrders(){
  //read the page, and capture & return everything between '[' and ']'

  stringPos = 0;
  memset( &inString, 0, 32 ); //clear inString memory

  while(true){

    if (client.available()) {
      char c = client.read();

      if (c == '[' ) { //'<' is our begining character
        startRead = true; //Ready to start reading the part 
      }else if(startRead){

        if(c != ']'){ //']' is our ending character
          inString[stringPos] = c;
          stringPos ++;
        }else{
          //got what we need here! We can disconnect now
          startRead = false;
          client.stop();
          client.flush();
          Serial.println("disconnecting.");
          return inString;

        }

      }
    }

  }

}



///////////////////////////////////////////////////////////////////////////////////////////
//   3. Convert the degrees into the closest number of steps relative to the step motor model.
////////////////////////////////////////////////////////////////////////////////////////////

int degreesToSteps(float angleDegrees) {
  //return the closest number of steps to accomplish to move the motor for the specified angle

  const float stepsPerDegree = stepsPerRev/360.00;
  int steps = angleDegrees * stepsPerDegree + 0.5;  // we add 0.5 as when converting to an int the floating part is truncated, this allows to round the value to the nearest Int
  return steps;
  
}



//////////////////////////////////////////////////////////////////////////////
//   4. Turn step motor to a certain angle (clockwise or anticlockwise)
//////////////////////////////////////////////////////////////////////////////




//set pins to ULN2003 high in sequence from 1 to 4
//delay "motorSpeed" between each pin setting (to determine speed)

void antiClockwise(float degrees)
{

  int steps = degreesToSteps(degrees);
  
  for(int i = 0; i<= steps; i++)
  { 
  
    for(int j = 0; j < 8; j++)
    {
    setOutput(j);
    delayMicroseconds(motorSpeed);
    }

  }
}

void clockwise(float degrees)
{
  
 int steps = degreesToSteps(degrees);
 for(int i = 0; i<= steps; i++)
  {   
    for(int j = 7; j >= 0; j--)
    {
      setOutput(j);
      delayMicroseconds(motorSpeed);
    }
  }
}


// send bits to motor driver

void setOutput(int out)
{
  digitalWrite(motorPin1, bitRead(lookup[out], 0));
  digitalWrite(motorPin2, bitRead(lookup[out], 1));
  digitalWrite(motorPin3, bitRead(lookup[out], 2));
  digitalWrite(motorPin4, bitRead(lookup[out], 3));
}

 

