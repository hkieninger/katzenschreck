/** Problems:
 * 		findObject->does not find always object
 * 		shared-memory->strange content, not enough semicolon = solved -> php bug with semaphore
 * 		segmentation fault -> core dump
 */

#include <iostream>
#include <exception>
#include <stdexcept>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/time.h>

#include <wiringPi.h>

#include "picture.h"
#include "webcam.h"
#include "object.h"
#include "shared_mem.h"
#include "share_manager.h"
#include "move.h"
#include "stepper_motor.h"
#include "servo_stepper.h"
#include "laser.h"

//randomly choosen keys for the shared memory and the coresponding semaphore
#define SEMAPHORE_KEY 311903333L
#define SHARED_MEMORY_KEY 1727109034L

//the desired width and height of the pictures
#define PICTURE_WIDTH 640
#define PICTURE_HEIGHT 480

//interval to display fps in seconds
#define FPS_DISPLAY_TIME 3

//global variables
bool is_running = true;

//pin configurations
const int motor_vertical_pin[] = {22, 23, 24, 25};
const int motor_horizontal_pin[] = {26, 27, 28, 29};
const int laser_pin = 21;

/* opens a webcam device
   @num: the number of the device entry
   @return: a pointer to the opened webcam device (don't forget to delete it) */
static webcam *openWebcam(int num) {
	struct webcam_entry **entry_list;
	int count = webcam::devices(&entry_list);
	if(count <= num)
		throw std::runtime_error("webcam not found");
	
	webcam *cam = new webcam(entry_list[num]->device, PICTURE_WIDTH, PICTURE_HEIGHT);
	std::cout << "device: " << entry_list[num]->device << "\tname: " << entry_list[num]->name << std::endl;
	
	for(int i = 0; i < count; i++) {
		free(entry_list[i]);
	}
	free(entry_list);

	return cam;
}

/* completes the picture_info structure for an rgb image, according to a yuv image
   @yuv_info: the info about the yuv image
   @rgb_info: the structure to be filled out */
static void yuv2rgb_info(const struct picture_info *yuv_info, struct picture_info *rgb_info) {
	rgb_info->width = yuv_info->width;
	rgb_info->height = yuv_info->height;
	rgb_info->size = yuv_info->size * 3 / 2;
}

static unsigned long long time_micros(void) {
	struct timeval tv;
	if(gettimeofday(&tv, NULL) == -1)
		throw std::runtime_error(strerror(errno));
	return 1000000l * tv.tv_sec + tv.tv_usec;
}

/* signal handler to terminate the programm properly on SIGINT */
static void sig_handler(int signal) {
	if(signal == SIGINT || signal == SIGTERM) {
		is_running = false;
	}
}

/* displays the frames per seconds all FPS_DISPLAY_TIME seconds */
static void display_fps(void) {
	static unsigned long long micros = time_micros();
	static unsigned int frames = 0;
	unsigned long long now = time_micros();
	if((now - micros) / 1000000 >= FPS_DISPLAY_TIME) {
		std::cout << "fps: " << (float) frames  * 1000000 / (now - micros) << std::endl;
		micros = now;
		frames = 0;
	}
	frames++;
}

