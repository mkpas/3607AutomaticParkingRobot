#include <Servo.h>            // Include servo library
#include <SoftwareSerial.h>   //Software Serial Port


#define RxD 7
#define TxD 6

SoftwareSerial blueToothSerial(RxD, TxD);

Servo left;                             // Declare left servo signal
Servo right;                            // Declare right servo signal
const int speedRight = 1450;            // Range: 1500(Stop)~1000(Full speed)
const int speedLeft = 1550;             // Range: 1500(Stop)~2000(Full speed)
const int revRight = 1550;              // Range: 1500(Stop)~2000(Full speed)
const int revLeft = 1450;               // Range: 1500(Stop)~1000(Full speed)
const int servoStop = 1500;
const int rightBound = 600;
const int leftBound = 700;
const int rightWall = 600;
const int leftWall = 600;

int sensor1 = A0; //connected to analog 0 (PHOTOTRANSISTOR READING)
int sensor2 = A2; //connected to analog 2 (PHOTOTRANSISTOR READING)
int sensor3 = A3; //connected to analog 3 (PHOTOTRANSISTOR READING)
int sensor4 = A4; //connected to analog 4 (PHOTOTRANSISTOR READING)
int ledPin = 9;    // LED connected to digital pin 9 (IR LED PIN)

boolean autopilot = false;
char recvChar;

void setup() {
  Serial.begin(9600);
  pinMode(RxD, INPUT);
  pinMode(TxD, OUTPUT);
  left.attach(13);
  right.attach(12);
  setupBlueToothConnection();
  delay(1000);
  Serial.flush();
}

void loop() {
  while (1) {
    if (blueToothSerial.available()) {       //check if there's any data sent from the remote bluetooth shield
      recvChar = blueToothSerial.read();
      Serial.print(recvChar);

      switch (recvChar) {
        case 'w':
          goForward();
          blueToothSerial.println("Moving forward.");
          break;
        case 'a':
          goLeft();
          blueToothSerial.println("Moving left.");
          break;
        case 's':
          reverse();
          blueToothSerial.println("Reversing.");
          break;
        case 'd':
          goRight();
          blueToothSerial.println("Moving right.");
          break;
        case 'q':
          turnOnSpotLeft();
          blueToothSerial.println("Turning on spot left.");
          break;
        case 'e':
          turnOnSpotRight();
          blueToothSerial.println("Turning on spot right.");
          break;
        case ' ':
          stopAll();
          blueToothSerial.println("Stopped.");
          break;
        case 'z':
          reverseLeft();
          blueToothSerial.println("Reversing left.");
          break;
        case 'c':
          reverseRight();
          blueToothSerial.println("Reversing right.");
          break;
        case 'p':
          autopilot = true;
          Serial.println("AUTO");
          blueToothSerial.println("Autopilot enabled");
          autopilotFn();
          break;
      }
    }
  }
}

