// SPDX-License-Identifier: BSD-3-Clause
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <unistd.h>

#include "ipc.h"
#include "utils.h"
#include "data_structures.h"
#include "client_handler.h"
#include "globals.h"

void* handle_client(void* arg)
{
    pid_t ret_pid;
    pid_t pid;
    int status;

    int id = *((int*)arg);

    while (threads[id].enabled) {
        int fd = extract_list(&global_list);

        pid = fork();
        switch (pid) {
        case -1:
            DIE(1, "fork");
            break;

        case 0:
            /* Child process */
            thread_function(fd);
            break;

        default:
            /* Parent process */
            ret_pid = waitpid(pid, &status, 0);
            DIE(ret_pid < 0, "waitpid parent");

            close(fd);
            break;
        }
    }

    return NULL;
}

void launch_threads()
{
    for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        threads[i].id = i;
        threads[i].enabled = true;
        pthread_create(&threads[i].thread, NULL, handle_client, &threads[i].id);
    }
}

void close_threads()
{
    for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        threads[i].enabled = false;
        pthread_join(threads[i].thread, NULL);
    }
}

int main(void)
{
    /* TODO: Implement server connection. */
    int ret = 0;

    setvbuf(stdout, NULL, _IONBF, 0);

    /* Remove socket_path. */
    remove(SOCKET_NAME);

    /* Create socket. */
    int listenfd = create_socket();

    /* Bind socket to path. */
    struct sockaddr_un addr, raddr;
    socklen_t raddrlen;

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    snprintf(addr.sun_path, strlen(SOCKET_NAME) + 1, "%s", SOCKET_NAME);

    int rc = bind(listenfd, (struct sockaddr*)&addr, sizeof(addr));
    DIE(rc < 0, "bind");

    /* Put in listen mode. */
    rc = listen(listenfd, 10);
    DIE(rc < 0, "listen");

    /* Init list */
    init_list(&global_list);

    /* Start threads */
    launch_threads();

    while (1) {
        /* Accept connection. */
        int connectfd = accept(listenfd, (struct sockaddr*)&raddr, &raddrlen);
        DIE(connectfd < 0, "accept");

        insert_list(&global_list, connectfd);
    }

    close_threads();

    return ret;
}
