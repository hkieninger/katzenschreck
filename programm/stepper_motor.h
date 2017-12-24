/*
 * motor.h
 *
 *  Created on: Nov 23, 2017
 *      Author: hans
 */

#ifndef STEPPER_MOTOR_H_
#define STEPPER_MOTOR_H_ 1

#include <stdint.h>

/* the class represents a stepper motor of the type 28BYJ–48 with the drive uln2003 in combination with a raspberry*/
class stepper_motor {
	public:
		/* transforms radians to steps the motor have to do to turn the specified angle */
		static int radian2step(double radian);

		/* constructor initialises the pins of the raspberry used for the motor
		 * wiringPiSetup() must be called before
		 * @pins: an array containing the four pins used for the stepper motor in ascending order */
		stepper_motor(const int *pins);

		/* destructor of the stepper motor, sets the pins to LOW */
		~stepper_motor(void);

		/* let the stepper motor turn the specified amount of steps in a clockwise direction, 4096 steps are 360°
		 * @steps: the amount of steps
		 * @millis: the time to sleep in millis between the steps, should be at least 1 */
		void clockwise(int steps, int millis);

		/* let the stepper motor turn the specified amount of steps in a counter clockwise direction, 4096 steps are 360°
		 * @steps: the amount of steps
		 * @millis: the time to sleep in millis between the steps, should be at least 1 */
		void counterclockwise(int steps, int millis);

	private:
		/* integer indicating the step the motor is current at, a number between 0 and 7 */
		int step;

		/* integer array containing the pins in ascending order */
		int pins[4];

		/* sequence how to controll the motor */
		const uint32_t sequence = 0b1000 | 0b1100 << 4 | 0b0100 << 8 | 0b0110 << 12 | 0b0010 << 16 | 0b0011 << 20 | 0b0001 << 24 | 0b1001 << 28;

};

#endif /* STEPPER_MOTOR_H_ */
