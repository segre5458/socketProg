#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>

#define PORT 18000

int main(int argc, char *argv[])
{
    struct sockaddr_in server;
    int sock;
    char buf[32];
    char *deststr;
    unsigned **addrptr;
    int n;

    if (argc != 2)
    {
        printf("Usage : %s dest\n", argv[0]);
        return 1;
    }
    deststr = argv[1];

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket");
        return 1;
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    if (inet_aton(deststr, &(server.sin_addr)) != 0)
    {
        struct hostent *host;
        if ((host = gethostbyname(deststr)) == NULL)
        {
            printf("%s : %s\n", hstrerror(h_errno), deststr);
            return 1;
        }

        addrptr = (unsigned **)host->h_addr_list;
        while (*addrptr != NULL)
        {
            server.sin_addr.s_addr = *(*addrptr);
            if (connect(sock, (struct sockaddr *)&server, sizeof(server)) == 0)
            {
                break;
            }
            addrptr++;
        }
    }
    else
    {
        if (connect(sock, (struct sockaddr *)&server, sizeof(server)) != 0)
        {
            perror("connect");
            return 1;
        }
    }

    memset(buf, 0, sizeof(buf));
    if ((n = read(sock, buf, sizeof(buf))) < 0)
    {
        perror("read");
        return 1;
    }

    printf("%d, %s\n", n, buf);
    close(sock);
    return 0;
}