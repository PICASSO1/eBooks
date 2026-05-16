#ifndef __GET_SCAN_H__
#define __GET_SCAN_H__

#include <netlink/attr.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <netlink/msg.h>
#include <netlink/netlink.h>
#include <netlink/socket.h>

struct get_cb_context {
    int err;
    int done;
};

int get_error_handler(struct sockaddr_nl *, struct nlmsgerr *, void *);
int get_valid_handler(struct nl_msg *, void *);
int get_finish_handler(struct nl_msg *, void *);
int get_ack_handler(struct nl_msg *, void *);

int nl80211_get_scan(struct nl80211_state *, const char *);

#endif    /* __GET_SCAN_H__ */