void autopilotFn() {
  
  boolean parkRight = false;
  boolean parkLeft = false;
  boolean parked = false;

  while (autopilot == true) {
    analogWrite(ledPin, 255);
    int sensorRight = analogRead(sensor2);
    int sensorLeft = analogRead(sensor3);
    int sensorRightFork = analogRead(sensor1);
    int sensorLeftFork = analogRead(sensor4);
    Serial.print("sensor1: ");
    Serial.print(sensorRight);
    Serial.print("   sensor2: ");
    Serial.print(sensorLeft);
    Serial.print("   sensor3: ");
    Serial.print(sensorRightFork);
    Serial.print("   sensor4: ");
    Serial.println(sensorLeftFork);

    //==== LINE FOLLOWING ALGORITHM ====
    if (sensorLeft < leftBound && sensorRight < rightBound) { // i.e. Front 2 sensors sense white
      goForward();
    }
    
    if (sensorLeft > leftBound && sensorRight < rightBound) { // i.e. Front Left sensor senses Black, Front Right senses White
      goLeft();
      Serial.println("GOING LEFT");
    }
    
    if (sensorLeft < leftBound && sensorRight > rightBound) { // i.e. Front Left sensor senses White, Front Right senses Black
      goRight();
      Serial.println("GOING RIGHT");
    }

    //==== FULLY PARKED ====
    if(sensorLeft > leftBound && sensorRight > rightBound && parked == true) {
      stopAll(); 
    }

    //==== RIGHT PARKING ALGORITHM ====
    if (sensorRightFork < rightWall && parked == false) {
      parkRight = true;
      Serial.println("RIGHT WALL DETECTED");
    }

    if ((sensorRightFork > rightWall) && (parkRight == true)) {
      stopAll();
      blueToothSerial.println("Park Right y/n?");

      while(1) {
        if (blueToothSerial.available()) { //check if there's any data sent from the remote bluetooth shield
          recvChar = blueToothSerial.read();
          Serial.print(recvChar);
          if (recvChar == 'y') {
            stopAll();
            delay(500);
            goForward();
            delay(1300);
            stopAll();
            delay(500);
            turnOnSpotRight();
            delay(850);
            stopAll();
            delay(500);
            parkRight = false;
            parked = true;
            break;
          } if (recvChar == 'n') {
            parkRight = false;
            parked = false;
            break;
          }
        }
      }
    }

    //==== LEFT PARKING ALGORITHM ====
    if (sensorLeftFork < leftWall && parked == false) {
      parkLeft = true;
      Serial.println("LEFT WALL DETECTED");
    }

    if ((sensorLeftFork > leftWall) && (parkLeft == true)) {
      stopAll();
      blueToothSerial.println("Park Left y/n?");

      while(1) {
        if (blueToothSerial.available()) { //check if there's any data sent from the remote bluetooth shield
          recvChar = blueToothSerial.read();
          Serial.print(recvChar);
          
          if (recvChar == 'y') {
            stopAll();
            delay(500);
            goForward();
            delay(1300);
            stopAll();
            delay(500);
            turnOnSpotLeft();
            delay(850);
            stopAll();
            delay(500);
            parkLeft = false;
            parked = true;
            break;
          } if (recvChar == 'n') {
            parkLeft = false;
            parked = false;
            break;
          }
        }
      }
    }

    //==== STILL ON AUTOPILOT? ====
    if (blueToothSerial.available()) {       //check if there's any data sent from the remote bluetooth shield
      recvChar = blueToothSerial.read();
      Serial.print(recvChar);
      if (recvChar == 'p') {
        autopilot = false;
        Serial.println("AUTO OFF");
        blueToothSerial.println("Autopilot disabled, resuming manual navigation.");
      }
    }
    
  }
}

void goForward() {
  right.writeMicroseconds(speedRight);
  left.writeMicroseconds(speedLeft);
}

void stopAll() {
  right.writeMicroseconds(servoStop);
  left.writeMicroseconds(servoStop);
}

void goLeft() {
  left.writeMicroseconds(servoStop);
  right.writeMicroseconds(speedRight);
}

void goRight() {
  right.writeMicroseconds(servoStop);
  left.writeMicroseconds(speedLeft);
}

void reverse() {
  right.writeMicroseconds(speedLeft);
  left.writeMicroseconds(speedRight);
}

void reverseLeft() {
  left.writeMicroseconds(servoStop);
  right.writeMicroseconds(revRight);
}

void reverseRight() {
  right.writeMicroseconds(servoStop);
  left.writeMicroseconds(revLeft);
}

void turnOnSpotLeft() {
  right.writeMicroseconds(speedRight);
  left.writeMicroseconds(revLeft);
}

void turnOnSpotRight() {
  right.writeMicroseconds(revRight);
  left.writeMicroseconds(speedLeft);
}

void setupBlueToothConnection()
{
  blueToothSerial.begin(38400);                           // Set BluetoothBee BaudRate to default baud rate 38500
  blueToothSerial.print("\r\n+STWMOD=0\r\n");             // set the bluetooth work in slave mode
  blueToothSerial.print("\r\n+STNA=SlaveUnique\r\n");    // set the bluetooth name as "SeeedBTSlave"
  blueToothSerial.print("\r\n+STOAUT=1\r\n");             // Permit Paired device to connect me
  blueToothSerial.print("\r\n+STAUTO=0\r\n");             // Auto-connection should be forbidden here
  delay(2000);                                            // This delay is required.
  while (blueToothSerial.available()) {
    Serial.write( blueToothSerial.read());
  }
  blueToothSerial.print("\r\n+INQ=1\r\n");                // make the slave bluetooth inquirable
  Serial.println("The slave bluetooth is inquirable!");
  blueToothSerial.flush();
  delay(2000);                                            // This delay is required.
  while (blueToothSerial.available()) {
    Serial.write( blueToothSerial.read());
  }
}

