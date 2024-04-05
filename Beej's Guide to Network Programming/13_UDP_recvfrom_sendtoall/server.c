#include "include.h"
#include "declare.h"
#include "define.h"

int main(void)
{
    int sockfdServer = -1, sockfdClient = -1, nRet = -1;
    struct sockaddr_in stSockAddrInServer, stSockAddrInClient;
    socklen_t serverLen = 0, clientLen = 0;
    char strTemp[32];
	
    sockfdServer = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfdServer == -1) {
        herror("socket");
        exit(EXIT_FAILURE);
    }

    bzero((void *)&stSockAddrInServer, sizeof(struct sockaddr_in));
    stSockAddrInServer.sin_family = AF_INET;
    stSockAddrInServer.sin_addr.s_addr = htonl(INADDR_ANY);
    stSockAddrInServer.sin_port = htons(PORT);
    serverLen = sizeof(struct sockaddr_in);
    nRet = bind(sockfdServer, (struct sockaddr *)&stSockAddrInServer, serverLen);
    if (nRet == -1) {
        herror("bind");
        exit(EXIT_FAILURE);
    }

    bzero((void *)&stSockAddrInClient, sizeof(struct sockaddr_in));
    clientLen = sizeof(struct sockaddr_in);
    printf("\nServer (%s: %u) is waiting.... \n", inet_ntoa(stSockAddrInServer.sin_addr), ntohs(stSockAddrInServer.sin_port));
    memset(strTemp, '\0', sizeof(char) * 32);
    while (1) {
        nRet = recvfrom(sockfdServer, (void *)strTemp, sizeof(char) * 32, 0, (struct sockaddr *)&stSockAddrInClient, &clientLen);
        if (nRet == -1) {
            herror("recvfrom");
            continue;
        }

        printf("\nServer recvfrom %d bytes from client (%s: %u): %s (%lu) \n", \
            nRet, inet_ntoa(stSockAddrInClient.sin_addr), ntohs(stSockAddrInClient.sin_port), strTemp, strlen(strTemp));
        strcat(strTemp, "World!! ");

        nRet = sendtoall(sockfdServer, (void *)strTemp, sizeof(char) * 32, 0, (struct sockaddr *)&stSockAddrInClient, clientLen);
        if (nRet == -1) {
            herror("sendto");
            continue;
        }

        printf("Server sendtoall %d bytes to client (%s: %u): %s (%lu) \n", \
            nRet, inet_ntoa(stSockAddrInClient.sin_addr), ntohs(stSockAddrInClient.sin_port), strTemp, strlen(strTemp));
        break;
    }
    close(sockfdServer);
    close(sockfdClient);
    printf("Server done!! \n");

    return 0;
}
