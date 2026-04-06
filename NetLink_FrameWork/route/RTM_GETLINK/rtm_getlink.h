#ifndef _RTM_GETLINK_H_
#define _RTM_GETLINK_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>         /* for socket(), bind(), send(), recvmsg() & struct iovec */
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <net/if.h>
#include <netdb.h>              /* for herror() */
#include <stddef.h>
#include <linux/if_arp.h>
#include <linux/if_link.h>

#define BUFSIZE 8192            /* 緩衝區大小 */

void parse_nlmsg_err(struct nlmsghdr *);

/* for parse nlmsg type */
struct nlmsg_type {
	__u16 type;
	char *description;
};

/* Refer from linux/rtnetlink.h */
static const struct nlmsg_type nlmsg_type_map[] = {
	{ RTM_NEWLINK, "RTM_NEWLINK" }, 
	{ RTM_DELLINK, "RTM_DELLINK" }, 
	{ RTM_GETLINK, "RTM_GETLINK" }, 
	{ RTM_SETLINK, "RTM_SETLINK" }, 

	{ RTM_NEWADDR, "RTM_NEWADDR" }, 
	{ RTM_DELADDR, "RTM_DELADDR" }, 
	{ RTM_GETADDR, "RTM_GETADDR" }, 

	{ RTM_NEWROUTE, "RTM_NEWROUTE" }, 
	{ RTM_DELROUTE, "RTM_DELROUTE" }, 
	{ RTM_GETROUTE, "RTM_GETROUTE" }, 

	{ RTM_NEWNEIGH, "RTM_NEWNEIGH" }, 
	{ RTM_DELNEIGH, "RTM_DELNEIGH" }, 
	{ RTM_GETNEIGH, "RTM_GETNEIGH" }, 

	{ RTM_NEWRULE, "RTM_NEWRULE" }, 
	{ RTM_DELRULE, "RTM_DELRULE" }, 
	{ RTM_GETRULE, "RTM_GETRULE" }, 

	{ RTM_NEWQDISC, "RTM_NEWQDISC" }, 
	{ RTM_DELQDISC, "RTM_DELQDISC" }, 
	{ RTM_GETQDISC, "RTM_GETQDISC" }, 

	{ RTM_NEWTCLASS, "RTM_NEWTCLASS" }, 
	{ RTM_DELTCLASS, "RTM_DELTCLASS" }, 
	{ RTM_GETTCLASS, "RTM_GETTCLASS" }, 

	{ RTM_NEWTFILTER, "RTM_NEWTFILTER" }, 
	{ RTM_DELTFILTER, "RTM_DELTFILTER" }, 
	{ RTM_GETTFILTER, "RTM_GETTFILTER" }, 

	{ RTM_NEWACTION, "RTM_NEWACTION" }, 
	{ RTM_DELACTION, "RTM_DELACTION" }, 
	{ RTM_GETACTION, "RTM_GETACTION" }, 

	{ RTM_NEWPREFIX, "RTM_NEWPREFIX" }, 
	{ RTM_GETMULTICAST, "RTM_GETMULTICAST" }, 
	{ RTM_GETANYCAST, "RTM_GETANYCAST" }, 

	{ RTM_NEWNEIGHTBL, "RTM_NEWNEIGHTBL" }, 
	{ RTM_GETNEIGHTBL, "RTM_GETNEIGHTBL" }, 
	{ RTM_SETNEIGHTBL, "RTM_SETNEIGHTBL" }, 

	{ RTM_NEWNDUSEROPT, "RTM_NEWNDUSEROPT" }, 

	{ RTM_NEWADDRLABEL, "RTM_NEWADDRLABEL" }, 
	{ RTM_DELADDRLABEL, "RTM_DELADDRLABEL" }, 
	{ RTM_GETADDRLABEL, "RTM_GETADDRLABEL" }, 

	{ RTM_GETDCB, "RTM_GETDCB" }, 
	{ RTM_SETDCB, "RTM_SETDCB" }, 

	{ RTM_NEWNETCONF, "RTM_NEWNETCONF" }, 
	{ RTM_DELNETCONF, "RTM_DELNETCONF" }, 
	{ RTM_GETNETCONF, "RTM_GETNETCONF" }, 

	{ RTM_NEWMDB, "RTM_NEWMDB" }, 
	{ RTM_DELMDB, "RTM_DELMDB" }, 
	{ RTM_GETMDB, "RTM_GETMDB" }, 

	{ RTM_NEWNSID, "RTM_NEWNSID" }, 
	{ RTM_DELNSID, "RTM_DELNSID" }, 
	{ RTM_GETNSID, "RTM_GETNSID" }, 

	{ RTM_NEWSTATS, "RTM_NEWSTATS" }, 
	{ RTM_GETSTATS, "RTM_GETSTATS" }, 

	{ RTM_NEWCACHEREPORT, "RTM_NEWCACHEREPORT" }, 

	{ RTM_NEWCHAIN, "RTM_NEWCHAIN" }, 
	{ RTM_DELCHAIN, "RTM_DELCHAIN" }, 
	{ RTM_GETCHAIN, "RTM_GETCHAIN" }, 

	{ RTM_NEWNEXTHOP, "RTM_NEWNEXTHOP" }, 
	{ RTM_DELNEXTHOP, "RTM_DELNEXTHOP" }, 
	{ RTM_GETNEXTHOP, "RTM_GETNEXTHOP" }, 
};

