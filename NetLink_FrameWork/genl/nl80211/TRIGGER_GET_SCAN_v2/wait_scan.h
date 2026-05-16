#ifndef __WAIT_SCAN_H__
#define __WAIT_SCAN_H__

#include <netlink/attr.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <netlink/msg.h>
#include <netlink/netlink.h>
#include <netlink/socket.h>
#include <errno.h>
#include <linux/nl80211.h>    /* for enum nl80211_commands cmd and enum nl80211_bss_scan_width cmd; */

struct scan_event_ctx {
    int done;
    int aborted;
};

int nl80211_subscribe_scan_group(struct nl80211_state *);
int scan_event_handler(struct nl_msg *, void *);
int nl80211_wait_scan_done(struct nl80211_state *);
int no_seq_check(struct nl_msg *, void *);

#endif    /* __WAIT_SCAN_H__ */
