#ifndef __GET_INTERFACE_H__
#define __GET_INTERFACE_H__

#include <errno.h>
#include <linux/nl80211.h>
#include <netlink/attr.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <netlink/msg.h>
#include <netlink/netlink.h>
#include <netlink/socket.h>

/* callback context，多個 callback 之間共享「目前收訊流程的狀態」 */
struct cb_context {
    int err;
    int done;
};

int error_handler(struct sockaddr_nl *, struct nlmsgerr *, void *);
int valid_handler(struct nl_msg *, void *);
int finish_handler(struct nl_msg *, void *);
int ack_handler(struct nl_msg *, void *);

int dump_interfaces(struct nl80211_state *);

#endif    /* __GET_INTERFACE_H__ */
