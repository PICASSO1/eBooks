#include "include.h"
#include "define.h"

int main(void)
{
    int sockfdClient = -1, sockfdServer = -1, nRet = -1;
    struct sockaddr_in stSockAddrInClient, stSockAddrInServer;
    socklen_t clientLen = 0, serverLen = 0;
    char strTemp[32];
    struct msghdr stMsgClient, stMsgServer;
    struct iovec stIoClient, stIoServer;
#if 1
    sockfdClient = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfdClient == -1) {
        herror("socket");
        exit(EXIT_FAILURE);
    }

    bzero((void *)&stSockAddrInClient, sizeof(struct sockaddr_in));
    stSockAddrInClient.sin_family = AF_INET;
    stSockAddrInClient.sin_addr.s_addr = inet_addr(IP_ADDRESS);
    stSockAddrInClient.sin_port = htons(PORT);
    clientLen = sizeof(struct sockaddr_in);

    memset(strTemp, '\0', sizeof(char) * 32);
    strcpy(strTemp, "Hello!! ");
	
    bzero((void *)&stIoClient, sizeof(struct iovec));
    stIoClient.iov_base = strTemp;
    stIoClient.iov_len = strlen(strTemp);

    bzero((void *)&stMsgClient, sizeof(struct msghdr));
    stMsgClient.msg_name = &stSockAddrInClient;
    stMsgClient.msg_namelen = sizeof(struct sockaddr_in);
    stMsgClient.msg_iov = &stIoClient;
    stMsgClient.msg_iovlen = 1;
    /* below 3 members ignore first .... */
/*  stMsgClient.msg_control = ;
    stMsgClient.msg_controllen = ;
    stMsgClient.msg_flags = ;
*/
    nRet = sendmsg(sockfdClient, &stMsgClient, 0);
    printf("Client (%s: %u) sendmsg %d bytes to server: %s (%lu) \n", \
        inet_ntoa(stSockAddrInClient.sin_addr), ntohs(stSockAddrInClient.sin_port), nRet, strTemp, strlen(strTemp));

    sleep(1);
#endif
#if 0
    sockfdClient = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfdClient == -1) {
        herror("socket");
        exit(EXIT_FAILURE);
    }

    bzero((void *)&stSockAddrInClient, sizeof(struct sockaddr_in));
    stSockAddrInClient.sin_family = AF_INET;
    stSockAddrInClient.sin_addr.s_addr = inet_addr(IP_ADDRESS);
    stSockAddrInClient.sin_port = htons(PORT);
    clientLen = sizeof(struct sockaddr_in);
    nRet = bind(sockfdClient, (struct sockaddr *)&stSockAddrInClient, clientLen);
    if (nRet == -1) {
        herror("bind");
        exit(EXIT_FAILURE);
    }

    memset(strTemp, '\0', sizeof(char) * 32);

    stIoClient.iov_base = strTemp;
    stIoClient.iov_len = sizeof(char) * 32;

    bzero((void *)&stSockAddrInServer, sizeof(struct sockaddr_in));
    serverLen = sizeof(struct sockaddr_in);

    bzero((void *)&stMsgClient, sizeof(struct msghdr));
    stMsgClient.msg_name = &stSockAddrInServer;
    stMsgClient.msg_namelen = serverLen;
    stMsgClient.msg_iov = &stIoClient;
    stMsgClient.msg_iovlen = 1;
    /* below 3 members ignore first .... */
/*  stMsgClient.msg_control = ;
    stMsgClient.msg_controllen = ;
    stMsgClient.msg_flags = ;
*/
    printf("\nClient (%s: %u) is waiting.... \n", inet_ntoa(stSockAddrInClient.sin_addr), ntohs(stSockAddrInClient.sin_port));
    while (1) {
        nRet = recvmsg(sockfdClient, &stMsgClient, 0);
        if (nRet == -1) {
            herror("recvmsg");
            continue;
        }
        printf("Client (%s: %u) recvmsg %d bytes from server: %s (%lu) \n", \
            inet_ntoa(stSockAddrInClient.sin_addr), ntohs(stSockAddrInClient.sin_port), nRet, strTemp, strlen(strTemp));
        break;
    }
#endif
    close(sockfdServer);
    close(sockfdClient);
    printf("Client done!! \n");

    return 0;
}
