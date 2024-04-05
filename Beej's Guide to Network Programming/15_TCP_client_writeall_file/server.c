#include "include.h"
#include "declare.h"
#include "define.h"

int main(void)
{
    int sockfdServer = -1, sockfdClient = -1;
    struct sockaddr_in stSockAddrInServer, stSockAddrInClient;
    socklen_t serverLen = 0, clientLen = 0;
    char strTemp[32];
    int nRet = -1, nFileSize = 0;
    FILE *fp = (FILE *)NULL;
	
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
        close(sockfdServer);
        exit(EXIT_FAILURE);
    }

    nRet = listen(sockfdServer, BACKLOG);
    if (nRet == -1) {
        herror("listen");
        close(sockfdServer);
        exit(EXIT_FAILURE);
    }

    fp = fopen("server.file", "wb");
    if (ferror(fp) != 0) {
        perror("fopen");
        fclose(fp);
        close(sockfdServer);
        exit(EXIT_FAILURE);
    }
	
    bzero((void *)&stSockAddrInClient, sizeof(struct sockaddr_in));
    clientLen = sizeof(struct sockaddr_in);
    printf("\nServer (%s: %u) is waiting.... \n", inet_ntoa(stSockAddrInServer.sin_addr), ntohs(stSockAddrInServer.sin_port));
    sockfdClient = accept(sockfdServer, (struct sockaddr *)&stSockAddrInClient, &clientLen);
    if (sockfdClient == -1) {
        herror("accept");
        fclose(fp);
        close(sockfdServer);
        exit(EXIT_FAILURE);
    }

    while (1) {
        memset(strTemp, '\0', sizeof(char) * 32);
        nRet = read(sockfdClient, (void *)strTemp, sizeof(char) * 32);
        if (nRet == 0)
            break;
        fwrite((void *)strTemp, sizeof(char), nRet, fp);
        nFileSize += nRet;
    }
    fclose(fp);

    printf("\nServer read %d bytes from client (%s: %u). \n", nFileSize, \
        inet_ntoa(stSockAddrInClient.sin_addr), ntohs(stSockAddrInClient.sin_port));
    close(sockfdServer);
    close(sockfdClient);
    printf("Server done!! \n");

    return 0;
}
