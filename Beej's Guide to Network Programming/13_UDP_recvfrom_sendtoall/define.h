#ifndef _DEFINE_H_
#define _DEFINE_H_

#define IP_ADDRESS    "127.0.0.1"
#define PORT          4096
#define BACKLOG       1

int 
sendtoall(sockfd, data, len, flags, to, tolen)
int sockfd;
const void *data;
int len;
unsigned int flags;
const struct sockaddr *to;
int tolen;
{
    size_t unTotal = 0U;
    size_t unByteLeft = len;
    int nRet = 0;

    while (unTotal < len) {
        nRet = sendto(sockfd, data + unTotal, unByteLeft, flags, to, tolen);
        if (nRet == -1)
            break;

        unTotal += nRet;
        unByteLeft -= nRet;
    }

    return (nRet == -1)? -1: unTotal;
}

#endif /* _DEFINE_H_ */
