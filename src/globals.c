#include "globals.h"

list global_list;
worker_thread threads[THREAD_POOL_SIZE];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
