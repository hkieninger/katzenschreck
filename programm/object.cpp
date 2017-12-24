#include <exception>
#include <stdexcept>
#include <vector>

#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <errno.h>

#include <math.h>
#include <string.h>

#include "picture.h"
#include "object.h"

//these values are connected to a resolution of 640x480
#define FOCAL_LENGTH_1 1849.8625516485
#define FOCAL_LENGTH_2 1888.8087912088
#define EPSILON 0 * M_PI / 180
#define ALPHA_1 90 * M_PI / 180
#define ALPHA_2 90 * M_PI / 180
#define THETA_1 1.4320961841646 * M_PI / 180
#define THETA_2 2.1475854282985 * M_PI / 180
#define A 220

/*#define FOCAL_LENGTH_1 1850
#define FOCAL_LENGTH_2 1850
#define EPSILON 0 * M_PI / 180
#define ALPHA_1 90 * M_PI / 180
#define ALPHA_2 90 * M_PI / 180
#define THETA_1 0 * M_PI / 180
#define THETA_2 0 * M_PI / 180
#define A 220*/

//maximal size (percentage) of the object pixel amount relative to the total amount of pixels of the picture, float number between 1 and 0
#define MAX_SIZE 0.3f
//minimal size (percentage) of the object pixel amount relative to the total amount of pixels of the picture, float number between 1 and 0
#define MIN_SIZE 0.001f
//maximal difference of size (percentage) between the objects of the two pictures relative to their average size
#define DIFF_SIZE 0.8f
//maximal difference in the y position between the to objects (percentage of the amount of pixels of the picture height)
#define DIFF_PY 0.15f

/* identifies a color (hsv) as part of an object
   @obj: the object
   @h: the hue of the color
   @s: the saturation of the color
   @v: the value of the color
   @return: 255 if it is part of the object and 0 if it is not part of the object */
static inline unsigned char isObject(const struct object *obj, unsigned char h, unsigned char s, unsigned char v) {
	if(((obj->hue - h + 255) % 255 <= obj->hue_threshold || (h - obj->hue + 255) % 255 <= obj->hue_threshold) &&
	obj->saturation - obj->saturation_threshold <= s && s <= obj->saturation + obj->saturation_threshold &&
	obj->value - obj->value_threshold <= v && v <= obj->value + obj->value_threshold)
		return 255;
	return 0;
}

/* fills out the passed object structure for the object in a picture at the location pixel and
   sets all the red pixels of the object to 0 (follows the system of the neighboring pixels of the pixel, which are part of the object
   @pixel: the location of the pixel to start in the buffer
   @obj: the object structure to fill out
   @info: information about the picture
   @buffer: a buffer containing a preprocessed picture (RGB values are equal to 255 if part of the object and 0 if they aren't)
   @return: the size of the object (system of neighboring pixels, which are part of the object) */
static int locateObject(int pixel, struct object *obj, const struct picture_info *info, unsigned char *buffer) {
	if(buffer[pixel] == 255) {
		buffer[pixel] = 0;
		int x = pixel / 3 % info->width;
		int y = pixel / 3 / info->width;
		obj->x += x;
		obj->y += y;
		obj->size++;
	
		if(y > 1) {
			if(x > 1)
				locateObject(pixel - 3 * (info->width - 1), obj, info, buffer);
			locateObject(pixel - 3 * info->width, obj, info, buffer);
			if(x < info->width - 1)
				locateObject(pixel - 3 * (info->width + 1), obj, info, buffer);
		}
		if(x > 1)
			locateObject(pixel - 3, obj, info, buffer);
		if(x < info->width - 1)
			locateObject(pixel + 3, obj, info, buffer);
		if(y < info->height - 1) {
			if(x > 1)
				locateObject(pixel + 3 * (info->width - 1), obj, info, buffer);
			locateObject(pixel + 3 * (info->width), obj, info, buffer);
			if(x < info->width - 1)
				locateObject(pixel + 3 * (info->width + 1), obj, info, buffer);
		}
		return obj->size;
	}
	return 0;
}

/* marks the pixels of an object in a picture
   @pixel: a pixel located in the object (the pixel has to be part of the object)
   @info: information about the picture
   @buffer: a buffer containing a preprocessed picture (all R values set to 0, G and B values set to 255 if part of the object */
