/**
 * File Name: main.c
 *
 * CopyLeft (C) 2024, Picasso's Fantasy Notepad. All rights reserved.
 *
 * Author: Pablo Picasso G. (PabloPicasso.G@gmail.com)
 *
 * Version: 1.0.0.build110218
 *
 * Date: 2018 / 11 / 02
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
    int nStatus = 0, i = -1;
    struct addrinfo stAddrInfo;
    struct addrinfo *servinfo = (struct addrinfo *)NULL;
    struct addrinfo *pNext = (struct addrinfo *)NULL;
    struct sockaddr_in *pSockAddrIn = (struct sockaddr_in *)NULL;
    struct sockaddr_in6 *pSockAddrIn6 = (struct sockaddr_in6 *)NULL;
    char strIPv4Address[INET_ADDRSTRLEN];
    char strIPv6Address[INET6_ADDRSTRLEN];

    bzero((void *)&stAddrInfo, sizeof(struct addrinfo));
    stAddrInfo.ai_family = AF_UNSPEC;
    stAddrInfo.ai_socktype = SOCK_STREAM;
    stAddrInfo.ai_flags = AI_PASSIVE;

    nStatus = getaddrinfo("localhost", "8080", &stAddrInfo, &servinfo);
    if (nStatus == -1) {
        fprintf(stderr, "getaddrinfo() error: %s \n", gai_strerror(nStatus));
        exit(EXIT_FAILURE);
    }

    for (pNext = servinfo; pNext != (struct addrinfo *)NULL; pNext = pNext->ai_next) {
        printf("pNext->ai_flags = %d \n", pNext->ai_flags);
        printf("pNext->ai_family = %d \n", pNext->ai_family);
        printf("pNext->ai_socktype = %d \n", pNext->ai_socktype);
        printf("pNext->ai_protocol = %d \n", pNext->ai_protocol);
        printf("pNext->ai_addrlen = %d \n", (int)pNext->ai_addrlen);
        if (pNext->ai_family == AF_INET) {
            printf("pSockAddrIn = (struct sockaddr_in *)pNext->ai_addr; \n");
            pSockAddrIn = (struct sockaddr_in *)pNext->ai_addr;
            printf("pSockAddrIn->sin_family = %d \n", pSockAddrIn->sin_family);
            printf("pSockAddrIn->sin_port = %u \n", ntohs(pSockAddrIn->sin_port));
            memset(strIPv4Address, '\0', sizeof(char) * INET_ADDRSTRLEN);
            inet_ntop(pNext->ai_family, (void *)&pSockAddrIn->sin_addr, strIPv4Address, sizeof(char) * INET_ADDRSTRLEN);
            printf("pSockAddrIn->sin_addr = %s \n", strIPv4Address);
        }
        else if (pNext->ai_family == AF_INET6) {
            printf("pSockAddrIn6 = (struct sockaddr_in6 *)pNext->ai_addr; \n");
            pSockAddrIn6 = (struct sockaddr_in6 *)pNext->ai_addr;
            printf("pSockAddrIn6->sin6_family = %d \n", pSockAddrIn6->sin6_family);
            printf("pSockAddrIn6->sin6_port = %u \n", ntohs(pSockAddrIn6->sin6_port));
            printf("pSockAddrIn6->sin6_flowinfo = %d \n", pSockAddrIn6->sin6_flowinfo);
            memset(strIPv6Address, '\0', sizeof(char) * INET6_ADDRSTRLEN);
            inet_ntop(pNext->ai_family, (void *)&pSockAddrIn6->sin6_addr, strIPv6Address, sizeof(char) * INET6_ADDRSTRLEN);
            printf("pSockAddrIn6->sin6_addr = %s \n", strIPv6Address);
            printf("pSockAddrIn6->sin6_scope_id = %d \n", pSockAddrIn6->sin6_scope_id);
        }
        else { }
        printf("pNext->ai_canonname: %s \n", pNext->ai_canonname);
        printf("\n");
    }
    freeaddrinfo((void *)servinfo);

    return 0;
}