void parse_nlmsg_type(__u16);

/* for parse interface family */
struct interface_family {
	unsigned char family;
	char *description;
};

/* Refer from /x86_64-linux-gnu/bits/socket.h */
static const struct interface_family interface_family_map[] = {
	{ AF_UNSPEC,     "AF_UNSPEC" },                       /* Unspecified. */
	{ AF_LOCAL,      "AF_LOCAL | AF_UNIX | AF_FILE" },    /* Local to host (pipes and file-domain). */
	{ AF_INET,       "AF_INET" },                         /* IP protocol family. */
	{ AF_AX25,       "AF_AX25" },                         /* Amateur Radio AX.25. */
	{ AF_IPX,        "AF_IPX" },                          /* Novell Internet Protocol. */
	{ AF_APPLETALK,  "AF_APPLETALK" },                    /* Appletalk DDP. */
	{ AF_NETROM,     "AF_NETROM" },                       /* Amateur radio NetROM. */
	{ AF_BRIDGE,     "AF_BRIDGE" },                       /* Multiprotocol bridge. */
	{ AF_ATMPVC,     "AF_ATMPVC" },                       /* ATM PVCs. */
	{ AF_X25,        "AF_X25" },                          /* Reserved for X.25 project. */
	{ AF_INET6,      "AF_INET6" },                        /* IP version 6. */
	{ AF_ROSE,       "AF_ROSE" },                         /* Amateur Radio X.25 PLP. */
	{ AF_DECnet,     "AF_DECnet" },                       /* Reserved for DECnet project. */
	{ AF_NETBEUI,    "AF_NETBEUI" },                      /* Reserved for 802.2LLC project. */
	{ AF_SECURITY,   "AF_SECURITY" },                     /* Security callback pseudo AF. */
	{ AF_KEY,        "AF_KEY" },                          /* PF_KEY key management API. */
	{ AF_NETLINK,    "AF_NETLINK | AF_ROUTE" },           /* Alias to emulate 4.4BSD. */
	{ AF_PACKET,     "AF_PACKET" },                       /* Packet family. */
	{ AF_ASH,        "AF_ASH" },                          /* Ash. */
	{ AF_ECONET,     "AF_ECONET" },                       /* Acorn Econet. */
	{ AF_ATMSVC,     "AF_ATMSVC" },                       /* ATM SVCs. */
	{ AF_RDS,        "AF_RDS" },                          /* RDS sockets. */
	{ AF_SNA,        "AF_SNA" },                          /* Linux SNA Project */
	{ AF_IRDA,       "AF_IRDA" },                         /* IRDA sockets. */
	{ AF_PPPOX,      "AF_PPPOX" },                        /* PPPoX sockets. */
	{ AF_WANPIPE,    "AF_WANPIPE" },                      /* Wanpipe API sockets. */
	{ AF_LLC,        "AF_LLC" },                          /* Linux LLC. */
	{ AF_IB,         "AF_IB" },                           /* Native InfiniBand address. */
	{ AF_MPLS,       "AF_MPLS" },                         /* MPLS. */
	{ AF_CAN,        "AF_CAN" },                          /* Controller Area Network. */
	{ AF_TIPC,       "AF_TIPC" },                         /* TIPC sockets. */
	{ AF_BLUETOOTH,  "AF_BLUETOOTH" },                    /* Bluetooth sockets. */
	{ AF_IUCV,       "AF_IUCV" },                         /* IUCV sockets. */
	{ AF_RXRPC,      "AF_RXRPC" },                        /* RxRPC sockets. */
	{ AF_ISDN,       "AF_ISDN" },                         /* mISDN sockets. */
	{ AF_PHONET,     "AF_PHONET" },                       /* Phonet sockets. */
	{ AF_IEEE802154, "AF_IEEE802154" },                   /* IEEE 802.15.4 sockets. */
	{ AF_CAIF,       "AF_CAIF" },                         /* CAIF sockets. */
	{ AF_ALG,        "AF_ALG" },                          /* Algorithm sockets. */
	{ AF_NFC,        "AF_NFC" },                          /* NFC sockets. */
	{ AF_VSOCK,      "AF_VSOCK" },                        /* vSockets. */
	{ AF_KCM,        "AF_KCM" },                          /* Kernel Connection Multiplexor. */
	{ AF_QIPCRTR,    "AF_QIPCRTR" },                      /* Qualcomm IPC Router. */
	{ AF_SMC,        "AF_SMC" },                          /* SMC sockets. */
	{ AF_XDP,        "AF_XDP" },                          /* XDP sockets. */
	{ AF_MAX,        "AF_MAX" },                          /* For now.. */
};

