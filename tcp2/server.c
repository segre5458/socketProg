#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <poll.h>
#include <sys/ioctl.h>

#include "common.h"

#define BUF_SIZE 500
#define BACKLOG_SIZE 5
#define POLL_SIZE 10

void loop(int sfd);

int main(int argc, char *argv[])
{
    int nfds = 0;
    int sfd;
    int val;
    struct pollfd fds[POLL_SIZE];

    if (argc != 2)
    {
        eprintf("Usage : %s port\n", argv[0]);
        return 1;
    }

    sfd = open_socket(NULL, argv[1]);
    if (sfd == -1)
    {
        eprintf("invalid sfd\n");
        return 1;
    }

    val = 1;
    ioctl(sfd, FIONBIO, &val);

    if (listen(sfd, BACKLOG_SIZE) == -1)
    {
        eprintf("could not listen\n");
        return 1;
    }

    int ready;
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
        ready = poll(fds, nfds, -1);
        if (ready == -1)
        {
            perror("ready");
            return 1;
        }
        for (int j = 0; j < nfds; j++)
        {
            if (fds[j].revents != 0)
            {
                printf("fd = &d; events: %s%s%s\n", fds[j].fd,
                       (fds[j].events & POLLIN) ? "POLLIN" : "",
                       (fds[j].revents & POLLHUP) ? "POLLHUP" : "",
                       (fds[j].revents & POLLERR) ? "POLLERR" : "");
            }
        }
        loop(sfd);
    }
}

void loop(int sfd)
{
    int s, fd;
    struct sockaddr_storage peer_addr;
    socklen_t peer_addr_len;
    ssize_t nread;
    char buf[BUF_SIZE];
    char host[NI_MAXHOST], service[NI_MAXSERV];

    peer_addr_len = sizeof(peer_addr);
    fd = accept(sfd, (struct sockaddr *)&peer_addr, &peer_addr_len);
    if (fd == -1)
    {
        eprintf("could not accept\n");
        return;
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
        nread = recv(fd, buf, BUF_SIZE, 0);
        if (nread == -1)
        {
            perror("read");
            continue;
        }
        if (nread == 0)
        {
            break;
        }

        printf("%sReceived %zd bytes: %s:%s\n", buf, nread, host, service);

        if (send(fd, buf, nread, 0) != nread)
        {
            eprintf("Error sending response\n");
        }
    }
    close(fd);
}
