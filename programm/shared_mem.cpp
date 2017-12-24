#include <exception>
#include <stdexcept>

#include <errno.h>
#include <string.h>

#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/shm.h>

#include "shared_mem.h"

#define UNLOCK 1
#define LOCK -1

shared_mem::shared_mem(key_t semaphore_key, key_t shared_mem_key, size_t size, bool *create) {
    //create semaphore (incompatible with php semaphore, nsems in semget should be 3 instead of 1 -> buggy)
    semid = semget (semaphore_key, 0, IPC_PRIVATE);
    if(semid == -1) { //semaphore doesn't exist -> create it
    	if(!*create) {
    		*create = true;
    		return;
    	}
        semid = semget(semaphore_key, 1, IPC_CREAT | IPC_EXCL | 0666);
        if(semid == -1)
            throw std::runtime_error(strerror(errno));
        if(semctl(semid, 0, SETVAL, UNLOCK) == -1)
            throw std::runtime_error(strerror(errno));
    }
    //create shared memory
    shmid = shmget(shared_mem_key, size, 0666);
    if(shmid == -1) { //share memory doesn't exist -> create it
    	if(!*create) {
    		*create = true;
    		remove_semaphore();
    	}
        shmid = shmget(shared_mem_key, size, IPC_CREAT | IPC_EXCL | 0666);
        if(shmid == -1)
            throw std::runtime_error(strerror(errno));
    }
    //attach shared memory
    ptr = shmat(shmid, NULL, 0);
    if(ptr == (void *) -1)
        throw std::runtime_error(strerror(errno));
}

void shared_mem::lock_semaphore(void) {
    //aquire semaphore
    struct sembuf semaphore_lock[1] = {0, LOCK, SEM_UNDO};
    if(semop(semid, semaphore_lock, 1) == -1)
        throw std::runtime_error(strerror(errno));
}

void shared_mem::unlock_semaphore(void) {
    //release semaphore
    struct sembuf semaphore_unlock[1] = {0, UNLOCK, SEM_UNDO};
    if(semop(semid, semaphore_unlock, 1) == -1)
        throw std::runtime_error(strerror(errno));
}

void *shared_mem::shared_mem_ptr(void) {
    return ptr;
}

void shared_mem::dettach_shared_mem(void) {
    //dettach shared memory
	if(shmdt(ptr) == -1)
        throw std::runtime_error(strerror(errno));
}

void shared_mem::remove_shared_mem(void) {
    //remove shared memory
    if(shmctl(shmid, IPC_RMID, NULL) == -1)
        throw std::runtime_error(strerror(errno));
}

void shared_mem::remove_semaphore(void) {
    //remove the semaphore
    if(semctl(semid, 0, IPC_RMID) == -1)
        throw std::runtime_error(strerror(errno));
}
