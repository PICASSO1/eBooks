/**
 * File Name: main.c
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
 *   - 這個範例呈現了 struct ifconf / ifreq - ioctl(); 的一些基本用法；struct ifconf / ifreq 的結構定義在：/usr/include/net/if.h
 *   - struct ifconf 僅能對應到 SIOCGIFCONF，功能為取得本機當中所有的 Networking Interface。
 *   - "SIOCS/GXXXX" 指的是 "Socket I/O Control Set/Get XXXX" ；所有的定義在：/usr/include/linux/sockios.h
 *   - 並不是所有的 SIOCS/GXXXX 都可以使用，還要端看 Networking Device Driver 的支援；
 *   - 例如：SIOCG/SIFMEM 大部份的 Driver 都不會讓使用者去做修改；
 *   - 在本範例當中，Link Encapsulation 也沒有辦法取得；但還是可以從 MAC Address 的另一個結構成員 (sa_family)來判斷。
 *   - ifconfig 指令的所有資訊，均可在本範例當中查看；但 Statistics (統計資訊)並無法從 ioctl(); 取得；
 *   - ifconfig 當中的 Statistics 資料，是透過 /proc/net/dev 取得得，程式碼是寫在 BusyBox - Networking: ifconfig.c / interface.c
 *   - 為了避免發生無法預期的行為，均使用 Get 取的資訊，並不使用 Set 來做設定。
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

void display_FLAGs(signed short *);
void display_IFENCAP(unsigned short *);

#define INTERFACE_CNT    16    /* We DON'T know how many Networking Interface in a PC, so we Fixed a number!! */