void parse_interface_family(unsigned char);

/* for parse device type */
struct device_type {
	unsigned short type;
	char *description;
};

/* Refer from linux/if_arp.h */
static const struct device_type device_type_map[] = {
	{ ARPHRD_NETROM,             "ARPHRD_NETROM" },                /* from KA9Q: NET/ROM pseudo */
	{ ARPHRD_ETHER,              "ARPHRD_ETHER" },                 /* Ethernet 10Mbps */
	{ ARPHRD_EETHER,             "ARPHRD_EETHER" },                /* Experimental Ethernet */
	{ ARPHRD_AX25,               "ARPHRD_AX25" },                  /* AX.25 Level 2 */
	{ ARPHRD_PRONET,             "ARPHRD_PRONET" },                /* PROnet token ring */
	{ ARPHRD_CHAOS,              "ARPHRD_CHAOS" },                 /* Chaosnet */
	{ ARPHRD_IEEE802,            "ARPHRD_IEEE802" },               /* IEEE 802.2 Ethernet/TR/TB */
	{ ARPHRD_ARCNET,             "ARPHRD_ARCNET" },                /* ARCnet */
	{ ARPHRD_APPLETLK,           "ARPHRD_APPLETLK" },              /* APPLEtalk */
	{ ARPHRD_DLCI,               "ARPHRD_DLCI" },                  /* Frame Relay DLCI */
	{ ARPHRD_ATM,                "ARPHRD_ATM" },                   /* ATM */
	{ ARPHRD_METRICOM,           "ARPHRD_METRICOM" },              /* Metricom STRIP (new IANA id) */
	{ ARPHRD_IEEE1394,           "ARPHRD_IEEE1394" },              /* IEEE 1394 IPv4 - RFC 2734 */
	{ ARPHRD_EUI64,              "ARPHRD_EUI64" },                 /* EUI-64 */
	{ ARPHRD_INFINIBAND,         "ARPHRD_INFINIBAND" },            /* InfiniBand */
	/* Dummy types for non ARP hardware */
	{ ARPHRD_SLIP,               "ARPHRD_SLIP" }, 
	{ ARPHRD_CSLIP,              "ARPHRD_CSLIP" }, 
	{ ARPHRD_SLIP6,              "ARPHRD_SLIP6" }, 
	{ ARPHRD_CSLIP6,             "ARPHRD_CSLIP6" }, 
	{ ARPHRD_RSRVD,              "ARPHRD_RSRVD" },                 /* Notional KISS type */
	{ ARPHRD_ADAPT,              "ARPHRD_ADAPT" }, 
	{ ARPHRD_ROSE,               "ARPHRD_ROSE" }, 
	{ ARPHRD_X25,                "ARPHRD_X25" },                   /* CCITT X.25 */
	{ ARPHRD_HWX25,              "ARPHRD_HWX25" },                 /* Boards with X.25 in firmware */
	{ ARPHRD_CAN,                "ARPHRD_CAN" },                   /* Controller Area Network */
	{ ARPHRD_PPP,                "ARPHRD_PPP" }, 
	{ ARPHRD_HDLC,               "ARPHRD_CISCO | ARPHRD_HDLC" },   /* Cisco HDLC */
	{ ARPHRD_LAPB,               "ARPHRD_LAPB" },                  /* LAPB */
	{ ARPHRD_DDCMP,              "ARPHRD_DDCMP" },                 /* Digital's DDCMP protocol */
	{ ARPHRD_RAWHDLC,            "ARPHRD_RAWHDLC" },               /* Raw HDLC */
	{ ARPHRD_RAWIP,              "ARPHRD_RAWIP" },                 /* Raw IP */

	{ ARPHRD_TUNNEL,             "ARPHRD_TUNNEL" },                /* IPIP tunnel */
	{ ARPHRD_TUNNEL6,            "ARPHRD_TUNNEL6" },               /* IP6IP6 tunnel */
	{ ARPHRD_FRAD,               "ARPHRD_FRAD" },                  /* Frame Relay Access Device */
	{ ARPHRD_SKIP,               "ARPHRD_SKIP" },                  /* SKIP vif */
	{ ARPHRD_LOOPBACK,           "ARPHRD_LOOPBACK" },              /* Loopback device */
	{ ARPHRD_LOCALTLK,           "ARPHRD_LOCALTLK" },              /* Localtalk device */
	{ ARPHRD_FDDI,               "ARPHRD_FDDI" },                  /* Fiber Distributed Data Interface */
	{ ARPHRD_BIF,                "ARPHRD_BIF" },                   /* AP1000 BIF */
	{ ARPHRD_SIT,                "ARPHRD_SIT" },                   /* sit0 device - IPv6-in-IPv4 */
	{ ARPHRD_IPDDP,              "ARPHRD_IPDDP" },                 /* IP over DDP tunneller */
	{ ARPHRD_IPGRE,              "ARPHRD_IPGRE" },                 /* GRE over IP */
	{ ARPHRD_PIMREG,             "ARPHRD_PIMREG" },                /* PIMSM register interface */
	{ ARPHRD_HIPPI,              "ARPHRD_HIPPI" },                 /* High Performance Parallel Interface */
	{ ARPHRD_ASH,                "ARPHRD_ASH" },                   /* Nexus 64Mbps Ash */
	{ ARPHRD_ECONET,             "ARPHRD_ECONET" },                /* Acorn Econet */
	{ ARPHRD_IRDA,               "ARPHRD_IRDA" },                  /* Linux-IrDA */
	/* ARP works differently on different FC media .. so  */
	{ ARPHRD_FCPP,               "ARPHRD_FCPP" },                  /* Point to point fibrechannel */
	{ ARPHRD_FCAL,               "ARPHRD_FCAL" },                  /* Fibrechannel arbitrated loop */
	{ ARPHRD_FCPL,               "ARPHRD_FCPL" },                  /* Fibrechannel public loop */
	{ ARPHRD_FCFABRIC,           "ARPHRD_FCFABRIC" },              /* Fibrechannel fabric */
	/* 787->799 reserved for fibrechannel media types */
	{ ARPHRD_IEEE802_TR,         "ARPHRD_IEEE802_TR" },            /* Magic type ident for TR */
	{ ARPHRD_IEEE80211,          "ARPHRD_IEEE80211" },             /* IEEE 802.11 */
	{ ARPHRD_IEEE80211_PRISM,    "ARPHRD_IEEE80211_PRISM" },       /* IEEE 802.11 + Prism2 header */
	{ ARPHRD_IEEE80211_RADIOTAP, "ARPHRD_IEEE80211_RADIOTAP" },    /* IEEE 802.11 + radiotap header */
	{ ARPHRD_IEEE802154,         "ARPHRD_IEEE802154" }, 
	{ ARPHRD_IEEE802154_MONITOR, "ARPHRD_IEEE802154_MONITOR" },    /* IEEE 802.15.4 network monitor */

	{ ARPHRD_PHONET,             "ARPHRD_PHONET" },                /* PhoNet media type */
	{ ARPHRD_PHONET_PIPE,        "ARPHRD_PHONET_PIPE" },           /* PhoNet pipe header */
	{ ARPHRD_CAIF,               "ARPHRD_CAIF" },                  /* CAIF media type */
	{ ARPHRD_IP6GRE,             "ARPHRD_IP6GRE" },                /* GRE over IPv6 */
	{ ARPHRD_NETLINK,            "ARPHRD_NETLINK" },               /* Netlink header */
	{ ARPHRD_6LOWPAN,            "ARPHRD_6LOWPAN" },               /* IPv6 over LoWPAN */
	{ ARPHRD_VSOCKMON,           "ARPHRD_VSOCKMON" },              /* Vsock monitor header */
	{ ARPHRD_VOID,               "ARPHRD_VOID | ARPHRD_NONE" }
};

void parse_device_type(unsigned short);

void parse_device_flags(unsigned);

void parse_rta_attributes(struct rtattr *);

void print_mac_address(const unsigned char *, unsigned int);
void print_hex(const unsigned char *, unsigned int);

#endif	/* _RTM_GETLINK_H_ */
