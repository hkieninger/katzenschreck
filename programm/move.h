/*
 * move.h
 *
 *  Created on: Nov 27, 2017
 *      Author: hans
 */

#ifndef MOVE_H_
#define MOVE_H_

#include "servo_stepper.h"
#include "stepper_motor.h"
#include "object.h"

void centerObject(struct object *obj, stepper_motor *motor, servo_stepper *servo);
void centerObjectVertical(struct object *obj, servo_stepper *servo);
void centerObjectHorizontal(struct object *obj, stepper_motor *motor);

#endif /* MOVE_H_ */
