#include "data_structures.h"
#include "utils.h"

#include <stdlib.h>

void init_list(list* l)
{
    if (l == NULL)
        return;

    l->head = NULL;
    pthread_mutex_init(&l->mutex, NULL);
    sem_init(&l->semaphore, 0, 0);
}

void insert_list(list* l, int fd)
{
    if (l == NULL)
        return;

    node* new_node = (node*)malloc(sizeof(node));
    DIE(new_node == NULL, "malloc");

    pthread_mutex_lock(&l->mutex);
    new_node->fd = fd;
    new_node->next = l->head;
    l->head = new_node;
    pthread_mutex_unlock(&l->mutex);

    sem_post(&l->semaphore);
}

int extract_list(list* l)
{
    int result;
    node* crt_node = NULL;

    if (l == NULL)
        return -1;

    sem_wait(&l->semaphore);
    pthread_mutex_lock(&l->mutex);
    result = l->head->fd;
    crt_node = l->head;
    l->head = (node*)l->head->next;
    free(crt_node);
    pthread_mutex_unlock(&l->mutex);

    return result;
}
