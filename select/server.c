#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <poll.h>

#include "common.h"

#define BUF_SIZE 500
#define BACKLOG_SIZE 5
#define POLL_SIZE 10

void loop(fd_set sfd, int maxfd, int fd1, int fd2);
ssize_t recieve_buf(fd_set fds, int fd, char buf[], size_t n, int flags);

int main(int argc, char *argv[])
{
    int sfd1, sfd2;
    int maxfd;
    fd_set rfds;
    // struct pollfd fds[POLL_SIZE];

    if (argc != 3)
    {
        eprintf("Usage : %s port1 port2\n", argv[0]);
        return 1;
    }

    sfd1 = open_socket(NULL, argv[1]);
    if (sfd1 == -1)
    {
        eprintf("invalid sfd\n");
        return 1;
    }
    sfd2 = open_socket(NULL, argv[2]);
    if (sfd2 == -1)
    {
        eprintf("invalid sfd\n");
        return 1;
    }

    if (listen(sfd1, BACKLOG_SIZE) == -1)
    {
        eprintf("could not listen\n");
        return 1;
    }
    if (listen(sfd2, BACKLOG_SIZE) == -1)
    {
        eprintf("could not listen\n");
        return 1;
    }

    // val = 1;
    // ioctl(sfd, FIONBIO, &val);

    // if (listen(sfd1, BACKLOG_SIZE) == -1)
    // {
    //     eprintf("could not listen\n");
    //     return 1;
    // }

    FD_ZERO(&rfds);
    FD_SET(sfd1, &rfds);
    FD_SET(sfd2, &rfds);
    maxfd = sfd1 > sfd2 ? sfd1 : sfd2;

    // TO DO: 1 vs n by POLL/EPOLL (or SELECT)
    // polling non blocking
    // accept → blocking
    // nonblocking → ioctl accept
    // 通信あり or acceptあり or acceptなし
    // acceptあり→fdsに登録nfdsを増やすnfdsは増えていく、保存だけ
    // pollはfdsの中でcheck
    // 通信あり→event見て
    // acceptなし→サーバー自身の処理をする、解析等
    for (;;)
    {
        // ready = poll(fds, nfds, -1);
        // if (ready == -1)
        // {
        //     perror("ready");
        //     return 1;
        // }
        // for (int j = 0; j < nfds; j++)
        // {
        //     if (fds[j].revents != 0)
        //     {
        //         printf("fd = &d; events: %s%s%s\n", fds[j].fd,
        //                (fds[j].events & POLLIN) ? "POLLIN" : "",
        //                (fds[j].revents & POLLHUP) ? "POLLHUP" : "",
        //                (fds[j].revents & POLLERR) ? "POLLERR" : "");
        //     }
        // }
        loop(rfds, maxfd, sfd1, sfd2);
    }
}

void loop(fd_set sfd, int maxfd, int fd1, int fd2)
{
    int retval;
    int n;
    fd_set fds;
    struct timeval tv;
    char buf[BUF_SIZE];

    memcpy(&fds, &sfd, sizeof(fd_set));
    tv.tv_sec = 10;
    tv.tv_usec = 0;

    retval = select(maxfd + 1, &fds, NULL, NULL, &tv);
    if (retval == -1)
    {
        perror("select()");
        return;
    }
    else if (retval == 0)
    {
        eprintf("no data input in 10 sec\n");
        return;
    }

    n = recieve_buf(fds, fd1, buf, BUF_SIZE, 0);
    if (n == -1)
    {
        eprintf("could not receive\n");
        return;
    }
    n = recieve_buf(fds, fd2, buf, BUF_SIZE, 0);
    if (n == -1)
    {
        eprintf("could not receive\n");
        return;
    }
}

ssize_t recieve_buf(fd_set fds, int fd, char buf[], size_t n, int flags)
{
    if (FD_ISSET(fd, &fds))
    {
        int s;
        ssize_t nread;
        struct sockaddr_storage peer_addr;
        socklen_t peer_addr_len;
        char host[NI_MAXHOST], service[NI_MAXSERV];

        memset(buf, 0, BUF_SIZE);

        peer_addr_len = sizeof(peer_addr);
        fd = accept(fd, (struct sockaddr *)&peer_addr, &peer_addr_len);
        if (fd == -1)
        {
            eprintf("could not accept\n");
            return -1;
        }

        s = getnameinfo((struct sockaddr *)&peer_addr,
                        peer_addr_len, host, NI_MAXHOST,
                        service, sizeof(service), NI_NUMERICSERV);
        if (s != 0)
        {
            eprintf("getnameinfo:%s\n", gai_strerror(s));
        }

        for (;;)
        {
            nread = recv(fd, buf, n, flags);
            if (nread == -1)
            {
                perror("read");
                // continue;
                return -1;
            }
            if (nread == 0)
            {
                break;
            }

            printf("%s\tReceived %zd bytes: %s:%s\n", buf, nread, host, service);

            if (send(fd, buf, nread, 0) != nread)
            {
                eprintf("Error sending response\n");
                return -1;
            }
        }

        close(fd);

        return nread;
    }
    else
    {
        return 0;
    }
}
