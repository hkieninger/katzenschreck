#ifndef SHARED_MEM
#define SHARED_MEM 1

#include <sys/types.h>

/* the class combines a shared memory with a semaphore */
class shared_mem {
    public:
        /* constructor for a shared memory object
           tries to get an existing semaphore with the passed key, if the semaphore doesn't exist it is created
           tries to open an existing shared memory with the passed key, if the shared memory doesn't exist it is created
           attaches the shared memory
           @semaphore_key: the key for the semaphore (0 to 2^31 - 1)
           @shared_mem_key: the key for the shared memory
           @size: the size of the shared memory in bytes
           @create: if false the shared memory isn't created and create is set to true */
        shared_mem(key_t semaphore_key, key_t shared_mem_key, size_t size, bool *create);

        /* aquires the semaphore, this function should be called when accessing the shared memory */
        void lock_semaphore(void);

        /* releases the semaphore, this function should be called after accessing the shared memory */
        void unlock_semaphore(void);

        /* should be called to get the pointer to the shared memory
           @return: the pointer to the shared memory */
        void *shared_mem_ptr(void);

        /* dettaches the shared memory
           should be called when your done with the shared memory
           the shared memory still exist in the os and other processes can still access it */
        void dettach_shared_mem(void);

        /* removes the shared memory from the os */
        void remove_shared_mem(void);

        /* removes the semaphore from the os */
        void remove_semaphore(void);

    private:
        int shmid, semid;
        void *ptr;
};

#endif
