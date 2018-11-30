//Joseph Schroedl FRC Team 2059

//A delay of 1000 Microseconds is Full Reverse
//A delay of 1000 to 1460 Microseconds is Proportional Reverse
//A delay of 1460 to 1540 Microseconds is neutral
//A delay of 1540 to 2000 Microseconds is Proportional Forward
//A delay of 2000 Microseconds is Full Forward

//For Xbox controller with USB shield
#include <Servo.h>
#include <XBOXRECV.h>

//Other Includes for USB shield
#ifdef dobogusinclude
#include <spi4teensy3.h>
#include <SPI.h>
#endif

/*****User Configurable Variables*****/

//Motor Controller Deadzones. This zone keeps the motor controllers from fluctuation from off to +-1% speed.
const int lowNeutral = 1460;
const int highNeutral = 1540;

const int motorNeutral = 1500;

//This is deadzone threshold on the joystick because the resting position of the joystick varries. Making this value bigger will reqire the user to move the joystick further before the code starts using the joystick values
const int joystickDeadzone = 7500;

const int backLeftPin = 28;  //Back Left Motor pin
int backLeftSpeed = 1500; //Back Left Motor starting speed
Servo backLeftMotor;  //Create the backLeftMotor object for the servo library

const int frontLeftPin = 43;  //Front Left Motor pin
int frontLeftSpeed = 1500; //Front Left Motor starting speed
Servo frontLeftMotor; //Create the frontLeftMotor object for the servo library

const int backRightPin = 32;  //Back Right Motor pin
int backRightSpeed = 1500; //Back Right Motor starting speed
Servo backRightMotor; //Create the backRightMotor object for the servo library

const int frontRightPin = 38; //Front Right Motor pin
int frontRightSpeed = 1500; //Front Right Motor starting speed
Servo frontRightMotor; //Create the frontRightMotor object for the servo library

const int shooterPin = 42;  //Top Roller Motor pin
//1460 for no running motors, no buttons held.
//1600 for low power, "X" button held.
//1800 for medium power, "A" button held.
//2000 for high power, "B" button held.
int shooterSpeed = 1500;  //Top Roller Motor starting speed
Servo shooterMotor; //Create the topRollerMotor object for the servo library

//Non-PWM Outputs
const int pistonExtendRelayPin = 22;

const int pistonRetractRelayPin = 26;

const int compressorRelayPin = 36;

const int airPressureSensorPin = 24;

const int redLEDPin = 44;

const int greenLEDPin = 45;

const int blueLEDPin = 46;

//Use the Xbox controller number 0, I am only using one controller
const int controlNum = 0;

const int filter = 15;

/*****Non-Configurable Variables*****/

short joyX = 0;             //joyX < 0 = Left, joyX > 0 = Right
short joyY = 0;             //joyY > 0 = Forward, joyY < 0 = Reverse

short joyYFiltered = 0;
short joyYFilteredPrev = 0;

//Joystick turret value
short turretJoyX = 0;

//Variable to keep track of whether we are driving in any direction
bool driving = false;

//Variable to keep track of whether we are shooting
bool shooting = false;

//Variable to keep track of the compressor status
bool compressor = false;

//Variable to keep track of the air pressure sensor status
bool airPressureSensor = true;

//Initialization for USB shield
USB Usb;
XBOXRECV Xbox(&Usb);

void setup()
{
  //Initialize USB shield
  Usb.Init();
  //Wait for initialization before continueing
  delay(1000);

  //Setup for the motor controller outputs pins
  //Drive train
  backLeftMotor.attach(backLeftPin);
  frontLeftMotor.attach(frontLeftPin);
  backRightMotor.attach(backRightPin);
  frontRightMotor.attach(frontRightPin);
  //Other motors
  shooterMotor.attach(shooterPin);

  //Other Input and Outputs
  pinMode(compressorRelayPin, OUTPUT);
  pinMode(airPressureSensorPin, INPUT_PULLUP);
  pinMode(pistonExtendRelayPin, OUTPUT);
  pinMode(pistonRetractRelayPin, OUTPUT);

  pinMode(redLEDPin, OUTPUT);
  pinMode(greenLEDPin, OUTPUT);
  pinMode(blueLEDPin, OUTPUT);

  Serial.begin(115200);

  //Wait for USB and other setup to finish. This delay is probably not necessary.
  delay(1000);
}

