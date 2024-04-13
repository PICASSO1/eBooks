/**
 * File Name: siocgarp.c
 *
 * CopyLeft (C) 2024, Picasso's Fantasy Notepad. All rights reserved.
 *
 * Author: Pablo Picasso G. (PabloPicasso.G@gmail.com)
 *
 * Version: 1.0.0.build041324
 *
 * Date: 2024 / 04 / 13
 *
 * Description:
 *
 *   - 這個範例呈現了 struct arpreq - ioctl(); 的一些基本用法；struct arpreq 的結構定義在：/usr/include/net/if_arp.h
 *   - ARP 相對應於 SIOC 的 Magic ID 有：SIOCDARP, SIOCGARP, SIOCSARP
 *   - RARP 相對應於 SIOC 的 Magic ID 有：SIOCDARP, SIOCGARP, SIOCSARP
 *   - 但 RARP 必須要有 kernel 的支援：rarp -a: "This kernel does not support RARP. "
 *   - Get 只需要３個參數：AF_INET、IPv4 Address、Interface Name
 *   - 但 Set / Delete 還必須多加上２個：Flags 和 MAC Address
 *   - 相對應於 "arp" 和 "cat -n /proc/net/arp"，SIOCGARP 所有的參數均能夠透過 ioctl(); 來取得。
 *   - 操作 "SIOC S/G/D (R)ARP" 必須使用 root 權限。
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

void display_IFENCAP(unsigned short *);

int main(void)
{
    int fd = -1, nRet = -1;
    struct arpreq arpr;

    bzero(&arpr, sizeof(struct arpreq));
    ((struct sockaddr_in *)&(arpr.arp_pa))->sin_family = AF_INET;
    inet_pton(AF_INET, IPADDR, &(((struct sockaddr_in *)&(arpr.arp_pa))->sin_addr));
    strncpy(arpr.arp_dev, INTERFACENAME, strlen(INTERFACENAME));

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    nRet = ioctl(fd, SIOCGARP, &arpr);
    if (nRet < 0) {
        perror("ioctl");
        close(fd);
        exit(EXIT_FAILURE);
    }

    printf("SIOCGARP (HWaddress): %02X:%02X:%02X:%02X:%02X:%02X ", \
        (unsigned char)arpr.arp_ha.sa_data[0], (unsigned char)arpr.arp_ha.sa_data[1], \
        (unsigned char)arpr.arp_ha.sa_data[2], (unsigned char)arpr.arp_ha.sa_data[3], \
        (unsigned char)arpr.arp_ha.sa_data[4], (unsigned char)arpr.arp_ha.sa_data[5]);

    display_IFENCAP(&arpr.arp_ha.sa_family);

    printf("Flags Information: 0x%02X \n", arpr.arp_flags);
    if (arpr.arp_flags & ATF_COM)            printf("Completed entry (ha valid) (0x%02X). \n", ATF_COM);
    if (arpr.arp_flags & ATF_PERM)           printf("Permanent entry (0x%02X). \n", ATF_PERM);
    if (arpr.arp_flags & ATF_PUBL)           printf("Publish entry (0x%02X). \n", ATF_PUBL);
    if (arpr.arp_flags & ATF_USETRAILERS)    printf("Has requested trailers (0x%02X). \n", ATF_USETRAILERS);
    if (arpr.arp_flags & ATF_NETMASK)        printf("Want to use a netmask (only for proxy entries) (0x%02X). \n", ATF_NETMASK);
    if (arpr.arp_flags & ATF_DONTPUB)        printf("Don't answer this addresses (0x%02X). \n", ATF_DONTPUB);
    if (arpr.arp_flags & ATF_MAGIC)          printf("Automatically added entry (0x%02X). \n", ATF_MAGIC);

    close(fd);
  
    return 0;
}

void 
display_IFENCAP(HWType)
unsigned short *HWType;
{
    printf("(");

    switch (*HWType) {
        /* ARP protocol HARDWARE identifiers. */
        case ARPHRD_NETROM:                printf("Net/Rom");                    break;    /* From KA9Q: NET/ROM pseudo. */
        case ARPHRD_ETHER:                 printf("Ethernet");                   break;    /* Ethernet 10/100Mbps. */
        case ARPHRD_EETHER:                printf("Experimental Ethernet");      break;    /* Experimental Ethernet. */
        case ARPHRD_AX25:                  printf("AX.25 Level 2");              break;    /* AX.25 Level 2. */
        case ARPHRD_PRONET:                printf("PRONet Token Ring");          break;    /* PROnet token ring. */
        case ARPHRD_CHAOS:                 printf("ChaosNet");                   break;    /* Chaosnet. */
        case ARPHRD_IEEE802:               printf("IEEE 802.2");                 break;    /* IEEE 802.2 Ethernet/TR/TB. */
        case ARPHRD_ARCNET:                printf("ARCNet");                     break;    /* ARCnet. */
        case ARPHRD_APPLETLK:              printf("Apple Talk");                 break;    /* APPLEtalk. */
        case ARPHRD_DLCI:                  printf("Frame Relay DLCI");           break;    /* Frame Relay DLCI. */
        case ARPHRD_ATM:                   printf("ATM");                        break;    /* ATM. */
        case ARPHRD_METRICOM:              printf("STRIP");                      break;    /* Metricom STRIP (new IANA id). */
        case ARPHRD_IEEE1394:              printf("IEEE 1394 IPv4");             break;    /* IEEE 1394 IPv4 - RFC 2734. */
        case ARPHRD_EUI64:                 printf("EUI-64");                     break;    /* EUI-64. */
        case ARPHRD_INFINIBAND:            printf("InfiniBand");                 break;    /* InfiniBand. */
        /* Dummy types for non ARP hardware. */
        case ARPHRD_SLIP:                  printf("SLIP");                       break;
        case ARPHRD_CSLIP:                 printf("CSLIP");                      break;
        case ARPHRD_SLIP6:                 printf("SLIP6");                      break;
        case ARPHRD_CSLIP6:                printf("CSLIP6");                     break;
        case ARPHRD_RSRVD:                 printf("Reserved");                   break;    /* Notional KISS type. */
        case ARPHRD_ADAPT:                 printf("ADAPT");                      break;
        case ARPHRD_ROSE:                  printf("ROSE");                       break;
        case ARPHRD_X25:                   printf("CCITT X.25");                 break;    /* CCITT X.25. */
        case ARPHRD_HWX25:                 printf("HW X.25");                    break;    /* Boards with X.25 in firmware. */
        case ARPHRD_CAN:                   printf("Controller Area Network");    break;    /* Controller Area Network. */
        case ARPHRD_MCTP:                  printf("MCTP");                       break;    
        case ARPHRD_PPP:                   printf("PPP");                        break;    
        case ARPHRD_CISCO:                 printf("Cisco HDLC");                 break;    /* Cisco HDLC. */
    /*  case ARPHRD_HDLC:                  printf("Cisco HDLC");                 break;    */
        case ARPHRD_LAPB:                  printf("LAPB");                       break;    /* LAPB. */
        case ARPHRD_DDCMP:                 printf("DDCMP");                      break;    /* Digital's DDCMP. */
        case ARPHRD_RAWHDLC:               printf("RawHDLC");                    break;    /* Raw HDLC. */
        case ARPHRD_RAWIP:                 printf("RawIP");                      break;    /* Raw IP. */

        case ARPHRD_TUNNEL:                printf("IPIP Tunnel");                break;    /* IPIP tunnel. */
        case ARPHRD_TUNNEL6:               printf("IPIP6 Tunnel");               break;    /* IPIP6 tunnel. */
        case ARPHRD_FRAD:                  printf("Frame Relay");                break;    /* Frame Relay Access Device. */
        case ARPHRD_SKIP:                  printf("SKIP");                       break;    /* SKIP vif. */
        case ARPHRD_LOOPBACK:              printf("Loopback");                   break;    /* Loopback device. */
        case ARPHRD_LOCALTLK:              printf("Localtalk");                  break;    /* Localtalk device. */
        case ARPHRD_FDDI:                  printf("Fiber");                      break;    /* Fiber Distributed Data Interface. */
        case ARPHRD_BIF:                   printf("AP1000 BIF");                 break;    /* AP1000 BIF. */
        case ARPHRD_SIT:                   printf("SIT0");                       break;    /* sit0 device - IPv6-in-IPv4. */
        case ARPHRD_IPDDP:                 printf("IP-in-DDP Tunnel");           break;    /* IP-in-DDP tunnel. */
        case ARPHRD_IPGRE:                 printf("GRE over IP");                break;    /* GRE over IP. */
        case ARPHRD_PIMREG:                printf("PIMSM");                      break;    /* PIMSM register interface. */
        case ARPHRD_HIPPI:                 printf("HIPPI");                      break;    /* High Performance Parallel I'face. */
        case ARPHRD_ASH:                   printf("ASH");                        break;    /* (Nexus Electronics) Ash. */
        case ARPHRD_ECONET:                printf("EcoNet");                     break;    /* Acorn Econet. */
        case ARPHRD_IRDA:                  printf("IrDA");                       break;    /* Linux-IrDA. */
        case ARPHRD_FCPP:                  printf("FCPP");                       break;    /* Point to point fibrechanel. */
        case ARPHRD_FCAL:                  printf("FCAL");                       break;    /* Fibrechanel arbitrated loop. */
        case ARPHRD_FCPL:                  printf("FCPL");                       break;    /* Fibrechanel public loop. */
        case ARPHRD_FCFABRIC:              printf("FCFABRIC");                   break;    /* Fibrechanel fabric. */
        case ARPHRD_IEEE802_TR:            printf("TR");                         break;    /* Magic type ident for TR. */
        case ARPHRD_IEEE80211:             printf("IEEE 802.11");                break;    /* IEEE 802.11. */
        case ARPHRD_IEEE80211_PRISM:       printf("IEEE 802.11 Prism2");         break;    /* IEEE 802.11 + Prism2 header. */
        case ARPHRD_IEEE80211_RADIOTAP:    printf("IEEE 802.11 RadioTap");       break;    /* IEEE 802.11 + radiotap header. */
        case ARPHRD_IEEE802154:            printf("IEEE 802.15.4");              break;    /* IEEE 802.15.4 header. */
        case ARPHRD_IEEE802154_PHY:        printf("IEEE 802.15.4 Physical");     break;    /* IEEE 802.15.4 PHY header. */

    /*  case ARPHRD_VOID:                  printf("Void");                       break;    /* Void type, nothing is known. */
    /*  case ARPHRD_NONE:                  printf("None");                       break;    /* Zero header length. */
        default:                           printf("Unknown");                    break;
    }

    printf(") \n");

    return;
}
