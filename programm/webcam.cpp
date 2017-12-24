#include <exception>
#include <stdexcept>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <limits.h>
#include <errno.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include <linux/videodev2.h>

#include "picture.h"
#include "webcam.h"

int webcam::devices(struct webcam_entry ***entry_list) {
	struct dirent **list;
	int entries = scandir("/sys/class/video4linux", &list, NULL, alphasort);
	if(entries < 2)
		return 0;
		
	*entry_list = (struct webcam_entry **) malloc((entries - 2) * sizeof(struct webcam_entry *));
	if(*entry_list == NULL)
		throw std::runtime_error(strerror(errno));
	
	for(int i = 2; i < entries; i++) {
		(*entry_list)[i - 2] = (struct webcam_entry *) malloc(sizeof(struct webcam_entry));
		
		size_t namelen = strlen(list[i]->d_name);
		snprintf((*entry_list)[i - 2]->device, 6 + namelen, "/dev/%s", list[i]->d_name); 
		
		char namefile[29 + namelen];
		snprintf(namefile, sizeof(namefile), "/sys/class/video4linux/%s/name", list[i]->d_name);
		int fd = open(namefile, O_RDONLY);
		if(fd < 0) {
			strncpy((*entry_list)[i - 2]->name, "no name", NAME_MAX);
			(*entry_list)[i - 2]->name[6] = '\0';
		} else {
			int ret = read(fd, (*entry_list)[i - 2]->name, NAME_MAX + 1);
			if(ret < 0) {
				strncpy((*entry_list)[i - 2]->name, "no name", NAME_MAX);
				(*entry_list)[i - 2]->name[6] = '\0';
			} else {
				(*entry_list)[i - 2]->name[ret - 1] = '\0';
			}
		}
		close(fd);
		
		free(list[i]);
	}
	free(list);
	
	return entries - 2;
}

webcam::webcam(const char *device, int width, int height) {
	//open device node
	fd = open(device, O_RDWR);
	if(fd < 0) {
		throw std::runtime_error(strerror(errno));
	}
	
	//check device capabilitys
	struct v4l2_capability cap;
	if(ioctl(fd, VIDIOC_QUERYCAP, &cap) < 0)
		throw std::runtime_error(strerror(errno));
	
	if(!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) || !(cap.capabilities & V4L2_CAP_STREAMING))
		throw std::runtime_error("video capture or streaming is not supported by this device");
	
	//set and check format
	struct v4l2_format format;
	format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	format.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
	format.fmt.pix.width = width;
	format.fmt.pix.height = height;

	if(ioctl(fd, VIDIOC_S_FMT, &format) < 0)
		throw std::runtime_error(strerror(errno));
	
	if(format.fmt.pix.pixelformat != V4L2_PIX_FMT_YUYV)
		throw std::runtime_error("yuyv format is not supported by this device");
	
	info.width = format.fmt.pix.width;
	info.height = format.fmt.pix.height;
	
	//request buffer (may use more buffers for simoultanosly process and get data)
	struct v4l2_requestbuffers bufrequest;
	bufrequest.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	bufrequest.memory = V4L2_MEMORY_MMAP;
	bufrequest.count = 1;
 
	if(ioctl(fd, VIDIOC_REQBUFS, &bufrequest) < 0)
		throw std::runtime_error(strerror(errno));
	
	//get buffer infos	
	memset(&buffer_info, 0, sizeof(buffer_info));
 
	buffer_info.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buffer_info.memory = V4L2_MEMORY_MMAP;
	buffer_info.index = 0;
 
	if(ioctl(fd, VIDIOC_QUERYBUF, &buffer_info) < 0)
		throw std::runtime_error(strerror(errno));
		
	info.size = buffer_info.length;
		
	//allocate the buffer
	buffer = (unsigned char *) mmap(NULL, buffer_info.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buffer_info.m.offset);

	if(buffer == MAP_FAILED)
		throw std::runtime_error(strerror(errno));

	memset(buffer, 0, buffer_info.length);
	
	//activate streaming (may first queue buffer in case it don't work)
	int type = buffer_info.type;
	if(ioctl(fd, VIDIOC_STREAMON, &type) < 0)
		throw std::runtime_error(strerror(errno));
}

void webcam::close_webcam(void) {
	//deactivate streaming
	int type = buffer_info.type;
	if(ioctl(fd, VIDIOC_STREAMOFF, &type) < 0)
		throw std::runtime_error(strerror(errno));
	
	//close device node
	if(close(fd) < 0)
		throw std::runtime_error(strerror(errno));
	
	//unmap the buffer
	if(munmap(buffer, buffer_info.length) < 0)
		throw std::runtime_error(strerror(errno));
}

void webcam::queue_buffer(void) {
	//put the buffer in the incoming queue.
	if(ioctl(fd, VIDIOC_QBUF, &buffer_info) < 0)
		throw std::runtime_error(strerror(errno));
}

void webcam::dequeue_buffer(void) {
	//the buffer is waiting in the outgoing queue.
	if(ioctl(fd, VIDIOC_DQBUF, &buffer_info) < 0)
		throw std::runtime_error(strerror(errno));
}

unsigned char *webcam::picture_data(void) {
	return buffer;
}
		
struct picture_info webcam::picture_information(void) {
	return info;
}
