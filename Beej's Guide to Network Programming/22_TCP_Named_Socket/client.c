#include "include.h"
#include "define.h"

int main(void)
{
    int sockfdClient = -1, nRet = -1;
    struct sockaddr_un stSockAddrInClient;
    socklen_t clientLen = 0;
    char strTemp[32];

    sockfdClient = socket(AF_LOCAL, SOCK_STREAM, 0);
    if (sockfdClient < 0) {
        herror("socket");
        exit(EXIT_FAILURE);
    }

    bzero((void *)&stSockAddrInClient, sizeof(struct sockaddr_un));
    stSockAddrInClient.sun_family = AF_LOCAL;
    strcpy(stSockAddrInClient.sun_path, "private.sock");
    clientLen = sizeof(struct sockaddr_un);
    nRet = connect(sockfdClient, (struct sockaddr *)&stSockAddrInClient, clientLen);
    if (nRet == -1) {
        herror("connect");
        exit(EXIT_FAILURE);
    }

    memset(strTemp, '\0', sizeof(char) * 32);
    strcpy(strTemp, "Hello!! ");
    nRet = write(sockfdClient, (void *)strTemp, strlen(strTemp));
    printf("Client write %d bytes to server: %s (%lu) \n", nRet, strTemp, strlen(strTemp));
    sleep(1);

    nRet = read(sockfdClient, (void *)strTemp, sizeof(char) * 32);
    printf("Client read %d bytes from server: %s (%lu) \n", nRet, strTemp, strlen(strTemp));
    close(sockfdClient);
    printf("Client done!! \n");

    return 0;
}
