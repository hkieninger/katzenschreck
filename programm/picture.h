#ifndef PICTURE
#define PICTURE 1

#include <stddef.h>

/* structure containing info about the picture
	@width: width of the picture
	@height: height of the picture */
struct picture_info {
	int width;
	int height;
	size_t size;
};

/* calculates a rgb picture from a yuv (or correctly ycbcr) picture (https://stackoverflow.com/questions/4041840/function-to-convert-ycbcr-to-rgb)
   @info: information about the picture
   @yuv_buffer: the buffer to read the yuv picture from
   @rgb_buffer: the buffer to write the rgb picture to, the rgb buffer must be 1.5 times bigger than the yuv_buffer*/
void yuv2rgb(const struct picture_info *info, const unsigned char *yuv_buffer, unsigned char *rgb_buffer);

/* transform the rgb picture in the buffer to a hsv picture (https://de.wikipedia.org/wiki/HSV-Farbraum)
   R = byte 0, G = byte 1, B = byte 2 -> H = byte 0, S = byte 1, V = byte 2
   @info: information about the picture
   @buffer: the buffer containing the rgb picture before and the hsv pictue after */
void rgb2hsv(const struct picture_info *info, unsigned char *buffer);

/* creates an jpeg picture from a rgb picture (https://github.com/LuaDist/libjpeg/blob/master/example.c, jdatadst.c in the libjpeg source code)
   @info: information about the picture
   @rgb: the buffer to read the rgb data from
   @jpeg: a pointer to a buffer which is big enough to contain the jpeg picture
   if the buffer is too small or jpeg points to NULL a new buffer, which is big enough will be allocated
   the new buffer (and the buffer which is too small) should be freed with free afterwards
   @size: the size of the jpeg buffer in bytes, if the jpeg buffer points to NULL size should be 0
   @return: the size of the jpeg picture in bytes */
unsigned long rgb2jpeg(const struct picture_info *info, unsigned char *rgb, unsigned char **jpeg, unsigned long size);

/* write a rgb picture into a file
   @file: the name of the file to write the picture, if the file already exist it will be overwritten
   @info: structure with info about the picture
   @rgb_buffer: buffer with the picture data */
void write_ppm(const char *file, const struct picture_info *info, const unsigned char *rgb_buffer);

/* write a jpeg picture into a file
   @file: the name of the file to write the picture, if the file already exist it will be overwritten
   @size: size of the jpeg pictue in bytes
   @jpeg_buffer: buffer with the picture data */
void write_jpeg(const char *file, unsigned long size, const unsigned char *jpeg_buffer);

#endif
