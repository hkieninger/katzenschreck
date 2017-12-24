/*
 * share_manager.cpp
 *
 *  Created on: Oct 30, 2017
 *      Author: hans
 */
//#include <iostream> //debug
//#include <stdio.h> //debug

#include <exception>
#include <stdexcept>

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>

#include "picture.h"
#include "base64.h"
#include "object.h"
#include "shared_mem.h"

#define TIMESTAMP_LEN 28
#define OBJECT_LEN 141

static unsigned long long time_micros(void) {
	struct timeval tv;
	if(gettimeofday(&tv, NULL) == -1)
		throw std::runtime_error(strerror(errno));
	return 1000000 * tv.tv_sec + tv.tv_usec;
}

extern void settimestamp(shared_mem *shm) {
	*((unsigned long long *) shm->shared_mem_ptr()) = time_micros();
}

extern unsigned long long gettimestamp(shared_mem *shm) {
	return *((unsigned long long *) shm->shared_mem_ptr());
}

extern char *gettimestampstr(shared_mem *shm) {
	char *str = (char *) malloc(TIMESTAMP_LEN);
	int ret = snprintf(str, TIMESTAMP_LEN, "micros=%020llu", gettimestamp(shm));
	if(ret != TIMESTAMP_LEN - 1)
			throw std::runtime_error(strerror(errno));
	return str;
}

static inline struct object *getobjectptr(shared_mem *shm) {
	return (struct object *) ((char *) shm->shared_mem_ptr() + sizeof(unsigned long long) + sizeof(char));
}

extern struct object getobject(shared_mem *shm) {
	return *getobjectptr(shm);
}

extern void setobject(shared_mem *shm, struct object *obj) {
	*getobjectptr(shm) = *obj;
}

static char *getmoveptr(shared_mem *shm) {
	return (char * ) shm->shared_mem_ptr() + sizeof(unsigned long long);
}

extern char getmove(shared_mem *shm) {
	return *getmoveptr(shm);
}

extern void setmove(shared_mem *shm, char move) {
	*getmoveptr(shm) = move;
}

static int lastcgiint(char *str) {
	char *c = strrchr(str, '=');
	if(c == NULL)
		throw std::runtime_error("invalid cgi string");
	int i = atoi(c + 1);
	c = strrchr(str, '&');
	if(c == NULL)
		c = str;
	*c = '\0';
	return i;
}

extern void setcgistr(shared_mem *shm, char *str) {
	struct object *obj = getobjectptr(shm);
	obj->value_threshold = lastcgiint(str);
	obj->saturation_threshold = lastcgiint(str);
	obj->hue_threshold = lastcgiint(str);
	obj->value = lastcgiint(str);
	obj->saturation = lastcgiint(str);
	obj->hue = lastcgiint(str);
	*getmoveptr(shm) = lastcgiint(str);
//set move
	//debug
	/*std::cout << (int) obj->hue << std::endl;
	std::cout << (int) obj->saturation << std::endl;
	std::cout << (int) obj->value << std::endl;
	std::cout << (int) obj->hue_threshold << std::endl;
	std::cout << (int) obj->saturation_threshold << std::endl;
	std::cout << (int) obj->value_threshold << std::endl;*/
}

extern char *getobjectstr(shared_mem *shm) {
	struct object *obj = getobjectptr(shm);
	char *str = (char *) malloc(OBJECT_LEN);
	int ret = snprintf(str, OBJECT_LEN,
			"x=%11d,y=%11d,size=%11d,"
			"hue=%03d,saturation=%03d,value=%03d,"
			"hue_threshold=%03d,saturation_threshold=%03d,value_threshold=%03d",
			obj->x, obj->y, obj->size,
			obj->hue, obj->saturation, obj->value,
			obj->hue_threshold, obj->saturation_threshold, obj->value_threshold);
	if(ret != OBJECT_LEN -1)
		throw std::runtime_error(strerror(errno));
	return str;
}

static inline unsigned long guessmaxjpeglen(struct picture_info *info) {
	return info->width * info->height; //8bits per pixel (https://stackoverflow.com/questions/9806091/maximum-file-size-of-jpeg-image-with-known-dimensions)
}

extern void setpictureinfo(shared_mem *shm, struct picture_info *info) {
	*((struct picture_info *) ((char *) shm->shared_mem_ptr() + sizeof(unsigned long long) + sizeof(char) + sizeof(struct object))) = *info;
}

extern void setpicture(shared_mem *shm, int i, struct picture_info *info, unsigned char *picture) {
	unsigned long maxjpeglen = guessmaxjpeglen(info);
	char *ptr = (char *) shm->shared_mem_ptr() +
			sizeof(unsigned long long) +
			sizeof(char) +
			sizeof(struct object) +
			sizeof(struct picture_info) +
			i * (sizeof(unsigned long) + maxjpeglen);
	unsigned char *buffer = (unsigned char *) (ptr + sizeof(unsigned long));
	unsigned long jpeg_length = rgb2jpeg(info, picture, &buffer, maxjpeglen);
	if(jpeg_length > maxjpeglen) {
		free(buffer);
		throw std::runtime_error("jpeg buffer too small");
	}
	*((unsigned long *) ptr) = jpeg_length;
}

extern char *getpicturestr(shared_mem *shm, int i) {
	static char *base64buffer = NULL; //static variable to avoid calling malloc often, when function is called often
	struct picture_info info = *((struct picture_info *) ((char *) shm->shared_mem_ptr() + sizeof(unsigned long long) + sizeof(char) + sizeof(struct object)));
	unsigned long maxjpeglen = guessmaxjpeglen(&info);
	if(base64buffer == NULL)
		base64buffer = (char *) malloc(base64encode_len(maxjpeglen));
	char *ptr = (char *) shm->shared_mem_ptr() +
			sizeof(unsigned long long) +
			sizeof(char) +
			sizeof(struct object) +
			sizeof(struct picture_info) +
			i * (sizeof(unsigned long) + maxjpeglen);
	unsigned long jpeg_length = *((unsigned long *) ptr);
	int base64_length = base64encode_len(jpeg_length);
	base64encode(base64buffer, (char *) (ptr + sizeof(unsigned long)), maxjpeglen); //normally should be jpeg_length, but it doesn't work! why???
	base64buffer[base64_length - 1] = '\0';
	return base64buffer;
}

extern unsigned long sharelen(struct picture_info *info, int pictures) {
	return
			sizeof(unsigned long long) +
			sizeof(char) +
			sizeof(struct object) +
			sizeof(struct picture_info) +
			pictures * (guessmaxjpeglen(info) + sizeof(unsigned long));
}
