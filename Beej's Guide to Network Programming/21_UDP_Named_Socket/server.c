#include "include.h"
#include "define.h"

int main(void)
{
    int sockfdServer = -1, nRet = -1;
    struct sockaddr_un stSockAddrInServer, stSockAddrInClient;
    socklen_t serverLen = 0, clientLen = 0;
    char strTemp[32];

    sockfdServer = socket(AF_LOCAL, SOCK_DGRAM, 0);
    if (sockfdServer < 0) {
        herror("socket");
        exit(EXIT_FAILURE);
    }

    bzero((void *)&stSockAddrInServer, sizeof(struct sockaddr_un));
    stSockAddrInServer.sun_family = AF_LOCAL;
    strcpy(stSockAddrInServer.sun_path, "server.sock");
    serverLen = sizeof(struct sockaddr_un);
    unlink("server.sock");
    nRet = bind(sockfdServer, (struct sockaddr *)&stSockAddrInServer, serverLen);
    if (nRet < 0) {
        herror("bind");
        exit(EXIT_FAILURE);
    }
	
    bzero((void *)&stSockAddrInClient, sizeof(struct sockaddr_un));
    clientLen = sizeof(struct sockaddr_un);
    memset(strTemp, '\0', sizeof(char) * 32);
    printf("Server is waiting.... \n");
    while(1) {
        nRet = recvfrom(sockfdServer, (void *)strTemp, sizeof(char) * 32, 0, (struct sockaddr *)&stSockAddrInClient, &clientLen);
        if (nRet < 0) {
            herror("recvfrom");
            continue;
        }
        printf("Server recvfrom %d bytes from client: %s (%lu) \n", nRet, strTemp, strlen(strTemp));
        strcat(strTemp, "World!! ");
        nRet = sendto(sockfdServer, (void *)strTemp, strlen(strTemp), 0, (struct sockaddr *)&stSockAddrInClient, clientLen);
        if (nRet == -1) {
            herror("sendto");
            continue;
        }
        printf("Server sendto %d bytes to client: %s (%lu) \n", nRet, strTemp, strlen(strTemp));
        break;
    }
    unlink("server.sock");
    close(sockfdServer);

    return 0;
}
