#include "rtm_getroute.h"

void
parse_nlmsg_err(nlh)
struct nlmsghdr *nlh;
{
	struct nlmsgerr *err = (struct nlmsgerr *)NULL;
	int rem = -1;

	if (nlh->nlmsg_len < NLMSG_LENGTH(sizeof(struct nlmsgerr))) {
		fprintf(stderr, "NLMSG_ERROR too short\n");
		return;
	}

	err = (struct nlmsgerr *)NLMSG_DATA(nlh);

	if (err->error == 0)
		printf("Netlink ACK: success\n");
	else
		fprintf(stderr, "NetLink ACK: error = %d (%s)\n", err->error, strerror(-err->error));

	/* Extended Ack TLVs 可能出現在 struct nlmsgerr 後面；所以位置要從 nlmsgerr 結構體尾端開始算。 */
	rem = nlh->nlmsg_len - NLMSG_LENGTH(sizeof(struct nlmsgerr));
	if (rem > 0) {
		struct nlattr *attr = (struct nlattr *)((char *)err + sizeof(struct nlmsgerr));

		for (; RTA_OK((struct rtattr *)attr, rem); attr = (struct nlattr *)RTA_NEXT((struct rtattr *)attr, rem)) {
			switch (attr->nla_type) {
				case NLMSGERR_ATTR_MSG:
					fprintf(stderr, "ExtAck MSG : %s\n", (char *)RTA_DATA(attr));
					break;
				case NLMSGERR_ATTR_OFFS:
					fprintf(stderr, "ExtAck OFFS: %u\n", *(unsigned int *)RTA_DATA(attr));
					break;
				case NLMSGERR_ATTR_UNUSED:
					fprintf(stderr, "ExtAck UNUSED: %u\n", *(unsigned short *)RTA_DATA(attr));
					break;
				case NLMSGERR_ATTR_COOKIE:
					fprintf(stderr, "ExtAck COOKIE len = %u\n", attr->nla_len);
					break;
				default:
					fprintf(stderr, "ExtAck attr type = %u; len = %u \n", attr->nla_type, attr->nla_len);
				break;
			}
		}
	}
	return;
}

void 
parse_nlmsg_type(nlmsg_type)
__u16 nlmsg_type;
{
	size_t i = 0;

	printf("Message Type: ");
	for (i = 0; i < sizeof(nlmsg_type_map) / sizeof(struct nlmsg_type); i++) {
		if (nlmsg_type_map[i].type == nlmsg_type) {
			printf("%s (%u) \n", nlmsg_type_map[i].description, nlmsg_type);
			return;
		}
	}
	printf("Unknown nlmsg message type (%u) \n", nlmsg_type);

	return;
}

void
parse_address_type(rtm_family)
unsigned char rtm_family;
{
	size_t i = 0;

	printf("Address Type: ");
	for (i = 0; i < sizeof(address_type_map) / sizeof(struct address_type); i++) {
		if (address_type_map[i].type == rtm_family) {
			printf("%s (%u) \n", address_type_map[i].description, rtm_family);
			return;
		}
	}
	printf("Unknown address type (%u) \n", rtm_family);

	return;
}

