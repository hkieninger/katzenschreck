#include <exception>
#include <stdexcept>

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <jpeglib.h>

#include "picture.h"

static inline int max_int(int x, int y) {
	if(x > y)
		return x;
	return y;
}

static inline int min_int(int x, int y) {
	if(x < y)
		return x;
	return y;
}

extern void yuv2rgb(const struct picture_info *yuv_info, const unsigned char *yuv_buffer, unsigned char *rgb_buffer) {
	int y0, cb, y1, cr;
	int red, green, blue;
	int index;
	for(int i = 0; i < yuv_info->size; i += 4) {
		y0 = yuv_buffer[i];
		cb = yuv_buffer[i + 1];
		y1 = yuv_buffer[i + 2];
		cr = yuv_buffer[i + 3];
		
		index = i / 2 * 3;
		
		red = 140200 * (cr - 128) / 100000;
		green = (-34414 * (cb - 128) - 71414 * (cr - 128)) / 100000;
		blue = 177200 * (cb - 128) / 100000;
		
		//for the first pixel
		rgb_buffer[index] = max_int(0, min_int(255, y0 + red));
		rgb_buffer[index + 1] = max_int(0, min_int(255, y0 + green));
		rgb_buffer[index + 2] = max_int(0, min_int(255, y0 + blue));
		
		//for the second pixel
		rgb_buffer[index + 3] = max_int(0, min_int(255, y1 + red));
		rgb_buffer[index + 4] = max_int(0, min_int(255, y1 + green));
		rgb_buffer[index + 5] = max_int(0, min_int(255, y1 + blue));
	}
}

extern void rgb2hsv(const struct picture_info *info, unsigned char *buffer) {
	int red, green, blue, min, max;
	for(int i = 0; i < info->size; i += 3) {	
		red = buffer[i];
		green = buffer[i + 1];
		blue = buffer[i + 2];
		
		max = max_int(red, max_int(green, blue));
		min = min_int(red, min_int(green, blue));

		if(max == min) {
			buffer[i] = 0;
		} else if(max == red) {
			buffer[i] = 15300 * (green - blue) / (max - min) / 360;
		} else if(max == green) {
			buffer[i] = (30600 + 15300 * (blue - red) / (max - min)) / 360;
		} else {
			buffer[i] = (61200 + 15300 * (red - green) / (max - min)) / 360;
		}
		
		if(max == 0) {
			buffer[i + 1] = 0;
		} else {
			buffer[i + 1] = 255 * (max - min) / max;
		}
		
		buffer[i + 2] = max;
	}
}

/* https://stackoverflow.com/questions/19857766/error-handling-in-libjpeg */
static void jpeg_error_exit (j_common_ptr cinfo){
    char jpeg_last_error_msg[JMSG_LENGTH_MAX];
    (*( cinfo->err->format_message )) (cinfo, jpeg_last_error_msg);
    throw std::runtime_error(jpeg_last_error_msg);
}

extern unsigned long rgb2jpeg(const struct picture_info *info, unsigned char *rgb, unsigned char **jpeg, unsigned long size) {
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	
	cinfo.err = jpeg_std_error(&jerr);
	jerr.error_exit = jpeg_error_exit; //throw exception instead of exiting programm
	jpeg_create_compress(&cinfo);
	
	jpeg_mem_dest(&cinfo, jpeg, &size);
	
	cinfo.image_width = info->width;
	cinfo.image_height = info->height;
	cinfo.input_components = 3;
	cinfo.in_color_space = JCS_RGB; 
	jpeg_set_defaults(&cinfo);
	
	jpeg_start_compress(&cinfo, TRUE);
	
	unsigned char *buffer_array[1];
	int row_stride = cinfo.image_width * cinfo.input_components;
	while (cinfo.next_scanline < cinfo.image_height) {
		buffer_array[0] = rgb + cinfo.next_scanline * row_stride;
		jpeg_write_scanlines(&cinfo, buffer_array, 1);
	}
	
	jpeg_finish_compress(&cinfo);
	jpeg_destroy_compress(&cinfo);
	
	return size;
}

extern void write_ppm(const char *file, const struct picture_info *info, const unsigned char *rgb_buffer) {
	int fd = open(file, O_CREAT | O_WRONLY, 0644);
	if(fd < 0)
		throw std::runtime_error(strerror(errno));
	char header[1024];
	int ret = sprintf(header, "P6 %d %d 255\n", info->width, info->height);
	if(write(fd, header, ret) < 0)  //write ppm header
		throw std::runtime_error(strerror(errno));
	if(write(fd, rgb_buffer, info->size) < 0) //write rgb pixel data
		throw std::runtime_error(strerror(errno));
	close(fd);
}

extern void write_jpeg(const char *file, unsigned long size, const unsigned char *jpeg_buffer) {
	int fd = open(file, O_CREAT | O_WRONLY, 0644);
	if(fd < 0)
		throw std::runtime_error(strerror(errno));
	if(write(fd, jpeg_buffer, size) < 0)
		throw std::runtime_error(strerror(errno));
	close(fd);
}
