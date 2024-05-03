#include "include.h"
#include "define.h"

int main(void)
{
    int sockfdServer = -1, sockfdClient = -1, nRet = -1;
    struct sockaddr_in stSockAddrInServer, stSockAddrInClient;
    socklen_t serverLen = 0, clientLen = 0;
    char strTemp[32];
    struct msghdr stMsgServer, stMsgClient;
    struct iovec stIoServer, stIoClient;
#if 1
    sockfdServer = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfdServer == -1) {
        herror("socket");
        exit(EXIT_FAILURE);
    }
	
    bzero((void *)&stSockAddrInServer, sizeof(struct sockaddr_in));
    stSockAddrInServer.sin_family = AF_INET;
    stSockAddrInServer.sin_addr.s_addr = inet_addr(IP_ADDRESS);
    stSockAddrInServer.sin_port = htons(PORT);
    serverLen = sizeof(struct sockaddr_in);
    nRet = bind(sockfdServer, (struct sockaddr *)&stSockAddrInServer, serverLen);
    if (nRet == -1) {
        herror("bind");
        exit(EXIT_FAILURE);
    }

    memset(strTemp, '\0', sizeof(char) * 32);

    stIoServer.iov_base = strTemp;
    stIoServer.iov_len = sizeof(char) * 32;

    bzero((void *)&stSockAddrInClient, sizeof(struct sockaddr_in));
    clientLen = sizeof(struct sockaddr_in);

    bzero((void *)&stMsgServer, sizeof(struct msghdr));
    stMsgServer.msg_name = &stSockAddrInClient;
    stMsgServer.msg_namelen = clientLen;
    stMsgServer.msg_iov = &stIoServer;
    stMsgServer.msg_iovlen = 1;
    /* below 3 members ignore first .... */
/*  stMsgServer.msg_control = ;
    stMsgServer.msg_controllen = ;
    stMsgServer.msg_flags = ;
*/
    printf("\nServer (%s: %u) is waiting.... \n", inet_ntoa(stSockAddrInServer.sin_addr), ntohs(stSockAddrInServer.sin_port));
    while (1) {
        nRet = recvmsg(sockfdServer, &stMsgServer, 0);
        if (nRet == -1) {
            herror("recvmsg");
            continue;
        }

        printf("\nServer recvmsg %d bytes from client (%s: %u): %s (%lu) \n", \
            nRet, inet_ntoa(stSockAddrInClient.sin_addr), ntohs(stSockAddrInClient.sin_port), strTemp, strlen(strTemp));
        break;
    }
#endif
#if 0
    sockfdServer = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfdServer == -1) {
        herror("socket");
        exit(EXIT_FAILURE);
    }

    bzero((void *)&stSockAddrInServer, sizeof(struct sockaddr_in));
    stSockAddrInServer.sin_family = AF_INET;
    stSockAddrInServer.sin_addr.s_addr = inet_addr(IP_ADDRESS);
    stSockAddrInServer.sin_port = htons(PORT);
    serverLen = sizeof(struct sockaddr_in);

    memset(strTemp, '\0', sizeof(char) * 32);
    strcpy(strTemp, "World!! ");

    bzero((void *)&stIoServer, sizeof(struct iovec));
    stIoServer.iov_base = strTemp;
    stIoServer.iov_len = strlen(strTemp);

    bzero((void *)&stMsgServer, sizeof(struct msghdr));
    stMsgServer.msg_name = &stSockAddrInServer;
    stMsgServer.msg_namelen = sizeof(struct sockaddr_in);
    stMsgServer.msg_iov = &stIoServer;
    stMsgServer.msg_iovlen = 1;
    /* below 3 members ignore first .... */
/*  stMsgServer.msg_control = ;
    stMsgServer.msg_controllen = ;
    stMsgServer.msg_flags = ;
*/
    nRet = sendmsg(sockfdServer, &stMsgServer, 0);
    printf("Server (%s: %u) sendmsg %d bytes to client: %s (%lu) \n", \
        inet_ntoa(stSockAddrInServer.sin_addr), ntohs(stSockAddrInServer.sin_port), nRet, strTemp, strlen(strTemp));
#endif
    close(sockfdServer);
    close(sockfdClient);
    printf("Server done!! \n");

    return 0;
}