int 
main(argc, argv, envp)
int argc;
char *argv[];
char **envp;
{
    int fd = -1, nRet = -1, idx = -1;
    struct ifconf ifc;
    struct ifreq ifr[INTERFACE_CNT];
    unsigned int if_cnt = 0;

    fd = socket(AF_INET, SOCK_DGRAM, 0);

    bzero((void *)&ifc, sizeof(struct ifconf));
    ifc.ifc_len = sizeof(struct ifreq) * INTERFACE_CNT;
    ifc.ifc_buf = (char *)ifr;
	
    nRet = ioctl(fd, SIOCGIFCONF, &ifc);
    if (nRet < 0 ) {
        perror("ioctl");
        close(fd);
        exit(EXIT_FAILURE);
    }
    if_cnt = ifc.ifc_len / sizeof(struct ifreq);

    for (idx = 0; idx < if_cnt; idx++) {
        /* Get Interface Index. */
        nRet = ioctl(fd, SIOCGIFINDEX, &ifr[idx]);
        if (nRet < 0 ) {
            perror("ioctl");
            close(fd);
            exit(EXIT_FAILURE);
        }
        printf("SIOCGIFINDEX: %d \n", ifr[idx].ifr_ifindex);

        /* Get Interface Name. */
        nRet = ioctl(fd, SIOCGIFNAME, &ifr[idx]);
        if (nRet < 0 ) {
            perror("ioctl");
            close(fd);
            exit(EXIT_FAILURE);
        }
        printf("SIOCGIFNAME: %s \n", ifr[idx].ifr_name);

        /* Get IPv4 Address. */
        nRet = ioctl(fd, SIOCGIFADDR, &ifr[idx]);
        if (nRet < 0 ) {
            perror("ioctl");
            close(fd);
            exit(EXIT_FAILURE);
        }
        printf("SIOCGIFADDR: %s \n", inet_ntoa(((struct sockaddr_in *)&(ifr[idx].ifr_addr))->sin_addr));

        /* Get IPv4 NetMask. */
        nRet = ioctl(fd, SIOCGIFNETMASK, &ifr[idx]);
        if (nRet < 0 ) {
            perror("ioctl");
            close(fd);
            exit(EXIT_FAILURE);
        }
        printf("SIOCGIFNETMASK: %s \n", inet_ntoa(((struct sockaddr_in *)&(ifr[idx].ifr_netmask))->sin_addr));

        /* Get BroadCast IPv4 Address. */
        nRet = ioctl(fd, SIOCGIFBRDADDR, &ifr[idx]);
        if (nRet < 0 ) {
            perror("ioctl");
            close(fd);
            exit(EXIT_FAILURE);
        }
        printf("SIOCGIFBRDADDR: %s \n", inet_ntoa(((struct sockaddr_in *)&(ifr[idx].ifr_broadaddr))->sin_addr));

        /* Get MAC (Hareware) Address. */
        nRet = ioctl(fd, SIOCGIFHWADDR, &ifr[idx]);
        if (nRet < 0 ) {
            perror("ioctl");
            close(fd);		
            exit(EXIT_FAILURE);
        }
        printf("SIOCSIFHWADDR: %02X:%02X:%02X:%02X:%02X:%02X \n", \
            (unsigned char)ifr[idx].ifr_hwaddr.sa_data[0], (unsigned char)ifr[idx].ifr_hwaddr.sa_data[1], \
            (unsigned char)ifr[idx].ifr_hwaddr.sa_data[2], (unsigned char)ifr[idx].ifr_hwaddr.sa_data[3], \
            (unsigned char)ifr[idx].ifr_hwaddr.sa_data[4], (unsigned char)ifr[idx].ifr_hwaddr.sa_data[5]);

        /* Get Encapsulation: ioctl: Inappropriate ioctl for device */
/*      nRet = ioctl(fd, SIOCGIFENCAP, &ifr[idx]);
        if (nRet < 0 ) {
            perror("ioctl");
            close(fd);
            exit(EXIT_FAILURE);
        }
*/      printf("SIOCGIFENCAP: %hu ", ifr[idx].ifr_hwaddr.sa_family);
        display_IFENCAP(&ifr[idx].ifr_hwaddr.sa_family);

        /* Get Destination Ipv4 Address. */
        nRet = ioctl(fd, SIOCGIFDSTADDR, &ifr[idx]);
        if (nRet < 0 ) {
            perror("ioctl");
            close(fd);
            exit(EXIT_FAILURE);
        }
        printf("SIOCGIFDSTADDR: %s \n", inet_ntoa(((struct sockaddr_in *)&(ifr[idx].ifr_dstaddr))->sin_addr));

        /* Get Flags. */
        nRet = ioctl(fd, SIOCGIFFLAGS, &ifr[idx]);
        if (nRet < 0 ) {
            perror("ioctl");
            close(fd);
            exit(EXIT_FAILURE);
        }
        printf("SIOCGIFFLAGS: %d ", (int)ifr[idx].ifr_flags);
        display_FLAGs(&ifr[idx].ifr_flags);
        printf("\n");

        /* Get MTU size. */
        nRet = ioctl(fd, SIOCGIFMTU, &ifr[idx]);
        if (nRet < 0 ) {
            perror("ioctl");
            close(fd);
            exit(EXIT_FAILURE);
        }
        printf("SIOCGIFMTU: %d \n", ifr[idx].ifr_mtu);

        /* Get Metric. */
        nRet = ioctl(fd, SIOCGIFMETRIC, &ifr[idx]);
        if (nRet < 0 ) {
            perror("ioctl");
            close(fd);
            exit(EXIT_FAILURE);
        }
        printf("SIOCGIFMETRIC: %d \n", ifr[idx].ifr_metric);

        /* Get Tx Queue Length. */
        nRet = ioctl(fd, SIOCGIFTXQLEN, &ifr[idx]);
        if (nRet < 0 ) {
            perror("ioctl");
            close(fd);
            exit(EXIT_FAILURE);
        }
        printf("SIOCGIFTXQLEN: %d \n", ifr[idx].ifr_qlen);

        printf("\n");
    }
    close(fd);

    return 0;
}

void 
display_FLAGs(flags)
signed short *flags;
{
    printf("<");

    if (*flags & IFF_UP)             printf("UP, ");
    if (*flags & IFF_BROADCAST)      printf("BROADCAST, ");
    if (*flags & IFF_DEBUG)          printf("DEBUG, ");
    if (*flags & IFF_LOOPBACK)       printf("LOOPBACK, ");
    if (*flags & IFF_POINTOPOINT)    printf("POINTOPOINT, ");
    if (*flags & IFF_NOTRAILERS)     printf("NOTRAILERS, ");
    if (*flags & IFF_RUNNING)        printf("RUNNING, ");
    if (*flags & IFF_NOARP)          printf("NOARP, ");
    if (*flags & IFF_PROMISC)        printf("PROMISC, ");
    if (*flags & IFF_ALLMULTI)       printf("ALLMULTI, ");
    if (*flags & IFF_MASTER)         printf("MASTER, ");
    if (*flags & IFF_SLAVE)          printf("SLAVE, ");
    if (*flags & IFF_MULTICAST)      printf("MULTICAST, ");
    if (*flags & IFF_PORTSEL)        printf("PORTSEL, ");
    if (*flags & IFF_AUTOMEDIA)      printf("AUTOMEDIA, ");
    if (*flags & IFF_DYNAMIC)        printf("DYNAMIC");

    printf(">");

    return;
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
