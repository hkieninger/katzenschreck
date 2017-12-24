#ifndef WEBCAM
#define WEBCAM 1

#include <limits.h>
#include <linux/videodev2.h>

#include "picture.h"

/* structure containing a entry for a webcam
	@device: the device node
	@name: the name of the device */
struct webcam_entry {
	char device[NAME_MAX + 6];
	char name[NAME_MAX + 1];
};

/* the webcam class represents a yuyv 4:2:2 webcam device (http://jwhsmith.net/2014/12/capturing-a-webcam-stream-using-v4l2/) */
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
		
		/* queues the internal buffer, next dequeue_buffer should be called */
		void queue_buffer(void);

		/*deques the buffer, now the data can be retreived with picture_data wich returns the internal buffer */
		void dequeue_buffer(void);
		
		/* returns a structure containing information about the picture
			@return: a structure containing the information */
		struct picture_info picture_information(void);
		
		/* returns a pointer to the buffer wich contains the yuv 4:2:2 data
		   4 bytes represent 2 pixels in following sequence y cb y cr (http://v4l.videotechnology.com/dwg/v4l2.html#V4L2-PIX-FMT-YUYV)
			@return: the pointer to the data */
		unsigned char *picture_data(void);

	private:
		/* file descriptor of the device node */
		int fd;
		
		/* structure containing info about the buffer to write the yuv 4:2:2 picture to */
		struct v4l2_buffer buffer_info;
		
		/* structure containing info about the picture */
		struct picture_info info;
		
		/*the buffer to write the mjpeg picture to */
		unsigned char *buffer;
};

#endif
