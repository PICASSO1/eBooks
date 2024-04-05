/**
 * File Name: main.c
 *
 * CopyLeft (C) 2024, Picasso's Fantasy Notepad. All rights reserved.
 *
 * Author: Pablo Picasso G. (PabloPicasso.G@gmail.com)
 *
 * Version: 1.0.0.build103118
 *
 * Date: 2018 / 10 / 31
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
    struct protoent *p_stProtoent0 = (struct protoent *)NULL;
    struct protoent *p_stProtoent1 = (struct protoent *)NULL;
    struct protoent *p_stProtoent2 = (struct protoent *)NULL;
    int nIdx = -1;

    p_stProtoent0 = getprotobyname("l2tp");
    if (p_stProtoent0 == (struct protoent *)NULL) {
        herror("getprotobyname");
        return 1;
    }

    printf("getprotobyname(\"l2tp\"): \n");
    printf("Protocol Name  : %s \n", p_stProtoent0->p_name);
    nIdx = 0;
    printf("Alias List     : ");
    while (p_stProtoent0->p_aliases[nIdx] != (char *)NULL) {
        printf("%s \n", p_stProtoent0->p_aliases[nIdx++]);
    }
    printf("Protocol Number: %d \n", p_stProtoent0->p_proto);
    printf("\n");

    p_stProtoent1 = getprotobynumber(89);
    if (p_stProtoent1 == (struct protoent *)NULL) {
        herror("getprotobynumber");
        return 1;
    }

    printf("getprotobynumber(89): \n");
    printf("Protocol Name  : %s \n", p_stProtoent1->p_name);
    nIdx = 0;
    printf("Alias List     : ");
    while (p_stProtoent1->p_aliases[nIdx] != (char *)NULL) {
        printf("%s \n", p_stProtoent1->p_aliases[nIdx++]);
    }
    printf("Protocol Number: %d \n", p_stProtoent1->p_proto);
    printf("\n");

    printf("getprotoent(): \n");
    while (p_stProtoent2 = getprotoent()) {
        printf("%-16s %-4d %-16s \n", p_stProtoent2->p_name, p_stProtoent2->p_proto, p_stProtoent2->p_aliases[0]);
    }
    endprotoent();

    return 0;
}
