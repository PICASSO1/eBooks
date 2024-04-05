#include "include.h"
#include "declare.h"
#include "define.h"

int main(void)
{
    int sockfdClient = -1, nRet = -1, fd = -1, nFileSize = 0;
    struct sockaddr_in stSockAddrInClient;
    socklen_t clientLen = 0;
    char strTemp[32];

    sockfdClient = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfdClient == -1) {
        herror("socket");
        exit(EXIT_FAILURE);
    }

    bzero((void *)&stSockAddrInClient, sizeof(struct sockaddr_in));
    stSockAddrInClient.sin_family = AF_INET;
    stSockAddrInClient.sin_addr.s_addr = inet_addr(IP_ADDRESS);
    stSockAddrInClient.sin_port = htons(PORT);
    clientLen = sizeof(struct sockaddr_in);
    nRet = connect(sockfdClient, (struct sockaddr *)&stSockAddrInClient, clientLen);
    if (nRet == -1) {
        herror("connect");
        close(sockfdClient);
        exit(EXIT_FAILURE);
    }

    memset(strTemp, '\0', sizeof(char) * 32);
    strcpy(strTemp, "The client request .... ");
    nRet = sendall(sockfdClient, (void *)strTemp, sizeof(char) * 32, 0);

    fd = open("client.file", O_CREAT | O_WRONLY);
    if (fd == -1) {
        perror("open");
        close(sockfdClient);
        exit(EXIT_FAILURE);
    }

    while (1) {
        memset(strTemp, '\0', sizeof(char) * 32);
        nRet = recv(sockfdClient, (void *)strTemp, sizeof(char) * 32, 0);
        if (nRet == 0)
            break;
        write(fd, (void *)strTemp, nRet);
        nFileSize += nRet;
    }
    close(fd);

    printf("Client recv %d bytes from server (%s: %d) \n", nFileSize, \
        inet_ntoa(stSockAddrInClient.sin_addr), ntohs(stSockAddrInClient.sin_port));
    close(sockfdClient);
    printf("Client done!! \n");

    return 0;
}
