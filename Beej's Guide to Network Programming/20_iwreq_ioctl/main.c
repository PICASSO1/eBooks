/**
 * File Name: main.c
 *
 * CopyLeft (C) 2024, Picasso's Fantasy Notepad. All rights reserved.
 *
 * Author: Pablo Picasso G. (PabloPicasso.G@gmail.com)
 *
 * Version: 1.0.0.build043024
 *
 * Date: 2024 / 04 / 30
 *
 * Description: 
 *  - 這個簡單的範例，呈現了無線網路 struct iwreq - ioctl(); 的一些基本用法；相關的資料結構定義在 /usr/include/linux/wireless.h
 *  - 但是，wireless 的 SIOCS/GXXXX 並不像 Ethernet 一樣，許多功能都必須要 driver 也要支援，要不然還是無法存取。
 *  - 這一個範例基本上都可以從 driver 讀出最原始的資料 (native data)；但是還必須寫一大堆的函式才能夠具體呈現，但自己不想花太多時間在這兒。
 *  - 例如：Frequency, Bit Rate, Tx Power, Sensitivity等等，這一部份就不想花太多時間在這裡。
 *  - struct iw_pointer 的最後一個成員 _u16 flags 在 wireless.h 也有許多相對應的描述，這裡也沒有花時間去做分析。 
 *  - 結論：看別人寫好的最快！不要自己搞。
 *  - SoC: Qualcomm IPQ8072A; 5GHz Radio: QCN5054
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
#include <net/if_arp.h>
#include <ifaddrs.h>
#include <linux/wireless.h>
#include <math.h>

#define INTERFACE_NAME

void display_OpMode(unsigned int *);
double iw_freq2float(const struct iw_freq *);

int 
main(argc, argv, envp)
int argc;
char *argv[];
char **envp;
{
    struct ifaddrs *stIfaddrs = (struct ifaddrs *)NULL;
    struct ifaddrs *pNext = (struct ifaddrs *)NULL;
    int nRet = -1, fd = -1, idx = -1;
    struct iwreq iwr;
    char strTemp[64];
    struct iw_range iwrange;                                                                     /* for SIOCGIWRANGE */
    struct iw_statistics iwstatistics;                                                           /* SIOCGIWSTATS */
    unsigned char key[IW_ENCODING_TOKEN_MAX];                                                    /* for SIOCGIWENCODE: Encoding key used */
    unsigned char generic[IW_GENERIC_IE_MAX];                                                    /* for SIOCGIWGENIE: Encoding key used */
    unsigned char keyext[sizeof(struct iw_encode_ext) + IW_ENCODING_TOKEN_MAX];                  /* for SIOCGIWENCODEEXT: Encoding Extended key used */

    nRet = getifaddrs(&stIfaddrs);
    if (nRet == -1) {
        herror("getifaddrs");
        exit(EXIT_FAILURE);
    }

    fd = socket(AF_INET, SOCK_STREAM, 0);

    for (pNext = stIfaddrs; pNext != (struct ifaddrs *)NULL; pNext = pNext->ifa_next) {
        bzero(&iwr, sizeof(struct iwreq));
        /* Get wireless name; link: IEEE 802.11ax */
        strncpy(iwr.ifr_ifrn.ifrn_name, pNext->ifa_name, strlen(pNext->ifa_name));
        printf("%s: ", iwr.ifr_ifrn.ifrn_name);                                                  /* like: "wlan" */
        nRet = ioctl(fd, SIOCGIWNAME, &iwr);
        if (nRet < 0) {
            printf("No Wireless Extensions. \n\n");                                              /* This is not a wireless interface. */
            continue;
        }
        else
            printf("\n\n");
        printf("SIOCGIWNAME: %s \n", iwr.u.name);

        /* Get ESSID */
        memset(strTemp, '\0', sizeof(char) * 64);
        iwr.u.essid.pointer = (void *)strTemp;
        iwr.u.essid.length = IW_ESSID_MAX_SIZE + 2; 
        printf("SIOCGIWESSID: ");
        nRet = ioctl(fd, SIOCGIWESSID, &iwr);
        if (nRet < 0 )
            printf("This device driver DOESN'T support this feature. \n");
        else {
            printf("%s \n", strTemp);
            printf(" - iwr.u.essid.flags = %hu \n", iwr.u.essid.flags);
        }

        /* Get Nickname */
        memset(strTemp, '\0', sizeof(char) * 64);
        iwr.u.data.pointer = (void *)strTemp;
        iwr.u.data.length = IW_ESSID_MAX_SIZE + 2; 
        printf("SIOCGIWNICKN: ");
        nRet = ioctl(fd, SIOCGIWNICKN, &iwr);
        if (nRet < 0 )
            printf("This device driver DOESN'T support this feature. \n");
        else {
            printf("%s \n", strTemp);
            printf(" - iwr.u.data.flags = %hu \n", iwr.u.data.flags);
        }
				
        /* Get Operation Mode */
        printf("SIOCGIWMODE: ");
        nRet = ioctl(fd, SIOCGIWMODE, &iwr);
        if (nRet < 0 )
            printf("This device driver DOESN'T support this feature. \n");
        else {
            display_OpMode(&iwr.u.mode);
            printf(" - iwr.u.mode = %u \n", iwr.u.mode);
        }
				
        /* Get BSSID (AP MAC Address) */
        printf("SIOCGIWAP: ");
        nRet = ioctl(fd, SIOCGIWAP, &iwr);
        if (nRet < 0 )
            printf("This device driver DOESN'T support this feature. \n");
        else {
            printf("%02X:%02X:%02X:%02X:%02X:%02X \n", \
                iwr.u.ap_addr.sa_data[0], iwr.u.ap_addr.sa_data[1], iwr.u.ap_addr.sa_data[2], \
                iwr.u.ap_addr.sa_data[3], iwr.u.ap_addr.sa_data[4], iwr.u.ap_addr.sa_data[5]);
            printf(" - iwr.u.ap_addr.sa_family = %hu \n", iwr.u.ap_addr.sa_family);
        }

        /* Get Network ID */
        printf("SIOCGIWNWID: ");
        nRet = ioctl(fd, SIOCGIWNWID, &iwr);
        if (nRet < 0 )
            printf("This device driver DOESN'T support this feature. \n");
        else {
            printf("\n");
            printf(" - iwr.u.nwid.value: %d \n", iwr.u.nwid.value);
            printf(" - iwr.u.nwid.fixed: %u \n", iwr.u.nwid.fixed);
            printf(" - iwr.u.nwid.disabled: %u \n", iwr.u.nwid.disabled);
            printf(" - iwr.u.nwid.flags: %u \n", iwr.u.nwid.flags);
        }

        /* Get RTS Threshold */
        printf("SIOCGIWRTS: ");
        nRet = ioctl(fd, SIOCGIWRTS, &iwr);
        if (nRet < 0 )
            printf("This device driver DOESN'T support this feature. \n");
        else {
            if (iwr.u.rts.disabled == 1)
                printf("Disable / OFF");
            else
                printf("%d", iwr.u.rts.value);
            printf("\n");
            printf(" - iwr.u.rts.value: %d \n", iwr.u.rts.value);
            printf(" - iwr.u.rts.fixed: %u \n", iwr.u.rts.fixed);
            printf(" - iwr.u.rts.disabled: %u \n", iwr.u.rts.disabled);
            printf(" - iwr.u.rts.flags: %u \n", iwr.u.rts.flags);
        }

        /* Get Fragmentation */
        printf("SIOCGIWFRAG: ");
        nRet = ioctl(fd, SIOCGIWFRAG, &iwr);
        if (nRet < 0 )
            printf("This device driver DOESN'T support this feature. \n");
        else {
            if (iwr.u.frag.disabled == 1)
                printf("Disable / OFF");
            else
                printf("%d", iwr.u.frag.value);
            printf("\n");
            printf(" - iwr.u.frag.value: %d \n", iwr.u.frag.value);
            printf(" - iwr.u.frag.fixed: %u \n", iwr.u.frag.fixed);
            printf(" - iwr.u.frag.disabled: %u \n", iwr.u.frag.disabled);
            printf(" - iwr.u.frag.flags: %u \n", iwr.u.frag.flags);
        }

        /* Get Retyr Limits and lifetime */
        printf("SIOCGIWRETRY: ");
        nRet = ioctl(fd, SIOCGIWRETRY, &iwr);
        if (nRet < 0 )
            printf("This device driver DOESN'T support this feature. \n");
        else {
            printf("%d \n");
            printf(" - iwr.u.retry.value: %d \n", iwr.u.retry.value);
            printf(" - iwr.u.retry.fixed: %u \n", iwr.u.retry.fixed);
            printf(" - iwr.u.retry.disabled: %u \n", iwr.u.retry.disabled);
            printf(" - iwr.u.retry.flags: %u \n", iwr.u.retry.flags);
        }

        /* Get Range of several Parameters */
        bzero(&iwrange, sizeof(struct iw_range));
        iwr.u.data.pointer = (void *)&iwrange;
        iwr.u.data.length = sizeof(struct iw_range);
        printf("SIOCGIWRANGE: ");
        nRet = ioctl(fd, SIOCGIWRANGE, &iwr);
        if (nRet < 0 )
            printf("This device driver DOESN'T support this feature. \n");
        else {
        /*  ((struct iw_range *)&iwr.u.data.pointer).throughput */
            printf("\n");
            /* Informative stuff (to choose between different interface) */
            printf(" - iwrange.throughput = %u \n", iwrange.throughput);                         /* To give an idea.... */

            /* NWID (or domain id) */
            printf(" - iwrange.min_nwid = %u \n", iwrange.min_nwid);                             /* Minimal NWID we are able to set. */
            printf(" - iwrange.max_nwid = %u \n", iwrange.max_nwid);                             /* Maximal NWID we are able to set. */

            /* Old Frequency (backward compat - moved lower ) */
            printf(" - iwrange.old_num_channels = %hu \n", iwrange.old_num_channels);
            printf(" - iwrange.old_num_frequency = %u \n", iwrange.old_num_frequency);

            /* Scan capabilities */
            printf(" - iwrange.scan_capa = %u \n", iwrange.scan_capa);                           /* IW_SCAN_CAPA_* bit field. */

            /* Wireless event capability bitmasks */
            printf(" - iwrange.event_capa[6] = ");
            for (idx = 0; idx < 6; idx++)
                printf("%u ", iwrange.event_capa[idx]);
            printf("\n");

            /* signal level threshold range */
            printf(" - iwrange.sensitivity = %d \n", iwrange.sensitivity);

            /* Quality of link & SNR stuff */
            /* Quality of the link. */
            printf(" - iwrange.max_qual.qual = %u \n", iwrange.max_qual.qual);
            printf(" - iwrange.max_qual.level = %u \n", iwrange.max_qual.level);
            printf(" - iwrange.max_qual.noise = %u \n", iwrange.max_qual.noise);
            printf(" - iwrange.max_qual.updated = %u \n", iwrange.max_qual.updated);
            /* Quality of the link. */
            printf(" - iwrange.avg_qual.qual = %u \n", iwrange.avg_qual.qual);
            printf(" - iwrange.avg_qual.level = %u \n", iwrange.avg_qual.level);
            printf(" - iwrange.avg_qual.noise = %u \n", iwrange.avg_qual.noise);
            printf(" - iwrange.avg_qual.updated = %u \n", iwrange.avg_qual.updated);
            /* Rates */
            printf(" - iwrange.num_bitrates = %u \n", iwrange.num_bitrates);                     /* Number of entries in the list. */
            printf(" - iwrange.bitrate[IW_MAX_BITRATES] = ");                                    /* list, in bps. */
            for (idx = 0; idx < IW_MAX_BITRATES; idx++)
                printf("%d ", iwrange.bitrate[idx]);
            printf("\n");

            /* RTS threshold */
            printf(" - iwrange.min_rts = %d \n", iwrange.min_rts);                               /* Minimal RTS threshold. */
            printf(" - iwrange.max_rts = %d \n", iwrange.max_rts);                               /* Maximal RTS threshold. */

            /* Frag threshold */
            printf(" - iwrange.min_frag = %d \n", iwrange.min_frag);                             /* Minimal frag threshold. */
            printf(" - iwrange.max_frag = %d \n", iwrange.max_frag);                             /* Maximal frag threshold. */

            /* Power Management duration & timeout */
            printf(" - iwrange.min_pmp = %d \n", iwrange.min_pmp);                               /* Minimal PM period. */
            printf(" - iwrange.max_pmp = %d \n", iwrange.max_pmp);                               /* Maximal PM period. */
            printf(" - iwrange.min_pmt = %d \n", iwrange.min_pmt);                               /* Minimal PM timeout. */
            printf(" - iwrange.max_pmt = %d \n", iwrange.max_pmt);                               /* Maximal PM timeout. */
            printf(" - iwrange.pmp_flags = %hu \n", iwrange.pmp_flags);                          /* How to decode max/min PM period. */
            printf(" - iwrange.pmt_flags = %hu \n", iwrange.pmt_flags);                          /* How to decode max/min PM timeout. */
            printf(" - iwrange.pm_capa = %hu \n", iwrange.pm_capa);                              /* What PM options are supported. */

            /* Encoder stuff */
            printf(" - iwrange.encoding_size[IW_MAX_ENCODING_SIZES] = ");                        /* Different token sizes. */
            for (idx = 0; idx < IW_MAX_ENCODING_SIZES; idx++)
                printf("%hu ", iwrange.encoding_size[idx]);
            printf("\n");

            printf(" - iwrange.num_encoding_sizes = %u \n", iwrange.num_encoding_sizes);         /* Number of entry in the list. */
            printf(" - iwrange.max_encoding_tokens = %u \n", iwrange.max_encoding_tokens);       /* Max number of tokens. */

            /* For drivers that need a "login/passwd" form */
            printf(" - iwrange.encoding_login_index = %u \n", iwrange.encoding_login_index);     /* token index for login token. */

            /* Transmit power */
            printf(" - iwrange.txpower_capa = %hu \n", iwrange.txpower_capa);                    /* What options are supported. */
            printf(" - iwrange.num_txpower = %u \n", iwrange.num_txpower);                       /* Number of entries in the list. */
            printf(" - iwrange.txpower[IW_MAX_TXPOWER] = ");                                     /* list, in bps. */
            for (idx = 0; idx < IW_MAX_TXPOWER; idx++)
                printf("%d ", iwrange.txpower[idx]);
            printf("\n");

            /* Wireless Extension version info */
            printf(" - iwrange.we_version_compiled = %u \n", iwrange.we_version_compiled);       /* Must be WIRELESS_EXT. */
            printf(" - iwrange.we_version_source = %u \n", iwrange.we_version_source);           /* Last update of source. */

            /* Retry limits and lifetime */
            printf(" - iwrange.retry_capa = %hu \n", iwrange.retry_capa);                        /* What retry options are supported. */
            printf(" - iwrange.retry_flags = %hu \n", iwrange.retry_flags);                      /* How to decode max/min retry limit. */
            printf(" - iwrange.r_time_flags = %hu \n", iwrange.r_time_flags);                    /* How to decode max/min retry life. */
            printf(" - iwrange.min_retry = %d \n", iwrange.min_retry);                           /* Minimal number of retries. */
            printf(" - iwrange.max_retry = %d \n", iwrange.max_retry);                           /* Maximal number of retries. */
            printf(" - iwrange.min_r_time = %d \n", iwrange.min_r_time);                         /* Minimal retry lifetime. */
            printf(" - iwrange.max_r_time = %d \n", iwrange.max_r_time);                         /* Maximal retry lifetime. */

            /* Frequency */
            printf(" - iwrange.num_channels = %hu \n", iwrange.num_channels);                    /* Number of channels [0; num - 1]. */
            printf(" - iwrange.num_frequency = %u \n", iwrange.num_frequency);                   /* Number of entry in the list. */

            printf(" - iwrange.freq[IW_MAX_FREQUENCIES]: \n");                                   /* list. */
            for (idx = 0; idx < IW_MAX_FREQUENCIES; idx++) {
                printf(" -- iwrange.freq[%d].m = %d \n", idx, iwrange.freq[idx].m);
                printf(" -- iwrange.freq[%d].e = %d \n", idx, (int)iwrange.freq[idx].e);
                printf(" -- iwrange.freq[%d].i = %u \n", idx, iwrange.freq[idx].i);
                printf(" -- iwrange.freq[%d].flags = %u \n", idx, iwrange.freq[idx].flags);
                printf("\n");
            }

            printf(" - iwrange.enc_capa = %u \n", iwrange.enc_capa);                             /* IW_ENC_CAPA_* bit field */
        }

        /* Get Frequency */
        printf("SIOCGIWFREQ: ");
        nRet = ioctl(fd, SIOCGIWFREQ, &iwr);
        if (nRet < 0 )
            printf("This device driver DOESN'T support this feature. \n");
        else {
            printf("\n");
            printf(" - iwr.u.freq.m: %d \n", iwr.u.freq.m);
            printf(" - iwr.u.freq.e: %d \n", (int)iwr.u.freq.e);
            printf(" - iwr.u.freq.i: %u \n", iwr.u.freq.i);
            printf(" - iwr.u.freq.flags: %u \n", iwr.u.freq.flags);
        }

        /* Get Encoding */
        memset(key, 0x00, sizeof(unsigned char) * IW_ENCODING_TOKEN_MAX);
        iwr.u.encoding.pointer = (void *)key;
        iwr.u.encoding.length = IW_ENCODING_TOKEN_MAX;
        printf("SIOCGIWENCODE: ");
        nRet = ioctl(fd, SIOCGIWENCODE, &iwr);
        if (nRet < 0 )
            printf("This device driver DOESN'T support this feature. \n");
        else {
            printf("\n");
            printf(" - iwr.u.encoding.pointer: ");
            for (idx = 0; idx < iwr.u.encoding.length; idx++)
                printf("%.2X", (unsigned char *)&iwr.u.encoding.pointer[idx]);
            printf("\n - iwr.u.encoding.flags: 0x%X \n", (unsigned int)iwr.u.encoding.flags);
        }

        /* Get bit rate (bps) */
        printf("SIOCGIWRATE: ");
        nRet = ioctl(fd, SIOCGIWRATE, &iwr);
        if (nRet < 0 )
            printf("This device driver DOESN'T support this feature. \n");
        else {
            printf("\n");
            printf(" - iwr.u.bitrate.value: %d \n", iwr.u.bitrate.value);
            printf(" - iwr.u.bitrate.fixed: %u \n", iwr.u.bitrate.fixed);
            printf(" - iwr.u.bitrate.disabled: %u \n", iwr.u.bitrate.disabled);
            printf(" - iwr.u.bitrate.flags: %hu \n", iwr.u.bitrate.flags);
        }

        /* Get Power Management Settings */
        printf("SIOCGIWPOWER: ");
        nRet = ioctl(fd, SIOCGIWPOWER, &iwr);
        if (nRet < 0 )
            printf("This device driver DOESN'T support this feature. \n");
        else {
            if (iwr.u.power.disabled == 1)
                printf("Disable / OFF");
            else
                printf("%d", iwr.u.power.value);
            printf("\n");
            printf(" - iwr.u.power.value: %d \n", iwr.u.power.value);
            printf(" - iwr.u.power.fixed: %u \n", iwr.u.power.fixed);
            printf(" - iwr.u.power.disabled: %u \n", iwr.u.power.disabled);
            printf(" - iwr.u.power.flags: %hu \n", iwr.u.power.flags);
        }

        /* Get Transmit Power */
        printf("SIOCGIWTXPOW: ");
        nRet = ioctl(fd, SIOCGIWTXPOW, &iwr);
        if (nRet < 0 )
            printf("This device driver DOESN'T support this feature. \n");
        else {
            if (iwr.u.txpower.disabled == 1)
                printf("Disable / OFF");
            else
                printf("%d dBm", iwr.u.txpower.value);
            printf("\n");
            printf(" - iwr.u.txpower.value: %d \n", iwr.u.txpower.value);
            printf(" - iwr.u.txpower.fixed: %u \n", iwr.u.txpower.fixed);
            printf(" - iwr.u.txpower.disabled: %u \n", iwr.u.txpower.disabled);
            printf(" - iwr.u.txpower.flags: %hu \n", iwr.u.txpower.flags);
        }

        /* Get Sensitivity (dBm) */
        printf("SIOCGIWSENS: ");
        nRet = ioctl(fd, SIOCGIWSENS, &iwr);
        if (nRet < 0 )
            printf("This device driver DOESN'T support this feature. \n");
        else {
            printf("\n");
            printf(" - iwr.u.sens.value: %d \n", iwr.u.sens.value);
            printf(" - iwr.u.sens.fixed: %u \n", iwr.u.sens.fixed);
            printf(" - iwr.u.sens.disabled: %u \n", iwr.u.sens.disabled);
            printf(" - iwr.u.sens.flags: %hu \n", iwr.u.sens.flags);
        }

        /* Get /proc/net/wireless Statistics */
        bzero(&iwstatistics, sizeof(struct iw_statistics));
        iwr.u.data.pointer = (void *)&iwstatistics;
        iwr.u.data.length = sizeof(struct iw_statistics);
        printf("SIOCGIWSTATS: ");
        nRet = ioctl(fd, SIOCGIWSTATS, &iwr);
        if (nRet < 0 )
            printf("This device driver DOESN'T support this feature. \n");
        else {
            printf("Link Quality=%u/%u; Rx invalid nwid: %u; Rx invalid crypt: %u; Rx invalid frag: %u; Tx excessive retries: %u; Invalid misc: %u; Missed beacon: %u \n", \
                iwstatistics.qual.qual, iwrange.max_qual.qual, iwstatistics.discard.nwid, iwstatistics.discard.code, iwstatistics.discard.fragment, \
                iwstatistics.discard.retries, iwstatistics.discard.misc, iwstatistics.miss.beacon);
            printf(" - iwstatistics.status: %hu \n", iwstatistics.status);                       /* Status - device dependent for now. */
            printf(" - iwstatistics.qual.qual: %u \n", iwstatistics.qual.qual);                  /* link quality (%retries, SNR, %missed beacons or better...). */
            printf(" - iwstatistics.qual.level: %u \n", iwstatistics.qual.level);                /* signal level (dBm). */
            printf(" - iwstatistics.qual.noise: %u \n", iwstatistics.qual.noise);                /* noise level (dBm). */
            printf(" - iwstatistics.qual.updated: %u \n", iwstatistics.qual.updated);            /* Flags to know if updated. */
            printf(" - iwstatistics.discard.nwid: %u \n", iwstatistics.discard.nwid);            /* Rx : Wrong nwid/essid. */
            printf(" - iwstatistics.discard.code: %u \n", iwstatistics.discard.code);            /* Rx : Unable to code/decode (WEP). */
            printf(" - iwstatistics.discard.fragment: %u \n", iwstatistics.discard.fragment);    /* Rx : Can't perform MAC reassembly. */
            printf(" - iwstatistics.discard.retries: %u \n", iwstatistics.discard.retries);      /* Tx : Max MAC retries num reached. */
            printf(" - iwstatistics.discard.misc: %u \n", iwstatistics.discard.misc);            /* Others cases. */
            printf(" - iwstatistics.miss.beacon: %u \n", iwstatistics.miss.beacon);              /* Missed beacons/superframe. */
        }

        /* Get Generic information element */
        memset(generic, 0x00, sizeof(unsigned char) * IW_GENERIC_IE_MAX);
        iwr.u.data.pointer = (void *)generic;
        iwr.u.data.length = IW_GENERIC_IE_MAX;
        printf("SIOCGIWGENIE: ");
        nRet = ioctl(fd, SIOCGIWGENIE, &iwr);
        if (nRet < 0 )
            printf("This device driver DOESN'T support this feature. \n");
        else {
            for (idx = 0; idx < iwr.u.data.length; idx++)
                printf("%.2X ", (unsigned char *)&iwr.u.data.pointer[idx]);

            printf("\n");
            printf(" - iwr.u.data.flags: 0x%04X \n", iwr.u.data.flags);
        }

        /* Get Encoding Extended */
        memset(generic, 0x00, sizeof(unsigned char) * IW_GENERIC_IE_MAX);
        iwr.u.data.pointer = (void *)keyext;
        iwr.u.data.length = sizeof(struct iw_encode_ext) + IW_ENCODING_TOKEN_MAX;
        printf("SIOCGIWENCODEEXT: ");
        nRet = ioctl(fd, SIOCGIWENCODEEXT, &iwr);
        if (nRet < 0 )
            printf("This device driver DOESN'T support this feature. \n");
        else {
            for (idx = 0; idx < iwr.u.data.length; idx++)
                printf("%.2X ", (unsigned char *)&iwr.u.data.pointer[idx]);

            printf("\n");
            printf(" - iwr.u.data.flags: 0x%04X \n", iwr.u.data.flags);
        }

        printf("\n");
    }

    close(fd);

    freeifaddrs((void *)stIfaddrs);

    return 0;
}

/* Modes as human readable strings; refer from wireless.h */
void display_OpMode(unsigned int *MODE)
{
    switch(*MODE) {
        case IW_MODE_AUTO:       printf("Auto");              break;    /* Let the driver decides. */
        case IW_MODE_ADHOC:      printf("Ad-Hoc");            break;    /* Single cell network. */
        case IW_MODE_INFRA:      printf("Infrastructure");    break;    /* Multi cell network, roaming, ... */
        case IW_MODE_MASTER:     printf("Master");            break;    /* Synchronisation master or Access Point. */
        case IW_MODE_REPEAT:     printf("Repeater");          break;    /* Wireless Repeater (forwarder). */
        case IW_MODE_SECOND:     printf("Secondary");         break;    /* Secondary master/repeater (backup). */
        case IW_MODE_MONITOR:    printf("Monitor");           break;    /* Passive monitor (listen only). */
        case IW_MODE_MESH:       printf("Mesh");              break;    /* Mesh (IEEE 802.11s) network. */
        default:                 printf("Unknown / Bug");     break;
    }
    printf("\n");

    return;
}

