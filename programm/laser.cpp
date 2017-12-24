/*
 * laser.cpp
 *
 *  Created on: Nov 23, 2017
 *      Author: hans
 */

#include <wiringPi.h>

#include "laser.h"

laser::laser(int pin) {
	pinMode(pin, OUTPUT);
	digitalWrite(pin, LOW);
	on = false;
	this->pin = pin;
}

laser::~laser(void) {
	digitalWrite(pin, LOW);
}

void laser::turnOn(void) {
	digitalWrite(pin, HIGH);
	on = true;
}

void laser::turnOff(void) {
	digitalWrite(pin, LOW);
	on = false;
}

bool laser::isOn(void) {
	return on;
}
