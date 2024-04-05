/**
 * File Name: main.c
 *
 * CopyLeft (C) 2024, Picasso's Fantasy Notepad. All rights reserved.
 *
 * Author: Pablo Picasso G. (PabloPicasso.G@gmail.com)
 *
 * Version: 1.0.0.build102718
 *
 * Date: 2018 / 10 / 27
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
    struct servent *p_stServent0 = (struct servent *)NULL;
    struct servent *p_stServent1 = (struct servent *)NULL;
    struct servent *p_stServent2 = (struct servent *)NULL;
    int nIdx = -1;

    p_stServent0 = getservbyname("telnet", "tcp");
    if (p_stServent0 == (struct servent *)NULL) {
        herror("getservbtname");
        return 1;
    }

    printf("getservbyname(\"telnet\", \"tcp\"): \n");
    printf("Service Name: %s \n", p_stServent0->s_name);
    nIdx = 0;
    printf("Alias List  : \n");
    while (p_stServent0->s_aliases[nIdx] != (char *)NULL) {
        printf("%s \n", p_stServent0->s_aliases[nIdx++]);
    }
    printf("Port Number : %d \n", ntohs(p_stServent0->s_port));
    printf("Protocol    : %s \n", p_stServent0->s_proto);
    printf("\n");

    p_stServent1 = getservbyport(htons(69), "udp");
    if (p_stServent1 == (struct servent *)NULL) {
        herror("getservbyport");
        return 1;
    }

    printf("getservbyport(htons(69), \"udp\"): \n");
    printf("Service Name: %s \n", p_stServent1->s_name);
    nIdx = 0;
    printf("Alias List  : \n");
    while (p_stServent1->s_aliases[nIdx] != (char *)NULL) {
        printf("%s \n", p_stServent1->s_aliases[nIdx++]);
    }
    printf("Port Number : %d \n", ntohs(p_stServent1->s_port));
    printf("Protocol    : %s \n", p_stServent1->s_proto);
    printf("\n");

    printf("getservent(): \n");
    while (p_stServent2 = getservent()) {
        printf("%-16s, %d/%s \n", p_stServent2->s_name, ntohs(p_stServent2->s_port), p_stServent2->s_proto);
    }
    endservent();

    return 0;
}