//main function
int main(void) {
	/* setup part */
	if(setenv("WIRINGPI_CODES", "1", 0) == -1 || wiringPiSetup() == -1) {
		std::cerr << "wiringPiSetup error: " << strerror(errno) << std::endl;
		return EXIT_FAILURE;
	}
	signal(SIGINT, sig_handler);
	signal(SIGTERM, sig_handler);
	bool failure = false;
	webcam *camleft = NULL, *camright = NULL;
	shared_mem *shm = NULL;
	unsigned char *picture_buffer_left = NULL, *picture_buffer_right = NULL;
	stepper_motor motor(motor_horizontal_pin);
	servo_stepper servo(motor_vertical_pin);
	laser pointer(laser_pin);
	try {
		//open the webcam
		camleft = openWebcam(0);
		camright = openWebcam(1);
		
		//get the infos of the yuv picture and allocate the memory for the rgb picture
		struct picture_info yuv_info = camleft->picture_information();
		struct picture_info info;
		yuv2rgb_info(&yuv_info, &info);
		picture_buffer_left = (unsigned char *) malloc(info.size);
		picture_buffer_right = (unsigned char *) malloc(info.size);


		//create a semaphore and a shared memory containing information about the pictures, the object and the jpegs
		bool create = true;
		shm = new shared_mem(SEMAPHORE_KEY, SHARED_MEMORY_KEY, sharelen(&info, 4), &create);

		//the object structure representing the object to track
		struct object obj_left, obj_right;
		//track by default the green ball
		obj_left.hue = 81;
		obj_left.saturation = 130;
		obj_left.value = 115;
		//default thresholds
		obj_left.hue_threshold = 41;
		obj_left.saturation_threshold = 114;
		obj_left.value_threshold = 105;

		//set the picture info in the shared memory
		setpictureinfo(shm, &info);
		//set the object in the shared memory
		setobject(shm, &obj_left);

		//set the move in the shared memory
		setmove(shm, 0);

		/* main part */
		while(is_running) {
			display_fps();
			//take a picture and transform the data to rgb
			camleft->queue_buffer();
			camright->queue_buffer();
			camleft->dequeue_buffer();
			yuv2rgb(&yuv_info, camleft->picture_data(), picture_buffer_left);
			camright->dequeue_buffer();
			yuv2rgb(&yuv_info, camright->picture_data(), picture_buffer_right);

			//aquire/lock the semaphore
			shm->lock_semaphore();

			//get the object in the shared memory
			obj_left = getobject(shm);
			obj_right = obj_left;

			//update the timestamp in the shared memory
			settimestamp(shm);
			//put the picture in the shared memory
			setpicture(shm, 0, &info, picture_buffer_left);
			setpicture(shm, 1, &info, picture_buffer_right);

			//transform the picture to hsv
			rgb2hsv(&info, picture_buffer_left);
			rgb2hsv(&info, picture_buffer_right);

			//process the picture and track the object
			findeUmrissObject(&obj_left, &info, picture_buffer_left);
			findeUmrissObject(&obj_right, &info, picture_buffer_right);

			//put the processed picture in the shared memory
			setpicture(shm, 2, &info, picture_buffer_left);
			setpicture(shm, 3, &info, picture_buffer_right);

			if(makeSense(&obj_left, &obj_right, &info)) {
				//calculate the 3d position
				calculatePosition(&obj_left, &obj_right, &info);
				if(getmove(shm) == 1) {
					centerObject(&obj_left, &motor, &servo);
					pointer.turnOn();
				} else {
					pointer.turnOff();
				}
			} else {
				obj_left.x = -1;
				obj_left.y = -1;
				obj_left.size = -1;
				if(getmove(shm) != 1)
					pointer.turnOff();
			}

			//set the object in the shared memory
			setobject(shm, &obj_left);

			//release/unlock the semaphore
			shm->unlock_semaphore();
		}
	} catch(std::exception& e) {
		std::cerr << "running exception: " << e.what() << std::endl;
		failure = true;
	}

	/* cleanup part */
	//free the allocated buffers
	free(picture_buffer_left);
	free(picture_buffer_right);
	//release the resources associated with the webcam
	try {
		if(camleft != NULL)
			camleft->close_webcam();
	} catch(std::exception& e) {
		std::cerr << "cleanup exception webcamleft: " << e.what() << std::endl;
		failure = true;	
	}
	delete camleft;
	try {
	if(camright != NULL)
			camright->close_webcam();
	} catch(std::exception& e) {
		std::cerr << "cleanup exception webcamright: " << e.what() << std::endl;
		failure = true;
	}
	delete camright;
	//release the resoucres associated with the shared memory
	try {
		if(shm != NULL) {
			shm->dettach_shared_mem();
			shm->remove_shared_mem();
			shm->remove_semaphore();
		}
	} catch(std::exception& e) {
		std::cerr << "cleanup exception shared memory: " << e.what() << std::endl;
		failure = true;
	}
	delete shm;
	//exit the programm with the corresponding status
	if(failure)
		return EXIT_FAILURE;
	return EXIT_SUCCESS;
}