void 
parse_routing_protocol(rtm_protocol)
unsigned char rtm_protocol;
{
	printf("Routing Protocol: ");
#define PRINT_ROUTEING_PROTOCOL(VALUE, TARGET, STR) \
	do { \
		if ((VALUE) == (TARGET)) { \
			printf(STR); \
			printf(" (0x%02X)\n", VALUE); \
			return; \
		} \
	} while (0)
	PRINT_ROUTEING_PROTOCOL(rtm_protocol, RTPROT_UNSPEC  , "Unknown Route??");
	PRINT_ROUTEING_PROTOCOL(rtm_protocol, RTPROT_REDIRECT, "Route installed by ICMP redirects; not used by current IPv4");
	PRINT_ROUTEING_PROTOCOL(rtm_protocol, RTPROT_KERNEL  , "Route installed by kernel");
	PRINT_ROUTEING_PROTOCOL(rtm_protocol, RTPROT_BOOT    , "Route installed during boot");
	PRINT_ROUTEING_PROTOCOL(rtm_protocol, RTPROT_STATIC  , "Route installed by administrator");
	PRINT_ROUTEING_PROTOCOL(rtm_protocol, RTPROT_GATED   , "Apparently, GateD");
	PRINT_ROUTEING_PROTOCOL(rtm_protocol, RTPROT_RA      , "RDISC/ND Router Advertisements");
	PRINT_ROUTEING_PROTOCOL(rtm_protocol, RTPROT_MRT     , "Merit MRT");
	PRINT_ROUTEING_PROTOCOL(rtm_protocol, RTPROT_ZEBRA   , "Zebra");
	PRINT_ROUTEING_PROTOCOL(rtm_protocol, RTPROT_BIRD    , "BIRD");
	PRINT_ROUTEING_PROTOCOL(rtm_protocol, RTPROT_DNROUTED, "DECnet Routing Daemon");
	PRINT_ROUTEING_PROTOCOL(rtm_protocol, RTPROT_XORP    , "XORP");
	PRINT_ROUTEING_PROTOCOL(rtm_protocol, RTPROT_NTK     , "Netsukuku");
	PRINT_ROUTEING_PROTOCOL(rtm_protocol, RTPROT_DHCP    , "DHCP Client");
	PRINT_ROUTEING_PROTOCOL(rtm_protocol, RTPROT_MROUTED , "Multicast Daemon");
	PRINT_ROUTEING_PROTOCOL(rtm_protocol, RTPROT_BABEL   , "Babel Daemon");
	PRINT_ROUTEING_PROTOCOL(rtm_protocol, RTPROT_BGP     , "BGP Routes");
	PRINT_ROUTEING_PROTOCOL(rtm_protocol, RTPROT_ISIS    , "ISIS Routes");
	PRINT_ROUTEING_PROTOCOL(rtm_protocol, RTPROT_OSPF    , "OSPF Routes");
	PRINT_ROUTEING_PROTOCOL(rtm_protocol, RTPROT_RIP     , "RIP Routes");
	PRINT_ROUTEING_PROTOCOL(rtm_protocol, RTPROT_EIGRP   , "EIGRP Routes");
#undef PRINT_ROUTEING_PROTOCOL
	return;
}

void 
parse_route_origin(rtm_scope)
unsigned char rtm_scope;
{
	size_t i;

	printf("Route Origin: ");
	for (i = 0; i < sizeof(rtm_route_origin_map) / sizeof(struct origin_map); i++) {
		if (rtm_route_origin_map[i].scope == rtm_scope) {
			printf("%s (0x%02X) \n", rtm_route_origin_map[i].name, rtm_scope);
			return;
		}
	}
	printf("Unknown Scope Type (0x%02X)\n", rtm_scope);

	return;
}

void 
parse_route_type(rtm_type)
unsigned char rtm_type;
{
	size_t i;

	printf("Route Type: ");
	for (i = 0; i < sizeof(rtm_route_type_map) / sizeof(struct type_map); i++) {
		if (rtm_route_type_map[i].type == rtm_type) {
			printf("%s (0x%02X) \n", rtm_route_type_map[i].description, rtm_type);
			return;
		}
	}
	printf("Unknown Route Type (0x%02X)\n", rtm_type);
}

void
print_hex(data, len)
const unsigned char *data;
size_t len;
{
	unsigned int i;

	for (i = 0; i < len; i++)
		printf("%02x", data[i]);

	printf("\n");

	return;
}

void
print_ip_address(addr, family)
const void *addr;
unsigned char family;
{
	char buf[INET6_ADDRSTRLEN];

	memset(buf, '\0', sizeof(char) * INET6_ADDRSTRLEN);
	if (inet_ntop(family, addr, buf, sizeof(buf)) == NULL) {
		printf("Unknown");
		return;
	}
	printf("%s \n", buf);

	return;
}

