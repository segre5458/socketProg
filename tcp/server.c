#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define PORT 18000

int main()
{
    int sock;
    struct sockaddr_in address;
    struct sockaddr_in client;
    socklen_t len;
    int sock_cl;
    int n;
    int opt = 1;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket");
        return 1;
    }

    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
                     &opt, sizeof(opt)))
    {
        perror("setsocketopt");
        return 1;
    }

    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);
    address.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind");
        return 1;
    }

    if (listen(sock, 5) < 0)
    {
        perror("listen");
        return 1;
    }

    for (;;)
    {
        len = sizeof(client);
        if ((sock_cl = accept(sock, (struct sockaddr *)&client, &len) < 0))
        {
            perror("accept");
            break;
        }
        n = send(sock_cl, "HELLO", 5, 0);
        if (n < 0)
        {
            perror("send");
            break;
        }
        close(sock_cl);
    }
    close(sock);

    return 0;
}