static void markObject(int pixel, const struct picture_info *info, unsigned char *buffer) {
	if(buffer[pixel + 1] == 255) {
		buffer[pixel + 1] = 0;
		buffer[pixel] = 255;
		int x = pixel / 3 % info->width, y = pixel / 3 / info->width;
	
		if(y > 1) {
			if(x > 1) {
				markObject(pixel - 3 * (info->width - 1), info, buffer);
			}
			markObject(pixel - 3 * info->width, info, buffer);
			if(x < info->width - 1) {
				markObject(pixel - 3 * (info->width + 1), info, buffer);
			}
		}
		if(x > 1) {
			markObject(pixel - 3, info, buffer);
		}
		if(x < info->width - 1) {
			markObject(pixel + 3, info, buffer);
		}
		if(y < info->height - 1) {
			if(x > 1) {
				markObject(pixel + 3 * (info->width - 1), info, buffer);
			}
			markObject(pixel + 3 * (info->width), info, buffer);
			if(x < info->width - 1) {
				markObject(pixel + 3 * (info->width + 1), info, buffer);
			}
		}
	}
}

//ein pixel nach vorne abhaengig der richtung
static inline void vor(int *x, int *y, int richtung) {
	switch(richtung) {
		case 0: (*x)++; break;
		case 1: (*y)++; break;
		case 2: (*x)--; break;
		default: (*y)--;
	}
}

//die richtung nach rechts drehen
static inline void rechtsUm(int *richtung) {
	(*richtung)++;
	*richtung %= 4;
}

//die richtung nach links drehen
static inline void linksUm(int *richtung) {
	*richtung += 3;
	*richtung %= 4;
}

//die richtung kehren
static inline void kehrt(int *richtung) {
	*richtung += 2;
	*richtung %= 4;
}

//ueberprueft ob das pixel vorne teil des objectes ist und ob der pixel im bild liegt
static inline bool vornObject(int x, int y, int richtung, const struct picture_info *info, unsigned char *buffer) {
	switch(richtung) {
		case 0: return x < info->width - 1 && buffer[3 * (y * info->width + x + 1)] == 255;
		case 1: return y < info->height - 1 && buffer[3 * ((y + 1) * info->width + x)] == 255;
		case 2: return x > 0 && buffer[3 * (y * info->width + x - 1)] == 255;
		default: return y > 0 && buffer[3 * ((y - 1) * info->width + x)] == 255;
	}
}

//ueberprueft ob das pixel links teil des objectes ist
static inline bool linksObject(int x, int y, int richtung, const struct picture_info *info, unsigned char *buffer) {
	linksUm(&richtung);
	return vornObject(x, y, richtung, info, buffer);
}

//ueberprueft ob das pixel rechts teil des objectes ist
static inline bool rechtsObject(int x, int y, int richtung, const struct picture_info *info, unsigned char *buffer) {
	rechtsUm(&richtung);
	return vornObject(x, y, richtung, info, buffer);
}

//ueberprueft ob das pixel hinten teil des objectes ist
static inline bool hintenObject(int x, int y, int richtung, const struct picture_info *info, unsigned char *buffer) {
	kehrt(&richtung);
	return vornObject(x, y, richtung, info, buffer);
}

//ueberprueft ob ein pixel herum (nachbar) teil des objectes ist
static inline bool herumObject(int x, int y, const struct picture_info *info, unsigned char *buffer) {
	return vornObject(x, y, 0, info, buffer) ||
			linksObject(x, y, 0, info, buffer) ||
			rechtsObject(x, y, 0, info, buffer) ||
			hintenObject(x, y, 0, info, buffer);
}

/* die idee/grundzuege zum/vom algorithmus stammt aus dem Buch: Programmieren spielend gelernt mit dem Java-Hamster modell
   @pixel: muss vom obersten Rand der Flaeche von der man den Umriss finden will sein
   ebenfalls muss der Pixel teil der Flaeche sein und eine Nachbarpixel haben der auch teil der Flaeche ist
   ansonsten kann das programm in eine endlos schlaufe geraten oder anderes unerwuenschtes Verhalten zeigen
   @info: informationen ueber das bild (breite/laenge)
   @buffer: puffer das das bild enthält
   @return: ein vector/liste der den umriss der flaeche enthaelt 
   gerader index = ost/linker Rand der Linie, ungerader index = west/rechter Rand der Linie */
