#ifndef OBJECT
#define OBJECT 1

#include "picture.h"

/* structure representing an object identified by its color (hsv) in a picture
   @x: the x coordinate of the object in the picture
   @y: the y coordinate of the object in the picture
   @hue: the hue of the color of the object
   @saturation: the saturation of the color of the object
   @value: the value of the color of the object
   @hue_threshold: threshold for the hue
   @saturation_threshold: threshold for the saturation
   @value_threshold: threshold for the value */ 
struct object {
	int x, y, size;
	unsigned char hue, saturation, value;
	unsigned char hue_threshold, saturation_threshold, value_threshold;
};

/* fills out a passed object structure for an passed picture and modifys the picture to mark the object in it
   it takes the average of the coordinates of the pixels in the biggest system belonging to the object
   the function work with a recursive function, for big area it causes a segmentation fault, because of a stack overflow
   for this reason the function findeUmrissObject is recommended instead
   @obj: the structure to fill out
   @info: information about the picture
   @buffer: a buffer containing a picture in hsv format, on return containing a rgb picture with the object marked in it
   @return: the size of the object (amount of pixels) */
int findObject(struct object *obj, const struct picture_info *info, unsigned char *buffer);

/* fills out a passed object structure for an passed picture and modifys the picture to mark the object in it
   it takes the average of the coordinates of the pixels belonging to the object
   @obj: the structure to fill out
   @info: information about the picture
   @buffer: a buffer containing a picture in hsv format, on return containing a rgb picture with the object marked in it
   @return: the size of the object (amount of pixels) */
int simplefindObject(struct object *obj, const struct picture_info *info, unsigned char *buffer);

/* calculates the 3 dimensional position of an object from the position of the object in two pictures
 * the results are then stored in the left object structure
 * the x,y and z positions are given in milimeters from the middle between the webcams
 * the z position is stored in the element size of the object structure
 * @left: the object of the left picture, it will be modified
 * @right: the object of the right picture
 * @info: structure containing width and height of the picture
 * @return: the object structure left */
struct object *calculatePosition(struct object *left, struct object *right, struct picture_info *info);

/* fills out a passed object structure for an passed picture and modifys the picture to mark the object in it
   it takes the average of the coordinates of the pixels in the biggest system belonging to the object
   the function work with the shape/border of all the objects to recognize the biggest one
   @obj: the structure to fill out
   @info: information about the picture
   @buffer: a buffer containing a picture in hsv format, on return containing a rgb picture with the object marked in it
   @return: the size of the object (amount of pixels) */
int findeUmrissObject(struct object *obj, const struct picture_info *info, unsigned char *buffer);

/* gives a feedback if it make sense to calculate the 3d position of the object, dependend of the positions in the pictures
 * @left: the object of the left picture
 * @right: the object of the right picture
 * @return: true if it makes sense and false if it doesn't make sense */
bool makeSense(struct object *left, struct object *right, struct picture_info *info);

#endif
