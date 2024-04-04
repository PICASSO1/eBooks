/**
 * File Name: main.c
 *
 * CopyLeft (C) 2024, Picasso's Fantasy Notepad. All rights reserved.
 *
 * Author: Pablo Picasso G. (PabloPicasso.G@gmail.com)
 *
 * Version: 1.0.0.build102518
 *
 * Date: 2018 / 10 / 25
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
    int nRet = -1;
    char strHostName[32];

    memset(strHostName, '\0', sizeof(char) * 32);
    nRet = gethostname(strHostName, sizeof(char) * 32);
    if (nRet == -1) {
        herror("gethostname");
        exit(EXIT_FAILURE);
    }
    printf("gethostname(): strHostName: %s (%lu) \n", strHostName, strlen(strHostName));

    return 0;
}
