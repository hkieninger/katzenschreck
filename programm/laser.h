/*
 * laser.h
 *
 *  Created on: Nov 23, 2017
 *      Author: hans
 */

#ifndef LASER_H_
#define LASER_H_

/* this class represents a laser connected to a raspberry */
class laser {
	public:
		/* constructor intialises the pin
		 * @pin: the pin where the laser is attached */
		laser(int pin);

		/* destructor sets the pin to low */
		~laser(void);

		/* truns the laser on */
		void turnOn(void);

		/* turns the laser off */
		void turnOff(void);

		/* returns the status of the laser
		 * @return: true if the laser is on and false if the laser is off */
		bool isOn(void);

	private:
		/* the pin of the laser */
		int pin;

		/* the status of the laser */
		bool on;
};

#endif /* LASER_H_ */
