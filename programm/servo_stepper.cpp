/*
 * servo_stepper.cpp
 *
 *  Created on: Dec 2, 2017
 *      Author: hans
 */

#include "stepper_motor.h"

#include "servo_stepper.h"

servo_stepper::servo_stepper(const int *pins) {
	motor = new stepper_motor(pins);
	step = 0;
	//create routine to center the laser
}

servo_stepper::~servo_stepper(void) {
	set_step(0, 1);
	delete motor;
}

void servo_stepper::set_step(int step, int millis) {
	int turn = this->step - step;
	if(turn < 0) {
		motor->clockwise(-turn, millis);
	} else {
		motor->counterclockwise(turn, millis);
	}
	this->step = step;
}

int servo_stepper::get_step(void) {
	return step;
}
