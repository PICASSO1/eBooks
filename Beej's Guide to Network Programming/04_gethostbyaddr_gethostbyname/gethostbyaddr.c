/**
 * File Name: gethostbyaddr.c
 *
 * CopyLeft (C) 2024, Picasso's Fantasy Notepad. All rights reserved.
 *
 * Author: Pablo Picasso G. (PabloPicasso.G@gmail.com)
 *
 * Version: 1.0.0.build102918
 *
 * Date: 2018 / 10 / 29
 *
 * Description: See NOTE.TXT. 
 *
(*)?*/

#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <netdb.h>

int main(void)
{
    struct hostent *stHost = (struct hostent *)NULL;
    struct in_addr stInAddr, stTempInAddr;
    int nRet = 0, idx = -1;
    struct sockaddr_in stSkAddr;

    bzero((void *)&stInAddr, sizeof(struct in_addr));
    nRet = inet_aton("127.0.0.1", &stInAddr);

    stHost = gethostbyaddr(&stInAddr, sizeof(struct in_addr), AF_INET);
    if (stHost == (struct hostent *)NULL) {
        herror("gethostbyaddr");
        return 1;
    }

    printf("gethostbyaddr(): \n");
    printf("stHost->h_name: %s \n", stHost->h_name);
    printf("\n");

    idx = 0;
    do {
        printf("stHost->h_aliases: %s \n", stHost->h_aliases[idx++]);
    } while (stHost->h_aliases[idx] != (char *)NULL);
    printf("\n");

    printf("stHost->h_addrtype = %d \n", stHost->h_addrtype);
    printf("stHost->h_length = %d \n", stHost->h_length);
    printf("\n");

    idx = 0;
    do {
        printf("stHost->h_addr_list: %s \n", stHost->h_addr_list[idx++]);
    } while (stHost->h_addr_list[idx] != (char *)NULL);
    /* 在 <netdb.h> 中有定義 h_addr 就是 h_addr_list[0]!! */
    bzero((void *)&stTempInAddr, sizeof(struct in_addr));
    bzero((void *)&stSkAddr, sizeof(struct sockaddr_in));
    memcpy(&stSkAddr.sin_addr.s_addr, stHost->h_addr, 4);
    stTempInAddr.s_addr = stSkAddr.sin_addr.s_addr;
    printf("IP Address: %s \n", inet_ntoa(stTempInAddr));

    return 0;
}
