/**
 * File Name: main.c
 *
 * CopyLeft (C) 2024, Picasso's Fantasy Notepad. All rights reserved.
 *
 * Author: Pablo Picasso G. (PabloPicasso.G@gmail.com)
 *
 * Version: 1.0.0.build102318
 *
 * Date: 2018 / 10 / 23
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
  unsigned short int uhnRet = 0U;
  /*
  IP Address: "192.168.1.1"
  Hex: 0xC0 (192), 0xA8 (168), 0x01 (1), 0x01 (1) --> 0xC0A80101
  Network Byte Order: C0 A8 01 01 會轉變為 01 01 A8 C0 --> 0x0101A8C0 = 16885952
  */
  ulnRet = htonl(0xC0A80101);
  printf("IP Address: %s (0xC0A80101); \nhtonl(0xC0A80101): 0x%08X \n\n", "192.168.1.1", ulnRet);
  /*
  Port Number: 2048
  Hex: 2048 = 0x0800 (08 00) 會轉變為 0x0008 (00 08) = 8 
  */
  uhnRet = htons(2048);
  printf("Port      : %s        (0x    0800); \nhtons(0x    0800): 0x%08X \n\n", "2048", uhnRet);
  /*
  Network Byte Order: 16885952 (0x0101A8C0)
  Change to: 0xC0A80101 (192.168.1.1)
  */
  ulnRet = ntohl(0x0101A8C0);
  printf("Network Byte Order: 0x0101A8C0; \nntohl(0x0101A8C0): 0x%08X \n\n", ulnRet);
  /*
  Network Byte Order: 8 (0x0008)
  Change to: 0x0800 (2048)
  */
  uhnRet = ntohs(8);
  printf("Network Byte Order: 0x00000008; \nntohs(0x00000008): 0x%08X \n", uhnRet);

  return 0;
}
