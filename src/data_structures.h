#ifndef DATA_STRUCTURES_H_
#define DATA_STRUCTURES_H_

#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>

typedef struct {
    pthread_t thread;
    int id;
    bool enabled;
} worker_thread;

typedef struct {
    int fd;
    void* next;
} node;

typedef struct {
    node* head;
    pthread_mutex_t mutex;
    sem_t semaphore;
} list;

void init_list(list* l);
void insert_list(list* l, int fd);
int extract_list(list* l);

#endif // DATA_STRUCTURES_H_
