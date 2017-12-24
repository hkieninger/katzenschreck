/*
 * motor.cpp
 *
 *  Created on: Nov 23, 2017
 *      Author: hans
 */

#include <math.h>
#include <unistd.h>

#include <wiringPi.h>

#include "stepper_motor.h"

int stepper_motor::radian2step(double radian) {
	return round(radian * 64 * 64 / 2 / M_PI);
}

stepper_motor::stepper_motor(const int *pins) {
	step = 0;
	for(int i = 0; i < 4; i++) {
		pinMode(pins[i], OUTPUT);
		digitalWrite(pins[i], 0);
		this->pins[i] = pins[i];
	}
}

stepper_motor::~stepper_motor(void) {
	for(int i = 0; i < 4; i++) {
		digitalWrite(pins[i], 0);
	}
}

void stepper_motor::counterclockwise(int steps, int millis) {
	int bit;
	for(int i = 0; i < steps; i++) {
		step = step % 8;
		bit = 1;
		bit <<= step * 4;
		for(int j = 0; j < 4; j++) {
			digitalWrite(pins[j], sequence & bit << j);
		}
		step++;
		usleep(millis * 1000);
	}
}

void stepper_motor::clockwise(int steps, int millis) {
	int bit;
	for(int i = 0; i < steps; i++) {
		step = (step + 8) % 8;
		bit = 1;
		bit <<= step * 4;
		for(int j = 0; j < 4; j++) {
			digitalWrite(pins[j], sequence & bit << j);
		}
		step--;
		usleep(millis * 1000);
	}
}
