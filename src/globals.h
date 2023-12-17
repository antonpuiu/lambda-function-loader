#ifndef GLOBALS_H_
#define GLOBALS_H_

#include "data_structures.h"

#define THREAD_POOL_SIZE 12

extern list global_list;
extern worker_thread threads[THREAD_POOL_SIZE];
extern pthread_mutex_t mutex;

#endif // GLOBALS_H_
