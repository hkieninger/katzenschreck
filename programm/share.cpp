#include <iostream>

#include <exception>
#include <stdexcept>

#include <stdlib.h>

#include "shared_mem.h"
#include "share_manager.h"
#include "cgi.h"

//have to correspond with the keys in main.cpp
#define SEMAPHORE_KEY 311903333L
#define SHARED_MEMORY_KEY 1727109034L

int main(void) {
	std::cout << "Content-Type: text/plain"<< std::endl << std::endl;

	try {
		bool create = false;
		shared_mem shm(SEMAPHORE_KEY, SHARED_MEMORY_KEY, 0, &create);
		if(create)
			throw std::runtime_error("shared memory doesn't exist, webcam-server is may not running");

		shm.lock_semaphore();

		char *cgi_string = cgi_data();
		if(cgi_string != NULL && cgi_string[0] != '\0') {
			cgi_data2string(cgi_string);
			//std::cout << cgi_string << std::endl; //debug
			setcgistr(&shm, cgi_string);
		}
		free(cgi_string);

		char *str;
		str = gettimestampstr(&shm);
		std::cout << str << ";";
		free(str);
		str = getobjectstr(&shm);
		std::cout << getobjectstr(&shm) << ";";
		free(str);
		std::cout << "originalleft=" << getpicturestr(&shm, 0) << ";";
		std::cout << "origninalright=" << getpicturestr(&shm, 1) << ";";
		std::cout << "processedleft=" << getpicturestr(&shm, 2) << ";";
		std::cout << "processedright=" << getpicturestr(&shm, 3) << ";";

		shm.unlock_semaphore();
		shm.dettach_shared_mem();
	} catch(std::exception& e) {
		std::cout << "exception: " << e.what();
		std::cerr << "share exception: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