/* Refer from linux/if_addr.h */
void
parse_rta_attributes(attr, rtm_family)
struct rtattr *attr;
unsigned char rtm_family;
{
	switch (attr->rta_type) {
		case RTA_UNSPEC:
			printf("RTA_UNSPEC: Ignore / Debug \n");
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: RTA_UNSPEC (0x%04X)\n", RTA_UNSPEC);
			break;
		case RTA_DST:
			printf("Route Destination Address: ");
			print_ip_address(RTA_DATA(attr), rtm_family);
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: RTA_DST (0x%04X)\n", RTA_DST);
			break;
		case RTA_SRC:
			printf("Route Source Address: ");
			print_ip_address(RTA_DATA(attr), rtm_family);
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: RTA_SRC (0x%04X)\n", RTA_SRC);
			break;
		case RTA_IIF: {
			unsigned int ifindex = *(unsigned int *)RTA_DATA(attr);
			char ifname[IF_NAMESIZE];

			memset(ifname, '\0', sizeof(char) * IF_NAMESIZE);
			printf("Input Interface Index: %s (%u) \n", (if_indextoname(ifindex, ifname))? ifname: "Unknown", ifindex);
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: RTA_IIF (0x%04X)\n", RTA_IIF);
			break;
		}
		case RTA_OIF: {
			unsigned int ifindex = *(unsigned int *)RTA_DATA(attr);
			char ifname[IF_NAMESIZE];

			memset(ifname, '\0', sizeof(char) * IF_NAMESIZE);
			printf("Output Interface Index: %s (%u) \n", (if_indextoname(ifindex, ifname))? ifname: "Unknown", ifindex);
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: RTA_OIF (0x%04X)\n", RTA_OIF);
			break;
		}
		case RTA_GATEWAY:
			printf("Gateway of the Route: ");
			print_ip_address(RTA_DATA(attr), rtm_family);
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: RTA_GATEWAY (0x%04X)\n", RTA_GATEWAY);
			break;
		case RTA_PRIORITY:
			/* RTA_PRIORITY 在路由表中代表的就是 Metric 值 */
			printf("Metric (Priority): %u \n", *(unsigned int *)RTA_DATA(attr));
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: RTA_PRIORITY (0x%04X)\n", RTA_PRIORITY);
			break;
		case RTA_PREFSRC:
			/* RTA_PREFSRC 存放的是該路由建議使用的來源 IP (Preferred Source) */
			printf("Preferred Source Address: ");
			print_ip_address(RTA_DATA(attr), rtm_family);
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: RTA_PREFSRC (0x%04X)\n", RTA_PREFSRC);
			break;
		case RTA_METRICS:
			printf("RTA_METRICS: \n");
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: RTA_METRICS (0x%04X)\n", RTA_METRICS);
			printf("  Data: ");
			/* 直接呼叫你程式中的 print_hex，把這包 Nested TLV 給倒出來 */
			print_hex(RTA_DATA(attr), RTA_PAYLOAD(attr));
			break;
		case RTA_MULTIPATH:
			/* 這是現代路由的核心：多路徑路由 (Equal-Cost Multi-Path, ECMP) */
			/* 它包含了一組 struct rtnexthop 結構，每個結構後面可能還跟著 RTA_GATEWAY 等屬性 */
			printf("Nested Nexthops: \n");
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: RTA_MULTIPATH (0x%04X)\n", RTA_MULTIPATH);
			printf("  Data: ");
			print_hex(RTA_DATA(attr), RTA_PAYLOAD(attr));
			break;
		case RTA_PROTOINFO:
			/* 此屬性在現代 Kernel 中已不再使用 (Deprecated) */
			printf("RTA_PROTOINFO: \n");
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: RTA_PROTOINFO (0x%04X)\n", RTA_PROTOINFO);
			printf("  Data: ");
			print_hex(RTA_DATA(attr), RTA_PAYLOAD(attr));
			break;
		case RTA_FLOW:
			/* 這是領域標籤 (Realm ID)，常用於流量控制 (TC) 或特定路由策略 */
			printf("RTA_FLOW (Realm ID): %u\n", *(unsigned int *)RTA_DATA(attr));
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: RTA_FLOW (0x%04X)\n", RTA_FLOW);
			break;
		case RTA_CACHEINFO: {
			/* RTA_CACHEINFO 對應的是 kernel 內的 struct rta_cacheinfo */
			struct rta_cacheinfo *ci = (struct rta_cacheinfo *)RTA_DATA(attr);

			printf("RTA_CACHEINFO: \n");
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: RTA_CACHEINFO (0x%04X)\n", RTA_CACHEINFO);
			printf("  Data: ");
			print_hex((unsigned char *)RTA_DATA(attr), RTA_PAYLOAD(attr));
			/* 路由被使用的次數 */
			printf("    User Count: %u \n", ci->rta_clntref);
			printf("    Last Use: %u \n", ci->rta_lastuse);
			/* 指出這條路由還剩下多少時間 (Jiffies) 會過期，若為 0 則通常表示永不過期 */
			printf("    Expires: %d \n", ci->rta_expires);
			printf("    Error: %u \n", ci->rta_error);
			/* 這條路由在快取中的狀態標記 */
			printf("    Used Count: %u \n", ci->rta_used);
			printf("    ID :%u \n", ci->rta_id);
			printf("    TS: %u \n", ci->rta_ts);
			printf("    TSAGE: %u \n", ci->rta_tsage);
			break;
		}
		case RTA_SESSION:
			/* 骨董級屬性：早期嘗試在路由層級處理 Session 資訊，現已廢棄 */
			printf("RTA_SESSION (Legacy/No longer used): \n");
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: RTA_SESSION (0x%04X)\n", RTA_SESSION);
			printf("  Data: ");
			print_hex(RTA_DATA(attr), RTA_PAYLOAD(attr));
			/* Refer: linux/rtnetlink.h: struct rta_session */
			break;
		case RTA_MP_ALGO:
			/* 骨董級屬性：早期用於指定 Multipath 的平衡演算法，現在由核心內部自動處理 */
			printf("RTA_MP_ALGO (Legacy/No longer used): \n");
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: RTA_MP_ALGO (0x%04X)\n", RTA_MP_ALGO);
			printf("  Data: ");
			print_hex(RTA_DATA(attr), RTA_PAYLOAD(attr));
			break;
		case RTA_TABLE: {
			unsigned int table = *(unsigned int *)RTA_DATA(attr);
			const char *str = (const char *)NULL;

			/* Refer from linux/rtnetlink.h, enum rt_class_t */
			switch (table) {
				case RT_TABLE_UNSPEC:     str = "unspec";     break;
				case RT_TABLE_COMPAT:     str = "compat";     break;
				case RT_TABLE_DEFAULT:    str = "default";    break;
				case RT_TABLE_MAIN:       str = "main";       break;
				case RT_TABLE_LOCAL:      str = "local";      break;
				default:                  str = "unknown";    break;
			}

			printf("RTA_TABLE: %s (%d) \n", str, table);
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: RTA_TABLE (0x%04X)\n", RTA_TABLE);
			printf("  Data: \n");
			print_hex(RTA_DATA(attr), RTA_PAYLOAD(attr));
			break;
		}
		case RTA_MARK:
			/* 路由標記 (fwmark)；核心可以根據 iptables/nftables 打上的標記來決定路由走勢 */
			printf("RTA_MARK (Firewall Mark): 0x%08X\n", *(unsigned int *)RTA_DATA(attr));
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: RTA_MARK (0x%04X)\n", RTA_MARK);
			break;
		case RTA_MFC_STATS: {
			/* 多播轉發快取統計 (Multicast Forwarding Cache Statistics) */
			/* 這對應到 struct rta_mfc_stats，包含封包計數與錯誤統計 */
			struct rta_mfc_stats *mfc = (struct rta_mfc_stats *)RTA_DATA(attr);
			
			printf("RTA_MFC_STATS (Multicast Stats): \n");
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: RTA_MFC_STATS (0x%04X)\n", RTA_MFC_STATS);
			printf("  Packets : %llu\n", (unsigned long long)mfc->mfcs_packets);
			printf("  Bytes   : %llu\n", (unsigned long long)mfc->mfcs_bytes);
			printf("  Wrong IF: %llu\n", (unsigned long long)mfc->mfcs_wrong_if);
			break;
		}
		case RTA_VIA: {
			/* RTA_VIA 對應的是 struct rtvia，用於跨協議族的下一跳 (e.g. IPv4 over IPv6) */
			struct rtvia *via = (struct rtvia *)RTA_DATA(attr);
			char via_addr[INET6_ADDRSTRLEN];

			printf("RTA_VIA (Nexthop via different AF): \n");
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: RTA_VIA (0x%04X) \n", RTA_VIA);
			/* 根據 via->rtvia_family 決定如何解析地址 */
			inet_ntop(via->rtvia_family, via->rtvia_addr, via_addr, sizeof(via_addr));
			printf("  Family: %u \n", via->rtvia_family);
			printf("  Address: %s \n", via_addr);
			break;
		}
		case RTA_NEWDST:
			/* 通常用於路由重定向或 MPLS 標籤交換後的新目的地 */
			printf("RTA_NEWDST: \n");
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: RTA_NEWDST (0x%04X)\n", RTA_NEWDST);
			printf("  Data: ");
			print_hex(RTA_DATA(attr), RTA_PAYLOAD(attr));
			break;
		case RTA_PREF:
			/* 這是 IPv6 的路由器選路偏好值 (Router Preference) */
			/* 通常是一個 char，代表 Low, Medium (default), High */
			printf("RTA_PREF (IPv6 Preference): 0x%02X \n", *(unsigned char *)RTA_DATA(attr));
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: RTA_PREF (0x%04X)\n", RTA_PREF);
			break;
		case RTA_ENCAP_TYPE: {
			/* 封裝類型：這告訴核心這條路由是否進入了隧道 (如 MPLS, LISP, IPSEC, BPF) */
			/* 這是一個 16 位元的整數 (unsigned short) */
			unsigned short type = *(unsigned short *)RTA_DATA(attr);
			unsigned char *str = (unsigned char *)NULL;

			switch(type) {
				case LWTUNNEL_ENCAP_NONE:       str = "NONE";       break;
				case LWTUNNEL_ENCAP_MPLS:       str = "MPLS";       break;
				case LWTUNNEL_ENCAP_IP:         str = "IP";         break;
				case LWTUNNEL_ENCAP_ILA:        str = "ILA";        break;
				case LWTUNNEL_ENCAP_IP6:        str = "IPv6";       break;
				case LWTUNNEL_ENCAP_SEG6:       str = "SEG6";       break;
				case LWTUNNEL_ENCAP_BPF:        str = "BPF";        break;
				case LWTUNNEL_ENCAP_SEG6_LOCAL: str = "SEG6 LOCAL"; break;
				default:                        str = "Unknown";    break;
			}
			printf("RTA_ENCAP_TYPE: %s (%u) \n", str, type);
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: RTA_ENCAP_TYPE (0x%04X)\n", RTA_ENCAP_TYPE);
			break;
		}
		case RTA_ENCAP:
			/* 這是巢狀屬性 (Nested Attribute)，包含了具體的隧道封裝資料 */
			/* 內部結構取決於先前的 RTA_ENCAP_TYPE (例如 MPLS 或 IP) */
			printf("RTA_ENCAP: \n");
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: RTA_ENCAP (0x%04X)\n", RTA_ENCAP);
			printf("  Data: ");
			print_hex(RTA_DATA(attr), RTA_PAYLOAD(attr));
			break;
		case RTA_EXPIRES: {
			/* 路由失效的剩餘時間 (以 Jiffies 為單位) */
			/* 這通常是一個 long (或是 32/64-bit int，視系統架構而定) */
			unsigned long expires = *(unsigned long *)RTA_DATA(attr);

			printf("RTA_EXPIRES (Timeout in Jiffies): %lu\n", expires);
			printf("RTA_EXPIRES: \n");
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: RTA_EXPIRES (0x%04X)\n", RTA_EXPIRES);
			break;
		}
		case RTA_PAD:
			/* 這是填充位元，用於確保後續的屬性能夠在 64-bit 邊界上對齊 */
			printf("RTA_PAD (Padding): \n");
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: RTA_PAD (0x%04X)\n", RTA_PAD);
			break;
		case RTA_UID:
			/* 路由所有者的 User ID */
			/* 在多用戶環境或特定安全策略下，這能識別是哪個 UID 建立了這條路由 */
			printf("RTA_UID (Owner UID): %u \n", *(unsigned int *)RTA_DATA(attr));
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: RTA_UID (0x%04X)\n", RTA_UID);
			break;
		case RTA_TTL_PROPAGATE:
			/* 決定是否將內部 IP 的 TTL 值複製到外部隧道標頭 (如 MPLS) */
			/* 這通常是一個 unsigned char (布林值：0 或 1) */
			printf("RTA_TTL_PROPAGATE: %s \n", (*(unsigned char *)RTA_DATA(attr))? "Enabled": "Disabled");
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: RTA_TTL_PROPAGATE (0x%04X)\n", RTA_TTL_PROPAGATE);
			break;
		case RTA_IP_PROTO:
			/* 路由關聯的 IP 協定編號 (例如 TCP=6, UDP=17) */
			/* 這在做「基於協定的路由決策 (Policy Routing)」時會出現 */
			printf("RTA_IP_PROTO: %u \n", *(unsigned char *)RTA_DATA(attr));
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: RTA_IP_PROTO (0x%04X)\n", RTA_IP_PROTO);
			break;
		case RTA_SPORT:
			/* 來源連接埠 (Source Port) */
			/* 通常是一個 16 位元的整數 (__u16) */
			printf("RTA_SPORT (Source Port): %u \n", *(unsigned short *)RTA_DATA(attr));
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: RTA_SPORT (0x%04X)\n", RTA_SPORT);
			break;
		case RTA_DPORT:
			/* 目的連接埠 (Destination Port) */
			printf("RTA_DPORT (Destination Port): %u \n", *(unsigned short *)RTA_DATA(attr));
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: RTA_DPORT (0x%04X)\n", RTA_DPORT);
			break;
		case RTA_NH_ID:
			/* 這是 Nexthop Object 的 ID */
			/* 現代核心允許將下一跳資訊獨立成物件，多條路由可以共用同一個 NH ID */
			printf("RTA_NH_ID (Nexthop Object ID): %u \n", *(unsigned int *)RTA_DATA(attr));
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: RTA_NH_ID (0x%04X)\n", RTA_NH_ID);
			break;
	}
	return;
}

