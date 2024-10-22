#include <ESP32Servo.h>

Servo servoHeadHor, servoHeadVer, servoHeadBeak, servoWings, servoLegs;  // create servo object to control a servo

float posHeadHor, posHeadVer, posHeadBeak, posWings, posLegs;    // variable to store the servo position

// Recommended PWM GPIO pins on the ESP32 include 2,4,12-19,21-23,25-27,32-33
int servoPinHeadHor = 13;
int servoPinHeadVer = 14;
int servoPinBeak = 15;
int servoPinWings = 16;
int servoPinLegs = 17;

int seqCounter;
int move;
float moveSmooth;
float movePrev;
float lastPosition;

struct motorInstructions {
	int startTime;
	String servoName;
	int newPosition; // between 0 and 50
	float oldPosition;
	boolean active;
};

struct motorInstructions action[23] = {
	{3000, "headHor", 1000, 1500, false},
	{6000, "headHor", 2000, 1000, false},
	{9000, "headVer", 1000, 1500, false},
	{9000, "wings", 1900, 1500, false},
	{12000, "wings", 1500, 1900, false},
	{12000, "headVer", 1500, 1000, false},
	{12000, "headHor", 1500, 2000, false},
	{21000, "headHor", 2000, 1500, false},
	{24000, "headVer", 2000, 1500, false},
	{27000, "headHor", 1500, 2000, false},
	{33000, "headHor", 2000, 1500, false},
	{33000, "wings", 1900, 1500, false},
	{36000, "wings", 1500, 1900, false},
	{39000, "headHor", 1500, 2000, false},
	{39000, "headVer", 1500, 2000, false},
	{42000, "headVer", 2000, 1500, false},
	{42000, "headHor", 1000, 1500, false},
	{45000, "headHor", 1500, 1000, false},
	{47000, "wings", 1900, 1500, false},
	{50000, "wings", 1500, 1900, false},
	{55000, "wings", 1900, 1500, false},
	{58000, "wings", 1500, 1900, false},
	{60000, "headVer", 1500, 1000, false},
};

void setup() {
	// Allow allocation of all timers
	ESP32PWM::allocateTimer(0);
	ESP32PWM::allocateTimer(1);
	ESP32PWM::allocateTimer(2);
	ESP32PWM::allocateTimer(3);

	// Set the hz of the servos
	servoHeadHor.setPeriodHertz(50);    // standard 50 hz servo
	servoHeadVer.setPeriodHertz(50);    // standard 50 hz servo
	servoHeadBeak.setPeriodHertz(50);    // standard 50 hz servo
	servoWings.setPeriodHertz(50);    // standard 50 hz servo
	servoLegs.setPeriodHertz(50);    // standard 50 hz servo

	// Attach the servoObject to the pin of the board
	servoHeadHor.attach(servoPinHeadHor, 500, 2500);
	servoHeadVer.attach(servoPinHeadVer, 500, 2500);
	servoHeadBeak.attach(servoPinBeak, 500, 2500);
	servoWings.attach(servoPinWings, 500, 2500);
	servoLegs.attach(servoPinLegs, 500, 2500);

	// Move all servos to a specific start position
	servoHeadHor.writeMicroseconds(1500);
	servoHeadVer.writeMicroseconds(1500);
	servoHeadBeak.writeMicroseconds(1500);
	servoWings.writeMicroseconds(1500);
	servoLegs.writeMicroseconds(1500);

	// Serial.begin(9600);
	// Serial.println("Hello");
}

void loop() {
	// Here i have to create a loop to move the servos one by one.
	// And add the end it will look like if the servos all can move at the same time

	for (int i = 0; i < sizeof(action)/sizeof(action[0]); ++i) {
		// if time is same as starttime or action is still active
		if (seqCounter == action[i].startTime || action[i].active == true) {
			smoothMove(i);
		}
	}

	// run loop 100 times a second
	delay(20);
	if (seqCounter == 65000) {
		seqCounter = 0; // set counter to zero, zo the loop will start again.
	} else {
		seqCounter = seqCounter + 20;
	}

}

void smoothMove(int actionId) {
	// servo destination
	int move = action[actionId].newPosition;
	// Get latest servoPosition
	if (action[actionId].servoName == "headHor") lastPosition = posHeadHor;
	if (action[actionId].servoName == "headVer") lastPosition = posHeadVer;
	if (action[actionId].servoName == "beak") lastPosition = posHeadBeak;
	if (action[actionId].servoName == "wings") lastPosition = posWings;
	if (action[actionId].servoName == "legs") lastPosition = posLegs;
	// If Lastposition not is 0 and move is not active set the oldPosition
	if (lastPosition != 0 && action[actionId].active == false) {
		action[actionId].oldPosition = lastPosition;
	}
	float movePrev = action[actionId].oldPosition;
	moveSmooth = (move * 0.05) + (movePrev * 0.95);

	if (action[actionId].servoName == "headHor") servoHeadHor.writeMicroseconds(moveSmooth);
	if (action[actionId].servoName == "headVer") servoHeadVer.writeMicroseconds(moveSmooth);
	if (action[actionId].servoName == "beak") servoHeadBeak.writeMicroseconds(moveSmooth);
	if (action[actionId].servoName == "wings") servoWings.writeMicroseconds(moveSmooth);
	if (action[actionId].servoName == "legs") servoLegs.writeMicroseconds(moveSmooth);

	// Set the move already has achieved
	action[actionId].oldPosition = moveSmooth;

	// If the move is completed
	if (almostEqual(move, moveSmooth)) {
		action[actionId].active = false;
		if (action[actionId].servoName == "headHor") posHeadHor = action[actionId].newPosition;
		if (action[actionId].servoName == "headVer") posHeadVer = action[actionId].newPosition;
		if (action[actionId].servoName == "beak") posHeadBeak = action[actionId].newPosition;
		if (action[actionId].servoName == "wings") posWings = action[actionId].newPosition;
		if (action[actionId].servoName == "legs") posLegs = action[actionId].newPosition;
	}
	else {
		action[actionId].active = true;
	}
	//End of smoothing

}

boolean almostEqual(float a, float b) {
	const float DELTA = 1; // max difference before equal.
	return fabs(a - b) <= DELTA;
}
