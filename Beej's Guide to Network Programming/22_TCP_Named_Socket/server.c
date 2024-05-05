#include "include.h"
#include "define.h"

int main(void)
{
    int sockfdServer = -1, sockfdClient = -1, nRet = -1;
    struct sockaddr_un stSockAddrInServer, stSockAddrInClient;
    socklen_t serverLen = 0, clientLen = 0;
    char strTemp[32];

    sockfdServer = socket(AF_LOCAL, SOCK_STREAM, 0);
    if (sockfdServer == -1) {
        herror("socket");
        exit(EXIT_FAILURE);
    }
	
    stSockAddrInServer.sun_family = AF_LOCAL;
    strcpy(stSockAddrInServer.sun_path, "private.sock");
    serverLen = sizeof(struct sockaddr_un);
    unlink("private.sock");
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

    bzero((void *)&stSockAddrInClient, sizeof(struct sockaddr_un));
    clientLen = sizeof(struct sockaddr_un);
    printf("Server is waiting.... \n");
    sockfdClient = accept(sockfdServer, (struct sockaddr *)&stSockAddrInClient, &clientLen);
    if (sockfdClient == -1) {
        herror("accept");
        exit(EXIT_FAILURE);
    }

    memset(strTemp, '\0', sizeof(char) * 32);
    nRet = read(sockfdClient, (void *)strTemp, sizeof(char) * 32);
    printf("Server read %d bytes from client: %s (%lu) \n", nRet, strTemp, strlen(strTemp));

    strcat(strTemp, "World!! ");
    nRet = write(sockfdClient, (void *)strTemp, strlen(strTemp));
    printf("Server write %d bytes to client: %s (%lu) \n", nRet, strTemp, strlen(strTemp));

    close(sockfdClient);
    unlink("private.sock");
    close(sockfdServer);
    printf("Server done!! \n");

	return 0;
}
