#ifndef __MAIN_H__
#define __MAIN_H__

#include <net/if.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <linux/nl80211.h>

struct nl80211_state {
    struct nl_sock *sock;
    int nl80211_id;
};

int init_nl80211(struct nl80211_state *);
void deinit_nl80211(struct nl80211_state *);

#endif    /* __MAIN_H__ */
