#ifndef __MAIN_H__
#define __MAIN_H__

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <net/if.h>
#include <errno.h>

#include <linux/nl80211.h>    /* for enum nl80211_commands cmd and enum nl80211_bss_scan_width cmd; */

struct nl80211_state {
    struct nl_sock *sock;
    int nl80211_id;
};

struct nl80211_commands_type {
    enum nl80211_commands cmd;
    const char *desc;
};

/* Refer from <linux/wireless.h> */
enum SCAN_TYPE {
    IW_SCAN_TYPE_ACTIVE = 0,
    IW_SCAN_TYPE_PASSIVE
};

struct nl80211_bss_scan_width_type {
    enum nl80211_bss_scan_width cmd;
    const char *desc;
};

int init_nl80211(struct nl80211_state *);
void deinit_nl80211(struct nl80211_state *);

void print_mac(const unsigned char *, int);
void print_ssid(const unsigned char *, int);
const char *iftype_to_string(enum nl80211_iftype);
const char *channel_type_to_string(unsigned int);
const char *channel_width_to_string(unsigned int);
const char *cmd_to_string(enum nl80211_commands);
const char *bss_width_to_string(enum nl80211_bss_scan_width);

#endif    /* __MAIN_H__ */
