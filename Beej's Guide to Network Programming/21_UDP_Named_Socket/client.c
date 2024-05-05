#include "include.h"
#include "define.h"

int main(void)
{
    int sockfdClient = -1, nRet = -1;
    struct sockaddr_un stSockAddrInClient;
    socklen_t clientLen = 0;
    char strTemp[32];

    sockfdClient = socket(AF_LOCAL, SOCK_DGRAM, 0);
    if (sockfdClient < 0) {
        herror("socket");
        exit(EXIT_FAILURE);
    }

    bzero((void *)&stSockAddrInClient, sizeof(struct sockaddr_un));
    stSockAddrInClient.sun_family = AF_LOCAL;
    strcpy(stSockAddrInClient.sun_path, "client.sock");
    unlink("client.sock");
    nRet = bind(sockfdClient, (struct sockaddr *)&stSockAddrInClient, sizeof(struct sockaddr_un));
    if (nRet < 0) {
        herror("bind");
        exit(EXIT_FAILURE);
    }

    bzero((void *)&stSockAddrInClient, sizeof(struct sockaddr_un));
    stSockAddrInClient.sun_family = AF_LOCAL;
    strcpy(stSockAddrInClient.sun_path, "server.sock");
    nRet = connect(sockfdClient, (struct sockaddr *)&stSockAddrInClient, sizeof(struct sockaddr_un));
    if (nRet < 0) {
        herror("connect");
        exit(EXIT_FAILURE);
    }
	
    memset(strTemp, '\0', sizeof(char) * 32);
    strcpy(strTemp, "Hello!! ");
    nRet = send(sockfdClient, (void *)strTemp, strlen(strTemp), 0);
    printf("Client send %d bytes to server: %s (%lu) \n", nRet, strTemp, strlen(strTemp));
    sleep(1);
    nRet = recv(sockfdClient, (void *)strTemp, sizeof(char) * 32, 0);
    printf("Client recv %d bytes from server: %s (%lu) \n", nRet, strTemp, strlen(strTemp));
    unlink("client.sock");
    close(sockfdClient);
    printf("Client done!! \n");

    return 0;
}
