/**
 * File Name: main.c
 *
 * CopyLeft (C) 2024, Picasso's Fantasy Notepad. All rights reserved.
 *
 * Author: Pablo Picasso G. (PabloPicasso.G@gmail.com)
 *
 * Version: 1.0.0.build102118
 *
 * Date: 2018 / 10 / 21
 *
 * Description: See NOTE.TXT. 
 *
(*)?*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>

int main(void)
{
    unsigned long int ulnRet = 0UL;
    int nRet = -1;
    struct in_addr stInAddr;
    char *ptr = (char *)NULL;
    char strIPv4Address[INET_ADDRSTRLEN];

    ulnRet = inet_addr("192.168.1.1");
    if (ulnRet == -1) {
        herror("inet_addr");
        exit(EXIT_FAILURE);
    }
    printf("IPv4 inet_addr(\"192.168.1.1\") = 0x%08X (%lu) \n\n", ulnRet, ulnRet);

    bzero((void *)&stInAddr, sizeof(struct in_addr));
    nRet = inet_aton("192.168.1.1", &stInAddr);
    if (nRet == -1) {
        herror("inet_aton");
        exit(EXIT_FAILURE);
    }
    printf("IPv4 inet_aton(): nRet = %d; stInAddr.s_addr = 0x%08X (%u) \n\n", nRet, stInAddr.s_addr, stInAddr.s_addr);

    ptr = inet_ntoa(stInAddr);
    if (ptr == (char *)NULL) {
        herror("inet_ntoa");
        exit(EXIT_FAILURE);
    }
    printf("IPv4 inet_ntoa(): %s (%lu);  \n\n", ptr, strlen (ptr));

    bzero((void *)&stInAddr, sizeof(struct in_addr));
    nRet = inet_pton(AF_INET, "127.0.0.1", &stInAddr);
    if (nRet == -1) {
        herror("inet_pton");
        exit(EXIT_FAILURE);
    }
    printf("IPv4 inet_pton(): 0x%08X (%d) \n\n", stInAddr.s_addr, stInAddr.s_addr);

    memset(strIPv4Address, '\0', sizeof(char) * INET_ADDRSTRLEN);
    ptr = inet_ntop(AF_INET, (void *)&stInAddr, strIPv4Address, sizeof(char) * INET_ADDRSTRLEN);
    if (ptr == (char *)NULL){
        herror("inet_ntop");
        exit(EXIT_FAILURE);
    }
    printf("IPv4 inet_ntop(): ptr = %s (%lu); strIPv4Address: %s (%lu) \n\n", ptr, strlen(ptr), strIPv4Address, strlen(strIPv4Address));

    return 0;
}
