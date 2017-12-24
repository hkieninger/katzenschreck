/*
 * move.cpp
 *
 *  Created on: Nov 27, 2017
 *      Author: hans
 */

#include <math.h>

#include "stepper_motor.h"
#include "servo_stepper.h"
#include "object.h"
#include "move.h"

extern void centerObject(struct object *obj, stepper_motor *motor, servo_stepper *servo) {
	centerObjectHorizontal(obj, motor);
	centerObjectVertical(obj, servo);
}

extern void centerObjectVertical(struct object *obj, servo_stepper *servo) {
	double angle = atan(1.0 * obj->y / obj->size);
	servo->set_step(stepper_motor::radian2step(angle), 1);
}

extern void centerObjectHorizontal(struct object *obj, stepper_motor *motor) {
	double angle = atan(1.0 * obj->x / obj->size);
	if(angle > 0) {
		motor->counterclockwise(stepper_motor::radian2step(angle), 1);
	} else {
		motor->clockwise(stepper_motor::radian2step(-angle), 1);
	}
}

