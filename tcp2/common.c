#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>

int eprintf(const char *fmt, ...)
{
    int n = 0;
    va_list ap;

    va_start(ap, fmt);
    n = vfprintf(stderr, fmt, ap);
    va_end(ap);

    return n;
}

int open_socket(const char *host, const char *port)
{
    struct addrinfo hints;
    struct addrinfo *result;
    const struct addrinfo *rp;
    int sfd, s;
    int is_server = host == NULL;
    int ret;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = is_server ? AI_PASSIVE : 0;
    hints.ai_protocol = INADDR_ANY;

    s = getaddrinfo(host, port, &hints, &result);
    if (s != 0)
    {
        eprintf("getaddrinfo: %s\n", gai_strerror(s));
        return -1;
    }

    for (rp = result; rp != NULL; rp = rp->ai_next)
    {
        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sfd != -1)
        {
            ret = is_server ? bind(sfd, rp->ai_addr, rp->ai_addrlen) : connect(sfd, rp->ai_addr, rp->ai_addrlen);
            if (ret != -1)
            {
                break;
            }
            close(sfd);
        }
    }

    freeaddrinfo(result);

    if (rp == NULL)
    {
        eprintf("could not connect\n");
        return -1;
    }

    return sfd;
}