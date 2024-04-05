#include "include.h"
#include "define.h"

int main(void)
{
    int sockfdServer = -1, sockfdClient = -1, nRet = -1;
    struct sockaddr_in stSockAddrInServer, stSockAddrInClient;
    socklen_t serverLen = 0, clientLen = 0;
    char strTemp[32];
	
    sockfdServer = socket(AF_INET, SOCK_STREAM, 0);
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

    nRet = listen(sockfdServer, BACKLOG);
    if (nRet == -1) {
        herror("listen");
        exit(EXIT_FAILURE);
    }

    bzero((void *)&stSockAddrInClient, sizeof(struct sockaddr_in));
    clientLen = sizeof(struct sockaddr_in);
    printf("\nServer (%s: %u) is waiting.... \n", inet_ntoa(stSockAddrInServer.sin_addr), ntohs(stSockAddrInServer.sin_port));
    sockfdClient = accept(sockfdServer, (struct sockaddr *)&stSockAddrInClient, &clientLen);
    if (sockfdClient == -1) {
        herror("accept");
        exit(EXIT_FAILURE);
    }

    memset(strTemp, '\0', sizeof(char) * 32);
    nRet = recv(sockfdClient, (void *)strTemp, sizeof(char) * 32, 0);
    printf("\nServer recv %d bytes from client (%s: %u): %s (%lu) \n", \
        nRet, inet_ntoa(stSockAddrInClient.sin_addr), ntohs(stSockAddrInClient.sin_port), strTemp, strlen(strTemp));
    strcat(strTemp, "World!! ");
    nRet = send(sockfdClient, (void *)strTemp, sizeof(char) * 32, 0);
    printf("Server send %d bytes to client (%s: %u): %s (%lu) \n", \
        nRet, inet_ntoa(stSockAddrInClient.sin_addr), ntohs(stSockAddrInClient.sin_port), strTemp, strlen(strTemp));
    close(sockfdServer);
    close(sockfdClient);
    printf("Server done!! \n");

    return 0;
}
