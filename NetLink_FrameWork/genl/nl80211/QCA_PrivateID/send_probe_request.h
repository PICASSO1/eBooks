#ifndef __SEND_PROBE_REQUEST_H__
#define __SEND_PROBE_REQUEST_H__

#include <errno.h>
#include <linux/nl80211.h>
#include <netlink/attr.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <netlink/msg.h>
#include <netlink/netlink.h>
#include <netlink/socket.h>

/* Refer from: qca_vendor.h */
#define QCA_NL80211_VENDOR_ID                               0x00001374
#define QCA_WLAN_VENDOR_ATTR_CONFIG_GENERIC_COMMAND         17
#define QCA_WLAN_VENDOR_ATTR_CONFIG_GENERIC_VALUE           18
#define QCA_WLAN_VENDOR_ATTR_CONFIG_GENERIC_DATA            19
#define QCA_NL80211_VENDOR_SUBCMD_SET_WIFI_CONFIGURATION    74

/* Refer from: cfg80211_external.h */
#define QCA_NL80211_VENDOR_SUBCMD_WIFI_PARAMS               200
#define IEEE80211_PARAM_SEND_PROBE_REQ                      694


struct cb_context {
    int err;
    int done;
};

int error_handler(struct sockaddr_nl *, struct nlmsgerr *, void *);
int valid_handler(struct nl_msg *, void *);
int finish_handler(struct nl_msg *, void *);
int ack_handler(struct nl_msg *, void *);

int send_probe_request(struct nl80211_state *, const char *);

#endif    /* __SEND_PROBE_REQUEST_H__ */
