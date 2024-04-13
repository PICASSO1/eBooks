/**
 * File Name: siocdarp.c
 *
 * CopyLeft (C) 2024, Picasso's Fantasy Notepad. All rights reserved.
 *
 * Author: Pablo Picasso G. (PabloPicasso.G@gmail.com)
 *
 * Version: 1.0.0.build041324
 *
 * Date: 2024 / 04 / 13
 *
 * Description: See siocgarp.c.
 *
(*)?*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <linux/sockios.h>
#include <net/if.h>
#include <net/if_arp.h>

#define IPADDR           "192.168.1.230"
#define INTERFACENAME    "enp0s3"

int main(void)
{
    int fd = -1, nRet = -1;
    struct arpreq arpr;

    bzero(&arpr, sizeof(struct arpreq));
    ((struct sockaddr_in *)&(arpr.arp_pa))->sin_family = AF_INET;
    inet_pton(AF_INET, IPADDR, &(((struct sockaddr_in *)&(arpr.arp_pa))->sin_addr));
    strncpy(arpr.arp_dev, INTERFACENAME, strlen(INTERFACENAME));
    arpr.arp_flags = ATF_PERM | ATF_COM;
    arpr.arp_ha.sa_data[0] = 0xAB;
    arpr.arp_ha.sa_data[1] = 0xCD;
    arpr.arp_ha.sa_data[2] = 0xEF;
    arpr.arp_ha.sa_data[3] = 0x01;
    arpr.arp_ha.sa_data[4] = 0x23;
    arpr.arp_ha.sa_data[5] = 0x45;

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    nRet = ioctl(fd, SIOCDARP, &arpr);
    if (nRet < 0) {
        perror("ioctl");
        close(fd);
        exit(EXIT_FAILURE);
    }
    close(fd);
  
    return 0;
}
