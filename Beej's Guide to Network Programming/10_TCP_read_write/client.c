#include "include.h"
#include "define.h"

int main(void)
{
    int sockfdClient = -1, nRet = -1;
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
        exit(EXIT_FAILURE);
    }

    memset(strTemp, '\0', sizeof(char) * 32);
    strcpy(strTemp, "Hello!! ");
	nRet = write(sockfdClient, (void *)strTemp, sizeof(char) * 32);
    printf("Client (%s: %u) write %d bytes to server: %s (%lu) \n", \
        inet_ntoa(stSockAddrInClient.sin_addr), ntohs(stSockAddrInClient.sin_port),  nRet, strTemp, strlen(strTemp));
    sleep(1);
    nRet = read(sockfdClient, (void *)strTemp, sizeof(char) * 32);
    printf("Client (%s: %u) read %d bytes from server: %s (%lu) \n", \
        inet_ntoa(stSockAddrInClient.sin_addr), ntohs(stSockAddrInClient.sin_port),  nRet, strTemp, strlen(strTemp));
    close(sockfdClient);
    printf("Client done!! \n");

    return 0;
}
