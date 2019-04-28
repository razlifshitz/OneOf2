#ifdef VAR_SERVO_SPEED
  #define VAR_SERVO_SPEED 1
  #include <VarSpeedServo.h>
#endif

#include "paramsGenerator.h"
#include "myService.h"

Encoder encoder(ENCODER1_PIN, ENCODER2_PIN);
VarSpeedServo myservo;

//////////////////////////////
//////////CUP MODE////////////
////////////////////////////// 
bool isCupMod = false;

// -------------------------------------------- Properties
/* 
 * DELAY DATA (The numbers are in Miliseconds)
*/

// LENGTH
int minChangeInDelay = -1;
// Delay range
int maxDelay = -1;
int minDelay = -1;

// NUMBER OF MOVES (before each delay)
int minChangeInNumOfMoves =  -1; 
// count range
int maxNumOfCount = -1;
int minNumOfCount = -1;

long servoActiveDelay = -1;
long firstTime;
bool inDelayProcess = false;
int plateCounter = 0;  // Number of moves for the entire plate
int movesCounter = -1; // Number of moves in the current wave 
bool toMoveUp = true;
int lastServoLoc;
int servoStartLocation; //todo: implementation is needed



// -------------------------------------------- Logic Variables

// Delay length
int randDelay = -1;
int currentDelay = -1;

// Delay count of moves
int randCountOfMoves = -1;
int currentCountOfMoves = -1;

// Villroy and boch params
int currentQuarter = 0;
long encoderLocation;
long currentQurterActiveServo;
int nextServoLocation;
bool isEncoderReachedDestination;
bool isServoReachedDestination;
int waveSpeed;
bool beforeStart;
//-------------------------------------- END DATA SETTING


void servo_start() {
	Serial.println("servo_start()");
	myservo.attach(SERVO_PIN);  // attaches the servo on pin 9 to the servo object
}

void servo_stop() {
	Serial.println("servo_stop()");
	myservo.detach();
}

int getNextServoDestination(bool toMoveUp) {
	// Bottom range
	int minFrom = 5;
	int maxFrom = 20;

	// Upper range
	int minTo = 40;
	int maxTo = 60;

	return toMoveUp ? CalcRand(minTo,maxTo) : CalcRand(minFrom,maxFrom);
}

int getNextServoSpeed() {
	int numOfSpeedCategories = 5;
	WaveSpeed waveSpeeds[numOfSpeedCategories];
	waveSpeeds[1].initData(1, 10, 40);
	waveSpeeds[2].initData(2, 40, 50);
	waveSpeeds[3].initData(3, 50, 70);
	waveSpeeds[4].initData(4, 71, 100);
	waveSpeeds[5].initData(5, 101, 130);

	return calcNextSpeed(waveSpeeds, numOfSpeedCategories);
}

void calcNextLengthOfServoAction () {
  currentQurterActiveServo = encoderLocation + CalcRand(QUARTER_CLICKS_PER_ROUND_25, QUARTER_CLICKS_PER_ROUND_75);
}

bool servo_update() {
  #ifdef DEBUG_SERVO_MOVE_COUNTER 
    if (myservo.read() != lastServoLoc) {
      Serial.println(String("Servo Location: ") + lastServoLoc);
    }
  #endif

  lastServoLoc = myservo.read();
  encoderLocation = abs(encoder.read());

  // As Long as Encoder hasn't reached to destination where servo should delay, servo will work.
  if (!isEncoderReachedDestination) {
		beforeStart = false;
    isEncoderReachedDestination = motor_reachDestination(&encoder, currentQurterActiveServo);

    // checking if servo reached destination.
    // doing nothing if servo hasn't reached destination
    if (!isServoReachedDestination) {
      isServoReachedDestination = hasServoReachedDestination(lastServoLoc, nextServoLocation, toMoveUp);
	  
	    // continuing the movement to the current destination
	    myservo.write(nextServoLocation, waveSpeed, false);
    }
    
    // servo reached destination, calc next servo move
    else {
			isServoReachedDestination = false;
      // calc next servo move
      toMoveUp = !toMoveUp;      
      waveSpeed = getNextServoSpeed();
      nextServoLocation = getNextServoDestination(toMoveUp);

      // write to monitor
      plateCounter++;
      #ifdef DEBUG_SERVO_MOVE_COUNTER
        Serial.println(String("Starting move to location: ") + nextServoLocation);
      #endif
      
      // start new servo move
      myservo.write(nextServoLocation, waveSpeed, false);
    }
  } 
	// The Encoder has reached to destination where servo should delay, tell servo to go to low
	// location and delay.
	else if (!beforeStart && encoderLocation <= currentQuarter * QUARTER_CLICKS_PER_ROUND) {
		waveSpeed = CalcRand(30, 70);
		nextServoLocation = getNextServoDestination(false);
    isServoReachedDestination = hasServoReachedDestination(lastServoLoc, nextServoLocation, false);

    #ifdef DEBUG_SERVO_MOVE_COUNTER
     	Serial.println(String("Starting move to location: ") + nextServoLocation + String(" and Afterwards STARTING DELAY!"));
    #endif

		if (!isServoReachedDestination) {
			myservo.write(nextServoLocation, waveSpeed, false);
		}
  }
	// prepares next movement parameters
  else {
    currentQuarter++;

    if (currentQuarter <= 4) {
      // start next quarter
      isEncoderReachedDestination = false;
      calcNextLengthOfServoAction();
    }
  }
  
  return true;
}