static std::vector<int> *folgeUmriss(int pixel, const struct picture_info *info, unsigned char *buffer) {
	std::vector<int> *array = new std::vector<int>();
	int startX = pixel / 3 % info->width, startY = pixel / 3 / info->width;
	int x = startX, y = startY, richtung = 0; //0 = west, 1 = sued, 2 = ost, 3 = nord

	do {
		if(linksObject(x, y, richtung, info, buffer)) {
			linksUm(&richtung);
			vor(&x, &y, richtung);
		} else if(vornObject(x, y, richtung, info, buffer)) {
			vor(&x, &y, richtung);
		} else if(rechtsObject(x, y, richtung, info, buffer)) {
			rechtsUm(&richtung);
			vor(&x, &y, richtung);
		} else {
			kehrt(&richtung);
			vor(&x, &y, richtung);
		}

		buffer[3 * (y * info->width + x) + 1] = 255;

		int linie = y - startY;
		if(array->size() <= 2 * linie) {
			array->push_back(x);
			array->push_back(x);
		} else {
			if((*array)[2 * linie] > x)
				(*array)[2 * linie] = x;
			else if((*array)[2 * linie + 1] < x)
				(*array)[2 * linie + 1] = x;
		}

	} while(x != startX || y != startY);

	return array;
}

/* berechnet die flaeche des pixel system an @pixel und markiert/loescht die flaeche int/aus bestimmten farbteilen des @buffer */
static std::vector<int> *berechneObject(int pixel, struct object *obj, const struct picture_info *info, unsigned char *buffer) {
	/*if(buffer[pixel] == 0) {
		obj->size = 0;
		return NULL;
	}*/
	int startX = pixel / 3 % info->width, startY = pixel / 3 / info->width;
	if(!herumObject(startX, startY, info, buffer)) {
		obj->size = 1;
		return NULL;
	}
	std::vector<int> *umriss = folgeUmriss(pixel, info, buffer);
	int anzahl = 0, summeX = 0, summeY = 0;
	for(int y = 0; y < umriss->size(); y += 2) {
		int start = (*umriss)[y];
		int stop = (*umriss)[y + 1];
		for(int x = start; x < stop; x++) {
			int index = 3 * ((startY + y / 2) * info->width + x);
			if(buffer[index] == 255) {
				summeX += x;
				summeY += startY + y / 2;
				anzahl++;
				buffer[index + 2] = 255;
			}
			buffer[index] = 0;
		}
	}
	if(anzahl > 0) {
		obj->x = summeX / anzahl;
		obj->y = summeY / anzahl;
	}
	obj->size = anzahl;
	return umriss;
}

/* markiert das object an @pixel mit dem umriss @umriss */
static void markiereObject(int pixel, const struct picture_info *info, unsigned char *buffer, std::vector<int> *umriss) {
	int startY = pixel / 3 / info->width;
	for(int y = 0; y < umriss->size(); y += 2) {
		int start = (*umriss)[y];
		int stop = (*umriss)[y + 1];
		for(int x = start; x < stop; x++) {
			int index = 3 * ((startY + y / 2) * info->width + x);
			if(buffer[index + 2] == 255) {
				buffer[index] = 255;
			}
		}
	}
}

extern int findeUmrissObject(struct object *obj, const struct picture_info *info, unsigned char *buffer) {
	for(int i = 0; i < info->size; i += 3) {
		unsigned char isObj = isObject(obj, buffer[i], buffer[i + 1], buffer[i + 2]);
		buffer[i] = isObj;
		buffer[i + 1] = 0;
		buffer[i + 2] = 0;
	}

	std::vector<int> *umriss = NULL;
	int maxPixel = 0;
	int maxSize = 0;
	int maxX = 0;
	int maxY = 0;
	for(int i = 0; i < info->size; i+= 3) {
		if(buffer[i] != 0) {
			std::vector<int> *vec = berechneObject(i, obj, info, buffer);
			if(obj->size > maxSize) {
				delete umriss;
				umriss = vec;
				maxSize = obj->size;
				maxX = obj->x;
				maxY = obj->y;
				maxPixel = i;
			} else {
				delete vec;
			}
		}
	}

	if(umriss != NULL) {
		markiereObject(maxPixel, info, buffer, umriss);
		delete umriss;
	}
	obj->x = maxX;
	obj->y = maxY;
	obj->size = maxSize;

	return maxSize;
}

