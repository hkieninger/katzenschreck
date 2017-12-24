#ifndef WEBCAM
#define WEBCAM 1

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include <linux/videodev2.h>
#include <jpeglib.h>

/* structure containing a entry for a webcam
	@device: the device node
	@name: the name of the device */
struct webcam_entry {
	char device[NAME_MAX + 6];
	char name[NAME_MAX + 1];
};

/* structure containing info about the format of an rgb picture
	@width: width of the picture
	@height: height of the picture
	@bytes_per_pixel: amount of bytes per pixel */
struct pixel_info {
	int width;
	int height;
	int bytes_per_pixel;
};

/* structure containing info about the format of an mjpeg picture
	@length: the amount of bytes for the entire picture */
struct mjpeg_info {
	int length;
};

/* the webcam class represents a webcam device */
class webcam {
	public:
		/* returns a list of webcam devices attached to the computer, the memory allocated by this function must be freed afterwards
			@entry_list: ptr to a list of webcam_entry structures to write the entries in
			@return: the number of attached devices */
		static int devices(struct webcam_entry ***entry_list);

		/* constructor for the webcam object, allocate the required resources
			@device: the device node
			@width: desired width for the picture, the actual width doesn't have to correspond with this parameter
			@height: desired height for the picture, the actual height doesn't have to correspond with this parameter */
		webcam(const char *device, int width, int height);
		
		/* free the resources, the camera can't be used any more */
		void close_webcam(void);
		
		/* takes a picture wich can then be received via rgb_data or mjpeg_data */
		void take_picture(void);
		
		/* writes information about the format of a rgb picture into a structure (should be called after take picture)
			@info: the structure to write the information to*/
		void rgb_info(struct pixel_info *info);
		
		/*writes information about the format of a mjpeg picture into a structure
			@info: the structure to write the information to*/
		void mjpeg_info(struct mjpeg_info *info);
		
		/*decompress the underlying mjpeg picture and writes it into the passed buffer
			@buffer: the buffer to write the decompressed picture to
			@return: the passed buffer */
		unsigned char *rgb_data(unsigned char *rgb_buffer);
		
		/* returns the underlying buffer with the mjpeg picture
			@return: the buffer with the mjpeg picture */
		unsigned char *mjpeg_data(void);
		
		/* writes a jpeg picture into a file
			@file: the name of the file to write the picture, if the file already exist it will be overwritten
			@info: structure with info about the picture
			@buffer: buffer with the picture data */
		static void write_mjpeg(const char *file, struct mjpeg_info *info, unsigned char *buffer);
		
		/* write a rgb picture into a file
			@file: the name of the file to write the picture, if the file already exist it will be overwritten
			@info: structure with info about the picture
			@rgb_buffer: buffer with the picture data */
		static void write_ppm(const char *file, struct pixel_info *info, unsigned char *rgb_buffer);

	private:
		/* file descriptor of the device node */
		int fd;
		
		/*structure containing info about the buffer to write the mjpeg picture to */
		struct v4l2_buffer buffer_info;
		
		/*the buffer to write the mjpeg picture to */
		unsigned char *buffer;
		
		/*structure for the decompression of the mjpeg picture */
		struct jpeg_decompress_struct decompress_info;
		
		/*structure for the handling of errors of the decompression */
		struct jpeg_error_mgr jerr;
};

#endif
