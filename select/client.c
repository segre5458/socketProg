#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "common.h"

#define BUF_SIZE 500

struct addrinfo;
int main(int argc, char *argv[])
{
    int sfd;
    ssize_t len;
    ssize_t nread;
    char buf[BUF_SIZE];

    if (argc < 3)
    {
        eprintf("Usage : %s host port msg....\n", argv[0]);
        return 1;
    }

    sfd = open_socket(argv[1], argv[2]);
    if (sfd == -1)
    {
        eprintf("invalid sfd\n");
        return 1;
    }

    for (int j = 3; j < argc; j++)
    {
        len = strlen(argv[j]) + 1;
        if (len > BUF_SIZE)
        {
            eprintf("ignore message\n");
            continue;
        }
        if (write(sfd, argv[j], len) != len)
        {
            eprintf("failed write\n");
            return 1;
        }
        nread = read(sfd, buf, BUF_SIZE);
        if (nread == -1)
        {
            perror("read");
            return 1;
        }
        printf("Received %zd bytes: %s\n", nread, buf);
    }
    close(sfd);
    return 0;
}