extern int findObject(struct object *obj, const struct picture_info *info, unsigned char *buffer) {
	for(int i = 0; i < info->size; i += 3) {
		unsigned char isObj = isObject(obj, buffer[i], buffer[i + 1], buffer[i + 2]);
		buffer[i] = isObj;
		buffer[i + 1] = isObj;
		buffer[i + 2] = isObj;
	}
	
	int maxX = 0;
	int maxY = 0;
	int maxSize = 0;
	int pixel = 0;
	for(int i = 0; i < info->size; i += 3) {
		obj->x = 0;
		obj->y = 0;
		obj->size = 0;
		if(locateObject(i, obj, info, buffer) > maxSize) {
			maxX = obj->x;
			maxY = obj->y;
			maxSize = obj->size;
			pixel = i;
		}
	}
	
	obj->size = maxSize;
	if(maxSize > 0) {
		obj->x = maxX / maxSize;
		obj->y = maxY / maxSize;
		markObject(pixel, info, buffer);
	}
	return maxSize;
}

extern int simplefindObject(struct object *obj, const struct picture_info *info, unsigned char *buffer) {
	obj->size = obj->x = obj->y = 0;
	for(int i = 0; i < info->size; i += 3) {
		unsigned char isObj = isObject(obj, buffer[i], buffer[i + 1], buffer[i + 2]);
		if(isObj == 255) {
			obj->x += i / 3 % info->width;
			obj->y += i / 3 / info->width;
			obj->size++;
		}
		buffer[i] = isObj;
		buffer[i + 1] = 0;
		buffer[i + 2] = 0;
	}
	if(obj->size != 0) {
		obj->x /= obj->size;
		obj->y /= obj->size;
	}
	return obj->size;
}

extern bool makeSense(struct object *left, struct object *right, struct picture_info *info) {
	//the objects shouldn't exceed a certain size or be smaller than a certain size
	float area = info->width * info->height;
	float percentage = left->size / area;
	if(percentage > MAX_SIZE || percentage < MIN_SIZE)
		return false;
	percentage = right->size / area;
	if(percentage > MAX_SIZE || percentage < MIN_SIZE)
		return false;
	//left and right should have a similar size
	float average = (left->size + right->size) / 2.0f;
	float diff = abs(left->size - right->size) / average;
	if(diff > DIFF_SIZE)
		return false;
	//left and right py should be similar
	return abs(left->y - right->y) / (float) info->height < DIFF_PY;
}

//connected to the resolutions of the webcams 640x480
extern struct object *calculatePosition(struct object *left, struct object *right, struct picture_info *info) {
	double leftx = left->x - 320;
	double lefty = left->y - 240;
	double rightx = 320 - right->x;
	double righty = right->y - 240;

	double radius1 = sqrt(pow(leftx, 2) + pow(lefty, 2));
	double angle1 = atan(lefty / leftx);
	if(leftx < 0)
		angle1 += M_PI;
	double px1 = radius1 * cos(angle1 + THETA_1);
	double py1 = radius1 * sin(angle1 + THETA_1);

	double radius2 = sqrt(pow(rightx, 2) + pow(righty, 2));
	double angle2 = atan(righty / rightx);
	if(rightx < 0)
		angle2 += M_PI;
	double px2 = radius2 * cos(angle2 + THETA_2);

	double gamma1 = ALPHA_1 - atan(px1 / FOCAL_LENGTH_1);
	double gamma2 = ALPHA_2 - atan(px2 / FOCAL_LENGTH_2);
	double s1 = A * sin(gamma1) / sin(gamma1 + gamma2);
	double x1 = s1 * cos(gamma1);
	double zeta1 = atan(py1 / FOCAL_LENGTH_1);
	double l1 = s1 / cos(zeta1);
	double y = l1 * sin(EPSILON + zeta1);
	double z = sqrt(pow(l1 * cos(EPSILON + zeta1), 2) - pow(x1, 2));
	double x = x1 - A / 2;

	left->x = round(x);
	left->y = round(y);
	left->size = round(z);

	return left;
}
