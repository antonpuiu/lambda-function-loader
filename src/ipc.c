// SPDX-License-Identifier: BSD-3-Clause

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <unistd.h>

#include "ipc.h"
#include "log.h"
#include "utils.h"

#include <arpa/inet.h>

int create_socket(void)
{
    /* Create socket. */
    int fd = socket(PF_UNIX, SOCK_STREAM, 0);
    DIE(fd < 0, "open");

    return fd;
}

int connect_socket(int fd)
{
    struct sockaddr_un addr;

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    snprintf(addr.sun_path, sizeof(SOCKET_NAME), "%s", SOCKET_NAME);

    int rc = connect(fd, (struct sockaddr*)&addr, sizeof(addr));
    DIE(rc < 0, "connect");

    return 0;
}

ssize_t send_socket(int fd, const char* buf, size_t len)
{
    int rc = write(fd, buf, len);
    DIE(rc < 0, "write");

    return rc;
}

ssize_t recv_socket(int fd, char* buf, size_t len)
{
    int rc = read(fd, buf, len);
    DIE(rc < 0, "read");

    return rc;
}

void close_socket(int fd)
{
    close(fd);
}
