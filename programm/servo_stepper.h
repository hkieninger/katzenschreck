/*
 * servo_stepper.h
 *
 *  Created on: Dec 2, 2017
 *      Author: hans
 */

#ifndef SERVO_STEPPER_H_
#define SERVO_STEPPER_H_

#include "stepper_motor.h"

/* the class permits to use a stepper motor as a kind of servo motor */
class servo_stepper {
	public:
		/* constructor initialises the internal stepper motor, wiringPiSetup should be called before
		 * @pins: the pins of the stepper motor */
		servo_stepper(const int *pins);

		/* destructor turns the pins of and sets the motor to the initial state */
		~servo_stepper(void);

		/* set the step of the motor
		 * @step: the step (angle) to set
		 * @millis: the delay of the motor */
		void set_step(int step, int millis);

		/* returns the current step (angle)
		 * @return: the current step */
		int get_step(void);
	private:
		//internal stepper motor object
		stepper_motor *motor;
		//the current step
		int step;
};

#endif /* SERVO_STEPPER_H_ */
