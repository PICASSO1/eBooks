#ifndef _DEFINE_H_
#define _DEFINE_H_

#define IP_ADDRESS    "127.0.0.1"
#define PORT          4096
#define BACKLOG       1

int 
writeall(sockfd, data, len)
int sockfd;
void *data;
size_t len;
{
    size_t unTotal = 0U;
    size_t unByteLeft = len;
    int nRet = 0;

    while (unTotal < len) {
        nRet = write(sockfd, data + unTotal, unByteLeft);
        if (nRet == -1)
            break;

        unTotal += nRet;
        unByteLeft -= nRet;
    }

    return (nRet == -1)? -1: unTotal;
}

#endif /* _DEFINE_H_ */
