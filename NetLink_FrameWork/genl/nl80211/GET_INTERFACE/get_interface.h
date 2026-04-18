#ifndef __GET_INTERFACE_H__
#define __GET_INTERFACE_H__

#include <errno.h>
#include <net/if.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <linux/nl80211.h>

#include <netlink/attr.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <netlink/msg.h>
#include <netlink/netlink.h>
#include <netlink/socket.h>

struct nl80211_state {
    struct nl_sock *sock;
    int nl80211_id;
};

/* callback context，多個 callback 之間共享「目前收訊流程的狀態」 */
struct cb_context {
    int err;
    int valid;
    int finish;
    int ack;
};

struct nl80211_commands_type {
    enum nl80211_commands cmd;
    const char *desc;
};

static const struct nl80211_commands_type nl80211_commands_type_map[] = {
    /* 不想花時間寫這種東西！沒意義！ */
    { NL80211_CMD_NEW_INTERFACE, "NL80211_CMD_NEW_INTERFACE" },
};

int error_handler(struct sockaddr_nl *, struct nlmsgerr *, void *);

int finish_handler(struct nl_msg *, void *);
int ack_handler(struct nl_msg *, void *);

const char *iftype_to_string(enum nl80211_iftype);
void print_mac(const unsigned char *, int);

const char *channel_type_to_string(unsigned int);
const char *channel_width_to_string(unsigned int);
void print_ssid(const unsigned char *, int);

const char *cmd_to_string(enum nl80211_commands);

#endif    /* __GET_INTERFACE_H__ */
