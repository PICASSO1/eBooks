/**
 * File Name: gethostbyname.c
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

    stHost = gethostbyname("localhost");
    if (stHost == (struct hostent *)NULL) {
        herror("gethostbyname");
        return 1;
    }

    printf("gethostbyname(): \n");
    printf("stHost->h_name: %s \n", stHost->h_name);
    printf("\n");

    for (idx = 0; ; idx++) {
        printf("stHost->h_aliases: %s \n", stHost->h_aliases[idx]);
        if (stHost->h_aliases[idx] == (char *)NULL)
            break;
        else
            idx++;
    }
    printf("\n");

    printf("stHost->h_addrtype = %d \n", stHost->h_addrtype);
    /* Address Type為AF_INET, AF_INET6 */
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
    printf("IPv4 Address: %s \n", inet_ntoa(stTempInAddr));

    return 0;
}
