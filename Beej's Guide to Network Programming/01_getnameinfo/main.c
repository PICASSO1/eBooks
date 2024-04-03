/**
 * File Name: main.c
 *
 * CopyLeft (C) 2024, Picasso's Fantasy Notepad. All rights reserved.
 *
 * Author: Pablo Picasso G. (PabloPicasso.G@gmail.com)
 *
 * Version: 1.0.0.build110418
 *
 * Date: 2018 / 11 / 04
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
    struct sockaddr_in stSockAddrIn;
    char strHostName[32], strServName[32];
    int nRet = -1;

    bzero((void *)&stSockAddrIn, sizeof(struct sockaddr_in));
    inet_pton(AF_INET, "127.0.0.1", &stSockAddrIn.sin_addr);
    stSockAddrIn.sin_family = AF_INET;
    stSockAddrIn.sin_port = htons(23);

    memset(strHostName, '\0', sizeof(char) * 32);
    memset(strServName, '\0', sizeof(char) * 32);
    nRet = getnameinfo((struct sockaddr *)&stSockAddrIn, sizeof(struct sockaddr), strHostName, sizeof(char) * 32, strServName, sizeof(char) * 32, NI_NOFQDN);
    if (nRet != 0) {
        fprintf(stderr, "getaddrinfo() error: %s \n", gai_strerror(nRet));
        exit(EXIT_FAILURE);
    }

    printf("strHostName: %s (%lu) \n", strHostName, strlen(strHostName));
    printf("strServName: %s (%lu) \n", strServName, strlen(strServName));

    return 0;
}