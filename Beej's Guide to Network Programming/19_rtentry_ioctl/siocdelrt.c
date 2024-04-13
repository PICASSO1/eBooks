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
#include <net/route.h>

int 
main(argc, argv, envp)
int argc;
char *argv[];
char **envp;
{
    printf("SIOCDELRT!! \n");

    return 0;
}
