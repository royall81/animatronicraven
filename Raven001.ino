#include <ESP32Servo.h>

Servo servoHeadHor;  // create servo object to control a servo
Servo servoHeadVer;  // create servo object to control a servo
Servo servoHeadBeak;  // create servo object to control a servo
Servo servoWings;  // create servo object to control a servo
Servo servoLegs;  // create servo object to control a servo

float posHeadHor;    // variable to store the servo position
float posHeadVer;    // variable to store the servo position
float posHeadBeak;    // variable to store the servo position
float posWings;    // variable to store the servo position
float posLegs;    // variable to store the servo position

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
	int distance; // between 0 and 50
	float achieved;
	boolean active;
};

struct motorInstructions action[5] = {
	{9000, "headHor", 1000, 1500, false},
	{10000, "headVer", 1000, 1500, false},
	{12000, "headHor", 2000, 0, false},
	{12500, "headVer", 2000, 0, false},
	//{15000, "beak", 30, 0, false},
	// {20000, "wings", 30, 0, false},
	// {25000, "legs", 30, 0, false}
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

	// Attach the servoObject to the pin of the board
	servoHeadHor.attach(servoPinHeadHor, 500, 2400);
	servoHeadVer.attach(servoPinHeadVer, 500, 2400);
	// using default min/max of 1000us and 2000us
	// different servos may require different min/max settings
	// for an accurate 0 to 180 sweep
	servoHeadHor.writeMicroseconds(1500);
	servoHeadVer.writeMicroseconds(1500);

	// Serial.begin(9600);
	// Serial.println("Hello");
}

void loop() {
	// Here i have to create a loop to move the servos one by one.
	// And add the end it will look like if the servos all can move at the same time

	for (int i = 0; i < sizeof(action)/sizeof(action[0]); ++i) {
		// if time is same as starttime or action is still active
		if (seqCounter == action[i].startTime || action[i].active == true) {
			// Serial.print(action[i].startTime);
			// Serial.print(" , ");
			// Serial.print(action[i].servoName);
			// Serial.print(" , ");
			// Serial.print(action[i].distance);
			// Serial.print(" , ");
			// Serial.print(action[i].achieved);
			// Serial.print(" , ");
			// Serial.println(action[i].active);
			smoothMove(i);
		}
	}

	// run loop 100 times a second
	delay(20);
	seqCounter = seqCounter + 20;

	// for (pos; pos <= 100; pos += 1) { // goes from 0 degrees to 180 degrees
	// in steps of 1 degree
	// myservo.write(pos);    // tell servo to go to position in variable 'pos'
	// delay(15);             // waits 15ms for the servo to reach the position
	// Serial.println(pos);
	// }
	//for (pos = 180; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
	//myservo.write(pos);    // tell servo to go to position in variable 'pos'
	//delay(15);             // waits 15ms for the servo to reach the position
	//}
}

void smoothMove(int actionId) {
	// servo destination
	int move = action[actionId].distance;
	// Get latest servoPosition
	if (action[actionId].servoName == "headHor") lastPosition = posHeadHor;
	if (action[actionId].servoName == "headVer") lastPosition = posHeadVer;
	if (action[actionId].servoName == "beak") lastPosition = posHeadBeak;
	if (action[actionId].servoName == "wings") lastPosition = posWings;
	if (action[actionId].servoName == "legs") lastPosition = posLegs;
	if (lastPosition != 0 && action[actionId].active == false) {
		action[actionId].achieved = lastPosition;
	}
	float movePrev = action[actionId].achieved;
	moveSmooth = (move * 0.05) + (movePrev * 0.95);

	if (action[actionId].servoName == "headHor") servoHeadHor.writeMicroseconds(moveSmooth);
	if (action[actionId].servoName == "headVer") servoHeadVer.writeMicroseconds(moveSmooth);
	if (action[actionId].servoName == "beak") servoHeadBeak.writeMicroseconds(moveSmooth);
	if (action[actionId].servoName == "wings") servoWings.writeMicroseconds(moveSmooth);
	if (action[actionId].servoName == "legs") servoLegs.writeMicroseconds(moveSmooth);

	// Serial.print(seqCounter);
	// Serial.print(" , ");
	// Serial.print(action[actionId].servoName);
	// Serial.print(" , ");
	// Serial.print(moveSmooth);
	// Serial.print(" , ");
	// Serial.print(move);
	// Serial.print(" , ");
	// Serial.println(movePrev);

	// Set the move already has achieved
	action[actionId].achieved = moveSmooth;

	// If the move is completed
	if (almostEqual(move, moveSmooth)) {
		action[actionId].active = false;
		if (action[actionId].servoName == "headHor") posHeadHor = action[actionId].distance;
		if (action[actionId].servoName == "headVer") posHeadVer = action[actionId].distance;
		if (action[actionId].servoName == "beak") posHeadBeak = action[actionId].distance;
		if (action[actionId].servoName == "wings") posWings = action[actionId].distance;
		if (action[actionId].servoName == "legs") posLegs = action[actionId].distance;
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