void stopMotors()
{
  //Drive train
  backLeftMotor.writeMicroseconds(motorNeutral);
  frontLeftMotor.writeMicroseconds(motorNeutral);
  backRightMotor.writeMicroseconds(motorNeutral);
  frontRightMotor.writeMicroseconds(motorNeutral);
  //Other Motors
  shooterMotor.writeMicroseconds(motorNeutral);
}

void compressorRun() {
  //Code here will run regardless of the xbox controller connection.
  //Read the air pressure sensor to check the PSI of the pnumatics
  airPressureSensor = digitalRead(airPressureSensorPin);
  if (airPressureSensor <= 0) {
    digitalWrite(compressorRelayPin, HIGH);
  }
  else if (airPressureSensor >= 1) {
    digitalWrite(compressorRelayPin, LOW);
  }
  //Serial.print("airPressureSensor: ");
  //Serial.println(airPressureSensor);
}

void loop()
{
  compressorRun();

  Usb.Task();
  if (Xbox.XboxReceiverConnected) {
    //This if statement makes sure that the motors will run only when the controller is connected. The motors will stop running when the controller is disconnected. This is the only way to disable the system other than cutting power.
    if (Xbox.Xbox360Connected[controlNum]) {

      //We start by saying we are not shooting, driving, or collecting
      if (joyYFiltered < 1500 || joyYFiltered > 1500) {
        driving = true;
      }
      else {
        driving = false;
      }
      shooting = false;

      //We have to use the "Val" to seperate it from LeftHatX which is a different varible in the library
      //The "LeftHat" is the left joystick
      //"Pre" = Joystick value before maping the values
      int LeftHatXValPre = 0;
      int LeftHatYValPre = 0;

      //Grabs the xbox controller left analog stick data once, done here once to save cpu cycles
      int xboxLeftHatXData = Xbox.getAnalogHat(LeftHatX, controlNum);
      int xboxLeftHatYData = Xbox.getAnalogHat(LeftHatY, controlNum);

      //This is deadzone detection on the joystick because the resting position of the joystick varries
      if (xboxLeftHatXData > joystickDeadzone || xboxLeftHatXData < -joystickDeadzone) {
        LeftHatXValPre = xboxLeftHatXData;
        driving = true;
      }
      if (xboxLeftHatYData > joystickDeadzone || xboxLeftHatYData < -joystickDeadzone) {
        LeftHatYValPre = xboxLeftHatYData;
        driving = true;
      }

      //Serial.print("xboxLeftHatYData: ");
      //Serial.println(xboxLeftHatYData);

      //This section detects which buttons on the Xbox controller are being held and sets the shooter speed (shooterSpeed) based on which button pressed.
      //1460 for no running motors, no buttons held.
      //1600 for low power, "X" button held.
      //1800 for medium power, "A" button held.
      //2000 for high power, "B" button held.
      //topRollerSpeed and bottomRollerSpeed are set opposite because the shooter needs to spin rollers the opposite direction to pull the ball out
      if (Xbox.getButtonPress(A, controlNum)) {
        shooting = true;
      }
      else {
        shooting = false;
      }

      if ((Xbox.getButtonPress(R1, controlNum) && Xbox.getButtonPress(L1, controlNum)) || (Xbox.getButtonPress(BACK, controlNum))) {
        shooterSpeed = 1500;
      }
      else if (Xbox.getButtonPress(R2, controlNum) > 30) {
        shooterSpeed = map(Xbox.getButtonPress(R2, controlNum), 0, 255, 1500, 2000);
      }
      else if (Xbox.getButtonClick(R1, controlNum)) {
        if (shooterSpeed < 2000) {
          shooterSpeed += 100;
        }
        else if (shooterSpeed >= 2000) {
          shooterSpeed = 2000;
        }
      }
      else if (Xbox.getButtonClick(L1, controlNum)) {
        if (shooterSpeed > 1500) {
          shooterSpeed -= 100;
        }
        else if (shooterSpeed <= 1500) {
          shooterSpeed = 1500;
        }
      }

      shooterMotor.writeMicroseconds(shooterSpeed);

      //Serial.print("shooterSpeed: ");
      //Serial.println(shooterSpeed);

      //Convert the joystick values
      joyY = map(LeftHatYValPre, -32768, 32768, 1000, 2000);
      joyX = map(LeftHatXValPre, -32768, 32768, -150, 150);

      joyYFiltered = (joyY + (joyYFilteredPrev * filter)) / (filter + 1);
      joyYFilteredPrev = joyYFiltered;

      //Serial.print(joyY);
      //Serial.print(" ");
      //Serial.println(joyYFiltered);


      //Serial.print("joyY: ");
      //Serial.println(joyY);

      if (driving) {
        bool drivingForward = joyY > 1650;  //Are we driving?
        bool drivingReverse = joyY < 1300;

        if (joyY <= lowNeutral || joyY >= highNeutral) {
          backLeftSpeed = frontLeftSpeed = backRightSpeed = frontRightSpeed = joyYFiltered;   //Sets the speed for all motors based on the Forward/Reverse of the joystick
        }

        int absJoyX = abs(joyX);
        if (absJoyX > 15) {    //Am I moving the joystick left or right?
          if (joyX < 0 && (!drivingForward && !drivingReverse)) {     //Zero point turn Left
            backRightSpeed = highNeutral + absJoyX;   //highNeutral for forwards movement
            frontRightSpeed = highNeutral + absJoyX;  //lowNeutral for backwords movement
            backLeftSpeed = lowNeutral + joyX;
            frontLeftSpeed = lowNeutral + joyX;
          }
          else if (joyX > 0 && (!drivingForward && !drivingReverse)) {      //Zero point turn Right
            backRightSpeed = lowNeutral - joyX;
            frontRightSpeed = lowNeutral - joyX;
            backLeftSpeed = highNeutral + joyX;
            frontLeftSpeed = highNeutral + joyX;
          }
          else if (joyX < 0) {       //Turning Left
            backRightSpeed += drivingForward ? abs(joyX) : joyX;
            frontRightSpeed += drivingForward ? abs(joyX) : joyX;
          }
          else if (joyX > 0) {       //Turning Right
            backLeftSpeed += drivingForward ? joyX : -joyX;
            frontLeftSpeed += drivingForward ? joyX : -joyX;
          }
        }
      }

      if (driving) {
        //If we moved the joystick to drive then set the motors to the speed determined by the joystick

        backLeftMotor.writeMicroseconds(backLeftSpeed);
        frontLeftMotor.writeMicroseconds(frontLeftSpeed);
        backRightMotor.writeMicroseconds(backRightSpeed);
        frontRightMotor.writeMicroseconds(frontRightSpeed);
      }
      else {
        //Else set the motors to neutral speed so they don't run
        backLeftMotor.writeMicroseconds(motorNeutral);
        frontLeftMotor.writeMicroseconds(motorNeutral);
        backRightMotor.writeMicroseconds(motorNeutral);
        frontRightMotor.writeMicroseconds(motorNeutral);

        //I use the detach command from the Servo library so the motors don't abruptly stop. This keeps taller robots from falling forward or backward when stoping.
        /*backLeftMotor.detach();
          frontLeftMotor.detach();
          backRightMotor.detach();
          frontRightMotor.detach();*/
      }

      //Serial.println(shooting);
      if (shooting) {
        //If we pressed "A" button then call the shooter piston code
        //shooterPiston();

        digitalWrite(pistonExtendRelayPin, HIGH);
        delay(10);
        digitalWrite(pistonExtendRelayPin, LOW);
      }
      else {
        digitalWrite(pistonRetractRelayPin, HIGH);
        delay(10);
        digitalWrite(pistonRetractRelayPin, LOW);
      }

    }
    else {
      //******************The code after this is for safety.******************
      //If the Xbox controller is not connected set the motors to neutral. The motors will not spin without the Xbox controller connected.
      stopMotors();
    }
  }
  else {
    //If the Xbox controller is not connected set the motors to neutral. The motors will not spin without the Xbox controller connected.
    stopMotors();
  }
}
