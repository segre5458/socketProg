#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main()
{
    int sok;
    if ((sok = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket");
        printf("socket failed\n");
        return 1;
    }
    return 0;
}