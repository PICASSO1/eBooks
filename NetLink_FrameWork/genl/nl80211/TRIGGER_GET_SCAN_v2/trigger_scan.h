#ifndef __TRIGGER_SCAN_H__
#define __TRIGGER_SCAN_H__

#include <netlink/attr.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <netlink/msg.h>
#include <netlink/netlink.h>
#include <netlink/socket.h>

/* Forward Declaration (向前宣告):
 * trigger_scan.h 並不會知道 main.h 長什麼樣子；
 * 所以 trigger_scan.h 必須自己先宣告這個 struct nl80211_state; 的存在
 */
struct nl80211_state;

/* callback context，多個 callback 之間共享「目前收訊流程的狀態」 */
struct trigger_cb_context {
    int err;
    int done;
};

int trigger_error_handler(struct sockaddr_nl *, struct nlmsgerr *, void *);
int trigger_valid_handler(struct nl_msg *, void *);
int trigger_finish_handler(struct nl_msg *, void *);
int trigger_ack_handler(struct nl_msg *, void *);

extern int nl80211_trigger_scan(struct nl80211_state *, const char *, const enum SCAN_TYPE);

#endif    /* __TRIGGER_SCAN_H__ */
