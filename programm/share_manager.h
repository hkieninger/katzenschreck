/*
 * share_manager.h
 *
 * these are some helper functions to set and get the data in an ordered way to and from the shared memory
 * the programmer still has to lock and unlock the shared memory before and after calling these functions to avoid conflicts
 *
 * if these functions are used, the shared memory shouldn't be accessed from the outside to avoid a mess
 *
 *  Created on: Oct 30, 2017
 *      Author: hans
 */

#ifndef SHARE_MANAGER_H
#define SHARE_MANAGER_H

#include "picture.h"
#include "object.h"
#include "shared_mem.h"

/* sets a timestamp (time in micros since 1970) in the shared memory
 * @shm: the shared memory to set the timestamp */
void settimestamp(shared_mem *shm);

/* gets the timestamp from the shared memory
 * @shm: the shared memory to get the timestamp from
 * @return: the timestamp, micros since 1970 */
unsigned long long gettimestamp(shared_mem *shm);

/* gets the timestamp as string
 * @shm: the shared memory to get the timestamp from
 * @return: the timestamp as string, string should be freed with free */
char *gettimestampstr(shared_mem *shm);

/* copies the content of the object into the shared memory
 * @shm: the shared memory to set the object
 * @obj: the object the copy */
void setobject(shared_mem *shm, struct object *obj);

/* get a copy of the object in the shared memory
 * @shm: the shared memory to get the object from
 * @return: a copy of the object in the shared memory */
struct object getobject(shared_mem *shm);

/* returns the move char from the shared memory
 * @shm: the shared memory
 * @return: the char */
char getmove(shared_mem *shm);

/* set the move char in the  shared memory
 * @shm: the shared memory
 * @move: the move value */
void setmove(shared_mem *shm, char move);

/* modifies the color and color thresholds of the  object in the shared memory according to the passed string, it also modifies the move char
 * @shm: the shared memory
 * @str: the string containing 7 bytes (hsv components and thresholds) and the move char*/
void setcgistr(shared_mem *shm, char *str);

/* get the object as string
 * @shm: the shared memory
 * @return: a string representing the object (name=value pairs separated by ','), string should be freed with free */
char *getobjectstr(shared_mem *shm);

/* set infos about the pictures in the shared memory (all the pictures must have the same width, height and size)
 * @shm: the shared memory
 * @info: the structure containing the information about the pictures */
void setpictureinfo(shared_mem *shm, struct picture_info *info);

/* sets a rgb picture in the shared memory (the picture is compressed to a jpeg)s
 * @shm: the shared memory
 * @i: index of the picture (used to access it when getting it), start with 0 and add one for each succeeding picture
 * @picture: the buffer containing the pixel data of the picture in rgb */
void setpicture(shared_mem *shm, int i, struct picture_info *info, unsigned char *picture);

/* get a base64 string representing a jpeg picture from the shared memory
 * @shm: the shared memory
 * @i: the index of the picture when it was set
 * @return: the base64 string, memory mustn't be freed afterwards, else the behaviour is undefined */
char *getpicturestr(shared_mem *shm, int i);

/* returns the size needed for the shared memory depending of the picture size and the amount of pictures
 * @info: the structure containing information about the pictures
 * @pictures: the amount of pictures */
unsigned long sharelen(struct picture_info *info, int pictures);

#endif
