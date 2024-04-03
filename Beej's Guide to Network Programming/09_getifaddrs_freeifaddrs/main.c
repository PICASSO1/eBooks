/**
 * File Name: main.c
 *
 * CopyLeft (C) 2024, Picasso's Fantasy Notepad. All rights reserved.
 *
 * Author: Pablo Picasso G. (PabloPicasso.G@gmail.com)
 *
 * Version: 1.0.0.build111518
 *
 * Date: 2018 / 11 / 15
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
#include <ifaddrs.h>

int main(void)
{
    struct ifaddrs *stIfaddrs = (struct ifaddrs *)NULL;
    struct ifaddrs *pNext = (struct ifaddrs *)NULL;
    int nRet = -1, i = -1;
    struct sockaddr_in *pSockAddr_In4 = (struct sockaddr_in *)NULL;
    char strIPv4Addr[INET_ADDRSTRLEN];

    nRet = getifaddrs(&stIfaddrs);
    if (nRet == -1) {
        herror("getifaddrs");
        exit(EXIT_FAILURE);
    }

    for (pNext = stIfaddrs; pNext != (struct ifaddrs *)NULL; pNext = pNext->ifa_next) {
        if (pNext->ifa_addr->sa_family == AF_INET) {
            printf("IPv4 Interface (AF_INET, %d): \n", pNext->ifa_addr->sa_family);
            /* Interface Name */
            printf("pNext->ifa_name: %s \n", pNext->ifa_name);
            printf("pNext->ifa_flags: %d \n", pNext->ifa_flags);
            /* Interface Address */
            printf("IPv4 Address: \n");
            pSockAddr_In4 = (struct sockaddr_in *)pNext->ifa_addr;
            printf("pSockAddr_In4->sin_family: %d \n", pSockAddr_In4->sin_family);
            printf("pSockAddr_In4->sin_port: %d \n", ntohs(pSockAddr_In4->sin_port));
            memset(strIPv4Addr, '\0', sizeof(char) * INET_ADDRSTRLEN);
            inet_ntop(AF_INET, (void *)&pSockAddr_In4->sin_addr, strIPv4Addr, sizeof(char) * INET_ADDRSTRLEN);
            printf("pSockAddr_In4->sin_addr: %s \n", strIPv4Addr);
            /* Interface Netmask */
            printf("IPv4 Address Netmask: \n");
            pSockAddr_In4 = (struct sockaddr_in *)pNext->ifa_netmask;
            printf("pSockAddr_In4->sin_family: %d \n", pSockAddr_In4->sin_family);
            printf("pSockAddr_In4->sin_port: %d \n", ntohs(pSockAddr_In4->sin_port));
            memset(strIPv4Addr, '\0', sizeof(char) * INET_ADDRSTRLEN);
            inet_ntop(AF_INET, (void *)&pSockAddr_In4->sin_addr, strIPv4Addr, sizeof(char) * INET_ADDRSTRLEN);
            printf("pSockAddr_In->sin_addr: %s \n", strIPv4Addr);
            /* Interface Broadcast */
            printf("IPv4 Broadcast Address: \n");
            pSockAddr_In4 = (struct sockaddr_in *)pNext->ifa_ifu.ifu_broadaddr;
        /*  pSockAddr_In4 = (struct sockaddr_in *)pNext->ifa_ifu.ifu_dstaddr; */
            printf("pSockAddr_In4->sin_family: %d \n", pSockAddr_In4->sin_family);
            printf("pSockAddr_In4->sin_port: %d \n", ntohs(pSockAddr_In4->sin_port));
            memset(strIPv4Addr, '\0', sizeof(char) * INET_ADDRSTRLEN);
            inet_ntop(AF_INET, (void *)&pSockAddr_In4->sin_addr, strIPv4Addr, sizeof(char) * INET_ADDRSTRLEN);
            printf("pSockAddr_In4->sin_addr: %s \n", strIPv4Addr);
            /* Address-specific data (may be unused). */
            nRet = sizeof(pNext->ifa_data);
            printf("pNext->ifa_data:");
            for (i = 0; i < nRet; i++)
                printf(" %02X", (pNext->ifa_data) + i);
            printf(" (%d) \n", nRet);
        }
        else if (pNext->ifa_addr->sa_family == AF_INET6) { }
        else if (pNext->ifa_addr->sa_family == AF_PACKET) { }
        else { }
        printf("\n");
    }
    freeifaddrs((void *)stIfaddrs);

    return 0;
}
