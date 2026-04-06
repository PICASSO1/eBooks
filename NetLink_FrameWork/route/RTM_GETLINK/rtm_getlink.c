#include "rtm_getlink.h"

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
parse_interface_family(ifi_family)
unsigned char ifi_family;
{
	size_t i = 0;

	printf("Interface Family: ");
	for (i = 0; i < sizeof(interface_family_map) / sizeof(struct interface_family); i++) {
		if (interface_family_map[i].family == ifi_family) {
			printf("%s (%u) \n", interface_family_map[i].description, ifi_family);
			return;
		}
	}
	printf("Unknown interface family (%u) \n", ifi_family);

	return;
}


void
parse_device_type(ifi_type)
unsigned short ifi_type;
{
	size_t i = 0;

	printf("Device Type: ");
	for (i = 0; i < sizeof(device_type_map) / sizeof(struct device_type); i++) {
		if (device_type_map[i].type == ifi_type) {
			printf("%s (%u) \n", device_type_map[i].description, ifi_type);
			return;
		}
	}
	printf("Unknown device type (%u) \n", ifi_type);

	return;
}

void
parse_device_flags(ifi_flags)
unsigned ifi_flags;
{
	printf("Device Flags: ");
#define PRINT_FLAGS(FLAGS) \
	do { \
		if(ifi_flags & (FLAGS)) \
			printf(#FLAGS " "); \
	} while (0)
	PRINT_FLAGS(IFF_UP);
	PRINT_FLAGS(IFF_BROADCAST);
	PRINT_FLAGS(IFF_DEBUG);
	PRINT_FLAGS(IFF_LOOPBACK);
	PRINT_FLAGS(IFF_POINTOPOINT);
	PRINT_FLAGS(IFF_NOTRAILERS);
	PRINT_FLAGS(IFF_RUNNING);
	PRINT_FLAGS(IFF_NOARP);
	PRINT_FLAGS(IFF_PROMISC);
	PRINT_FLAGS(IFF_ALLMULTI);
	PRINT_FLAGS(IFF_MASTER);
	PRINT_FLAGS(IFF_SLAVE);
	PRINT_FLAGS(IFF_MULTICAST);
	PRINT_FLAGS(IFF_PORTSEL);
	PRINT_FLAGS(IFF_AUTOMEDIA);
	PRINT_FLAGS(IFF_DYNAMIC);
#undef PRINT_FLAGS
	printf("(0x%08X) \n", ifi_flags);

	return;
}

void
print_mac_address(mac, len)
const unsigned char *mac;
unsigned int len;
{
	unsigned int i = 0U;

	for (i = 0; i < len; i++) {
		printf("%02X", mac[i]);
		if (i != len - 1)
			printf(":");
	}
	printf("\n");

	return;
}

void
print_hex(data, len)
const unsigned char *data;
unsigned int len;
{
	unsigned int i;

	for (i = 0; i < len; i++)
		printf("%02x", data[i]);

	printf("\n");

	return;
}

void
parse_rta_attributes(attr)
struct rtattr *attr;
{
	switch (attr->rta_type) {
		case IFLA_UNSPEC:
			printf("IFLA_UNSPEC: Ignore / Debug \n");
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: IFLA_UNSPEC (0x%04X)\n", IFLA_UNSPEC);
			break;
		case IFLA_ADDRESS: {
			unsigned char *mac = (unsigned char *)RTA_DATA(attr);
			unsigned int mac_len = attr->rta_len - sizeof(struct rtattr);
				
			printf("HW Address: ");
			print_mac_address(mac, mac_len);
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: IFLA_ADDRESS (0x%04X)\n", IFLA_ADDRESS);
			break;
		}
		case IFLA_BROADCAST: {
			unsigned char *mac = (unsigned char *)RTA_DATA(attr);
			unsigned int mac_len = attr->rta_len - sizeof(struct rtattr);

			printf("Broadcast: ");
			print_mac_address(mac, mac_len);
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: IFLA_BROADCAST (0x%04X)\n", IFLA_BROADCAST);
			break;
		}
		case IFLA_IFNAME:
			printf("Device Name: %s\n", (char *)RTA_DATA(attr));
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: IFLA_IFNAME (0x%04X)\n", IFLA_IFNAME);
			break;
		case IFLA_MTU:
			printf("MTU: %u\n", *(unsigned int *)RTA_DATA(attr));
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: IFLA_MTU (0x%04X)\n", IFLA_MTU);
			break;
		case IFLA_LINK: {
			unsigned int ifindex = *(unsigned int *)RTA_DATA(attr);
			char ifname[IF_NAMESIZE];

			memset(ifname, '\0', sizeof(char) * IF_NAMESIZE);
			printf("IFLA_LINK: %s (%u)\n", (if_indextoname(ifindex, ifname))? ifname: "Unknown", ifindex);
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: IFLA_LINK (0x%04X)\n", IFLA_LINK);
			break;
		}
		case IFLA_QDISC:
			printf("Queueing Discipline: %s\n", (char *)RTA_DATA(attr));
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: IFLA_QDISC (0x%04X)\n", IFLA_QDISC);
			break;
		case IFLA_STATS: {
			printf("Interface Statistics: \n");
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: IFLA_STATS (0x%04X)\n", IFLA_STATS);

			struct rtnl_link_stats *st = (struct rtnl_link_stats *)RTA_DATA(attr);

			printf("  Rx packets           : %u\n", st->rx_packets);
			printf("  Tx packets           : %u\n", st->tx_packets);
			printf("  Rx bytes             : %u\n", st->rx_bytes);
			printf("  Tx bytes             : %u\n", st->tx_bytes);
			printf("  Rx errors            : %u\n", st->rx_errors);
			printf("  Tx errors            : %u\n", st->tx_errors);
			printf("  Rx dropped           : %u\n", st->rx_dropped);
			printf("  Tx dropped           : %u\n", st->tx_dropped);
			printf("  Multicast Rx         : %u\n", st->multicast);
			printf("  Collisions           : %u\n", st->collisions);
			printf("  Rx errors \n");
			printf("    Rx length errors   : %u\n", st->rx_length_errors);
			printf("    Rx over errors     : %u\n", st->rx_over_errors);
			printf("    Rx CRC errors      : %u\n", st->rx_crc_errors);
			printf("    Rx frame errors    : %u\n", st->rx_frame_errors);
			printf("    Rx FIFO errors     : %u\n", st->rx_fifo_errors);
			printf("    Rx missed errors   : %u\n", st->rx_missed_errors);
			printf("  Tx errors \n");      
			printf("    Tx aborted errors  : %u\n", st->tx_aborted_errors);
			printf("    Tx carrier errors  : %u\n", st->tx_carrier_errors);
			printf("    Tx FIFO errors     : %u\n", st->tx_fifo_errors);
			printf("    Tx heartbeat errors: %u\n", st->tx_heartbeat_errors);
			printf("    Tx window errors   : %u\n", st->tx_window_errors);
			break;
		}
		case IFLA_COST:	/* Legacy Attribute */
			printf("(IFLA_COST): %u\n", *(unsigned int *)RTA_DATA(attr));
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: IFLA_COST (0x%04X)\n", IFLA_COST);
			break;
		case IFLA_PRIORITY:	/* Legacy Attribute */
			printf("IFLA_PRIORITY: %u\n", *(unsigned int *)RTA_DATA(attr));
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: IFLA_PRIORITY (0x%04X)\n", IFLA_PRIORITY);
			break;
		case IFLA_MASTER: {
			unsigned int master = *(unsigned int *)RTA_DATA(attr);
			char ifname[IF_NAMESIZE];

			memset(ifname, '\0', sizeof(char) * IF_NAMESIZE);
			printf("IFLA_MASTER: %s (%u)\n", (if_indextoname(master, ifname))? ifname: "Unknown", master);
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: IFLA_MASTER (0x%04X)\n", IFLA_MASTER);
			break;
		}
		case IFLA_WIRELESS:	{	/* Wireless Extension event - see wireless.h */
			unsigned char *data = (unsigned char *)RTA_DATA(attr);
			unsigned int len = attr->rta_len - sizeof(struct rtattr);

			printf("IFLA_WIRELESS:\n");
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: IFLA_WIRELESS (0x%04X)\n", IFLA_WIRELESS);
			printf("  Data: ");
			print_hex(data, len);
			break;
		}
		case IFLA_PROTINFO:	{	/* Protocol specific information for a link */
			unsigned char *data = (unsigned char *)RTA_DATA(attr);
			int len = attr->rta_len - sizeof(struct rtattr);
			struct rtattr *pattr = (struct rtattr *)NULL;

			printf("IFLA_PROTINFO: \n");
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: IFLA_PROTINFO (0x%04X)\n", IFLA_PROTINFO);
			printf("  Data: ");
			print_hex(data, len);

			for (pattr = (struct rtattr *)RTA_DATA(attr); RTA_OK(pattr, len); pattr = RTA_NEXT(pattr, len)) {
				printf("    PROTINFO Attr Len : %u\n", pattr->rta_len);
				printf("    PROTINFO Attr Type: %u\n", pattr->rta_type);
			}
			break;
		}
		case IFLA_TXQLEN:
			printf("TxQueue Length: %u\n", *(unsigned int *)RTA_DATA(attr));
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: IFLA_TXQLEN (0x%04X)\n", IFLA_TXQLEN);
			break;
		case IFLA_MAP: {
			struct rtnl_link_ifmap *map = (struct rtnl_link_ifmap *)RTA_DATA(attr);

			printf("Map:\n");
			printf("  Len         : %u\n", attr->rta_len);
			printf("  Type        : IFLA_MAP (0x%04X)\n", IFLA_MAP);
			printf("  Memory start: 0x%016llx\n", (unsigned long long)map->mem_start);
			printf("  Memory end  : 0x%016llx\n", (unsigned long long)map->mem_end);
			printf("  Base address: 0x%016llx\n", (unsigned long long)map->base_addr);
			printf("  IRQ         : %u\n", map->irq);
			printf("  DMA         : %u\n", map->dma);
			printf("  Port        : %u\n", map->port);
			break;
		}
		case IFLA_WEIGHT:	/* Legacy Attribute */
			printf("IFLA_WEIGHT: %u\n", *(unsigned int *)RTA_DATA(attr));
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: IFLA_WEIGHT (0x%04X)\n", IFLA_WEIGHT);
			break;
		case IFLA_OPERSTATE: {
			unsigned char state = *(unsigned char *)RTA_DATA(attr);
			const char *str;

			switch (state) {
				case 0: str = "UNKNOWN";        break;
				case 1: str = "NOTPRESENT";     break;
				case 2: str = "DOWN";           break;
				case 3: str = "LOWERLAYERDOWN"; break;
				case 4: str = "TESTING";        break;
				case 5: str = "DORMANT";        break;
				case 6: str = "UP";             break;
				default: str = "INVALID";       break;
			}

			printf("Operstate: %s (%u)\n", str, state);
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: IFLA_OPERSTATE (0x%04X)\n", IFLA_OPERSTATE);
			break;
		}
		case IFLA_LINKMODE: {
			unsigned char mode = *(unsigned char *)RTA_DATA(attr);
			const char *str;

			switch (mode) {
				case 0: str = "DEFAULT";  break;
				case 1: str = "DORMANT";  break;
				default: str = "UNKNOWN"; break;
			}

			printf("Link Mode: %s (%u)\n", str, mode);
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: IFLA_LINKMODE (0x%04X)\n", IFLA_LINKMODE);
			break;
		}
		case IFLA_LINKINFO: {	/* This attribute describes what the "link Type / Kind" of this interface is. */
			unsigned char *data = (unsigned char *)RTA_DATA(attr);
			int linkinfo_len = attr->rta_len - sizeof(struct rtattr);
			struct rtattr *linfo_attr = (struct rtattr *)NULL;

			printf("IFLA_LINKINFO: \n");
			printf("  Len: %u\n", attr->rta_len);
			printf("  Type: IFLA_LINKINFO (0x%04X)\n", IFLA_LINKINFO);
			printf("  Data: ");
			print_hex(data, linkinfo_len);

			for (linfo_attr = (struct rtattr *)RTA_DATA(attr); RTA_OK(linfo_attr, linkinfo_len); linfo_attr = RTA_NEXT(linfo_attr, linkinfo_len)) {
				printf("    LINKINFO Attr Len : %u\n", linfo_attr->rta_len);
				printf("    LINKINFO Attr Type: 0x%04X\n", linfo_attr->rta_type);

				switch (linfo_attr->rta_type) {
					case IFLA_INFO_KIND:
						printf("    IFLA_INFO_KIND: %s\n", (char *)RTA_DATA(linfo_attr));
						break;
					case IFLA_INFO_DATA:
						printf("    IFLA_INFO_DATA:\n");
						printf("      Data: ");
						print_hex((unsigned char *)RTA_DATA(linfo_attr), linfo_attr->rta_len - sizeof(struct rtattr));
						break;
					case IFLA_INFO_XSTATS:
						printf("    IFLA_INFO_XSTATS:\n");
						printf("      Data: ");
						print_hex((unsigned char *)RTA_DATA(linfo_attr), linfo_attr->rta_len - sizeof(struct rtattr));
						break;
					case IFLA_INFO_SLAVE_KIND:
						printf("    IFLA_INFO_SLAVE_KIND: %s\n", (char *)RTA_DATA(linfo_attr));
						break;
					case IFLA_INFO_SLAVE_DATA:
						printf("    IFLA_INFO_SLAVE_DATA:\n");
						printf("      Data: ");
						print_hex((unsigned char *)RTA_DATA(linfo_attr), linfo_attr->rta_len - sizeof(struct rtattr));
						break;
					default:
						printf("    Unknown LINKINFO Attr\n");
						break;
				}
			}
			break;
		}
		case IFLA_NET_NS_PID:	/* Uses the PID to represent the namespace */
			printf("IFLA_NET_NS_PID: %u", *(unsigned int *)RTA_DATA(attr));
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: IFLA_NET_NS_PID (0x%04X)\n", IFLA_NET_NS_PID);
			break;
		case IFLA_IFALIAS:	/* Interface alias name */
			printf("IFLA_IFALIAS: %s \n", (char *)RTA_DATA(attr));
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: IFLA_IFALIAS (0x%04X)\n", IFLA_IFALIAS);
			break;
		case IFLA_NUM_VF:		/* Number of VFs if device is SR-IOV PF */
			printf("IFLA_NUM_VF: %u \n", *(int *)RTA_DATA(attr));
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: IFLA_NUM_VF (0x%04X)\n", IFLA_NUM_VF);
			break;
		case IFLA_VFINFO_LIST: {
			unsigned char *data = (unsigned char *)RTA_DATA(attr);
			int len = attr->rta_len - sizeof(struct rtattr);
			struct rtattr *vf_attr;

			printf("(IFLA_VFINFO_LIST): \n");
			printf("  Len: %u\n", attr->rta_len);
			printf("  Type: IFLA_VFINFO_LIST (0x%04X)\n", IFLA_VFINFO_LIST);
			printf("  Data: ");
			print_hex(data, len);

			for (vf_attr = (struct rtattr *)RTA_DATA(attr); RTA_OK(vf_attr, len); vf_attr = RTA_NEXT(vf_attr, len)) {
				int vf_len = vf_attr->rta_len - sizeof(struct rtattr);
				struct rtattr *vf_info = (struct rtattr *)NULL;

				printf("    VF Entry:\n");
				printf("      Len : %u\n", vf_attr->rta_len);
				printf("      Type: %u\n", vf_attr->rta_type);

				for (vf_info = (struct rtattr *)RTA_DATA(vf_attr); RTA_OK(vf_info, vf_len); vf_info = RTA_NEXT(vf_info, vf_len)) {
					printf("        VF Attr Type: %u\n", vf_info->rta_type);
					printf("        VF Attr Len : %u\n", vf_info->rta_len);

					printf("        Data: ");
					print_hex((unsigned char *)RTA_DATA(vf_info), vf_info->rta_len - sizeof(struct rtattr));
				}
			}
			break;
		}
		case IFLA_STATS64: {
			printf("Stats64:\n");
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: IFLA_STATS64 (0x%04X)\n", IFLA_STATS64);

			struct rtnl_link_stats64 *st64 = (struct rtnl_link_stats64 *)RTA_DATA(attr);

			printf("  Rx packets           : %llu\n", st64->rx_packets);
			printf("  Tx packets           : %llu\n", st64->tx_packets);
			printf("  Rx bytes             : %llu\n", st64->rx_bytes);
			printf("  Tx bytes             : %llu\n", st64->tx_bytes);
			printf("  Rx errors            : %llu\n", st64->rx_errors);
			printf("  Tx errors            : %llu\n", st64->tx_errors);
			printf("  Rx dropped           : %llu\n", st64->rx_dropped);
			printf("  Tx dropped           : %llu\n", st64->tx_dropped);
			printf("  Multicast Rx         : %llu\n", st64->multicast);
			printf("  Collisions           : %llu\n", st64->collisions);
			printf("  Rx errors \n");
			printf("    Rx length errors   : %llu\n", st64->rx_length_errors);
			printf("    Rx over errors     : %llu\n", st64->rx_over_errors);
			printf("    Rx CRC errors      : %llu\n", st64->rx_crc_errors);
			printf("    Rx frame errors    : %llu\n", st64->rx_frame_errors);
			printf("    Rx FIFO errors     : %llu\n", st64->rx_fifo_errors);
			printf("    Rx missed errors   : %llu\n", st64->rx_missed_errors);
			printf("  Tx errors \n");      
			printf("    Tx aborted errors  : %llu\n", st64->tx_aborted_errors);
			printf("    Tx carrier errors  : %llu\n", st64->tx_carrier_errors);
			printf("    Tx FIFO errors     : %llu\n", st64->tx_fifo_errors);
			printf("    Tx heartbeat errors: %llu\n", st64->tx_heartbeat_errors);
			printf("    Tx window errors   : %llu\n", st64->tx_window_errors);
			break;
		}
		case IFLA_VF_PORTS: {
			unsigned char *data = (unsigned char *)RTA_DATA(attr);
			int len = attr->rta_len - sizeof(struct rtattr);
			struct rtattr *vf_port_attr = (struct rtattr *)NULL;

			printf("(IFLA_VF_PORTS): \n");
			printf("  Len: %u\n", attr->rta_len);
			printf("  Type: IFLA_VF_PORTS (0x%04X)\n", IFLA_VF_PORTS);
			printf("  Data: ");
			print_hex(data, len);

			for (vf_port_attr = (struct rtattr *)RTA_DATA(attr); RTA_OK(vf_port_attr, len); vf_port_attr = RTA_NEXT(vf_port_attr, len)) {
				int port_len = vf_port_attr->rta_len - sizeof(struct rtattr);
				struct rtattr *port_info = (struct rtattr *)NULL;

				printf("    VF PORT Entry:\n");
				printf("      Len : %u\n", vf_port_attr->rta_len);
				printf("      Type: %u\n", vf_port_attr->rta_type);

				for (port_info = (struct rtattr *)RTA_DATA(vf_port_attr); RTA_OK(port_info, port_len); port_info = RTA_NEXT(port_info, port_len)) {
					printf("        PORT Attr Len : %u\n", port_info->rta_len);
					printf("        PORT Attr Type: %u\n", port_info->rta_type);
					printf("        Data: ");
					print_hex((unsigned char *)RTA_DATA(port_info), port_info->rta_len - sizeof(struct rtattr));
				}
			}
			break;
		}
		case IFLA_PORT_SELF: {
			unsigned char *data = (unsigned char *)RTA_DATA(attr);
			int len = attr->rta_len - sizeof(struct rtattr);
			struct rtattr *port_attr = (struct rtattr *)NULL;

			printf("(IFLA_PORT_SELF): \n");
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: IFLA_PORT_SELF (0x%04X)\n", IFLA_PORT_SELF);
			printf("  Data: ");
			print_hex(data, len);

			for (port_attr = (struct rtattr *)RTA_DATA(attr); RTA_OK(port_attr, len); port_attr = RTA_NEXT(port_attr, len)) {
				printf("    PORT_SELF Attr Len : %u\n", port_attr->rta_len);
				printf("    PORT_SELF Attr Type: %u\n", port_attr->rta_type);
				printf("    Data: ");
				print_hex((unsigned char *)RTA_DATA(port_attr), port_attr->rta_len - sizeof(struct rtattr));
			}
			break;
		}
		case IFLA_AF_SPEC: {
			unsigned char *data = (unsigned char *)RTA_DATA(attr);
			int af_len = attr->rta_len - sizeof(struct rtattr);
			struct rtattr *af_attr = (struct rtattr *)NULL;

			printf("AF SPEC:\n");
			printf("  Len: %u\n", attr->rta_len);
			printf("  Type: IFLA_AF_SPEC (0x%04X)\n", IFLA_AF_SPEC);
			printf("  Data: ");
			print_hex(data, af_len);

			for (af_attr = (struct rtattr *)RTA_DATA(attr); RTA_OK(af_attr, af_len); af_attr = RTA_NEXT(af_attr, af_len)) {
				printf("    AF Attr Len : %u\n", af_attr->rta_len);
				printf("    AF Attr Type: 0x%04X (%u)\n", af_attr->rta_type, af_attr->rta_type);

				switch (af_attr->rta_type) {
					case AF_INET:
						printf("    AF_INET\n");
						break;
					case AF_INET6:
						printf("    AF_INET6\n");
						break;
					default:
						printf("    UNKNOWN AF\n");
						break;
				}
			}
			break;
		}
		case IFLA_GROUP:		/* Group the device belongs to */
			printf("Group: %u\n", *(unsigned int *)RTA_DATA(attr));
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: IFLA_GROUP (0x%04X)\n", IFLA_GROUP);
			break;
		case IFLA_NET_NS_FD:	/* Uses the file descriptor to represent the namespace */
			printf("IFLA_NET_NS_FD: %u \n", *(unsigned int *)RTA_DATA(attr));
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: IFLA_NET_NS_FD (0x%04X)\n", IFLA_NET_NS_FD);
			break;
		case IFLA_EXT_MASK:		/* Extended info mask, VFs, etc */
			printf("IFLA_EXT_MASK: 0x%08X \n", *(unsigned int *)RTA_DATA(attr));
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: IFLA_EXT_MASK (0x%04X)\n", IFLA_EXT_MASK);
			break;
		case IFLA_PROMISCUITY:	/* Promiscuity count: > 0 means acts PROMISC */
			printf("Promiscuity: %u\n", *(unsigned int *)RTA_DATA(attr));
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: IFLA_PROMISCUITY (0x%04X)\n", IFLA_PROMISCUITY);
			break;
		case IFLA_NUM_TX_QUEUES:
			printf("Number of Tx Queues: %u\n", *(unsigned int *)RTA_DATA(attr));
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: IFLA_NUM_TX_QUEUES (0x%04X)\n", IFLA_NUM_TX_QUEUES);
			break;
		case IFLA_NUM_RX_QUEUES:
			printf("Number of Rx Queues: %u\n", *(unsigned int *)RTA_DATA(attr));
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: IFLA_NUM_RX_QUEUES (0x%04X)\n", IFLA_NUM_RX_QUEUES);
			break;
		case IFLA_CARRIER:
			printf("Carrier: %s\n", (*(unsigned char *)RTA_DATA(attr))? "Restricted": "Not Present");
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: IFLA_CARRIER (0x%04X)\n", IFLA_CARRIER);
			break;
		case IFLA_PHYS_PORT_ID: {
			unsigned char *data = (unsigned char *)RTA_DATA(attr);
			unsigned int len = attr->rta_len - sizeof(struct rtattr);

			printf("(IFLA_PHYS_PORT_ID): \n");
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: IFLA_PHYS_PORT_ID (0x%04X)\n", IFLA_PHYS_PORT_ID);
			printf("  Data: ");
			print_hex(data, len);
			break;
		}
		case IFLA_CARRIER_CHANGES:
			printf("Carrier Changes: %u\n", *(unsigned int *)RTA_DATA(attr));
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: IFLA_CARRIER_CHANGES (0x%04X)\n", IFLA_CARRIER_CHANGES);
			break;
		case IFLA_PHYS_SWITCH_ID: {
			unsigned char *data = (unsigned char *)RTA_DATA(attr);
			unsigned int len = attr->rta_len - sizeof(struct rtattr);
		
			printf("IFLA_PHYS_SWITCH_ID: \n");
			printf("  Len: %u\n", attr->rta_len);
			printf("  Type: IFLA_PHYS_SWITCH_ID (0x%04X)\n", IFLA_PHYS_SWITCH_ID);
			
			printf("  Data: ");
			print_hex(data, len);
			break;
		}
		case IFLA_LINK_NETNSID:
			printf("IFLA_LINK_NETNSID: %d \n", *(int *)RTA_DATA(attr));
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: IFLA_LINK_NETNSID (0x%04X)\n", IFLA_LINK_NETNSID);
			break;
		case IFLA_PHYS_PORT_NAME:
			printf("IFLA_PHYS_PORT_NAME: %s \n", (char *)RTA_DATA(attr));
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: IFLA_PHYS_PORT_NAME (0x%04X)\n", IFLA_PHYS_PORT_NAME);
			break;
		case IFLA_PROTO_DOWN:
			printf("IFLA_PROTO_DOWN: \n");
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: IFLA_PROTO_DOWN (0x%04X)\n", IFLA_PROTO_DOWN);
			printf("  Data: %02X \n", *(unsigned char *)RTA_DATA(attr));
			break;
		case IFLA_GSO_MAX_SEGS:
			printf("Maximum GSO Segment Count: %u\n", *(unsigned int *)RTA_DATA(attr));
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: IFLA_GSO_MAX_SEGS (0x%04X)\n", IFLA_GSO_MAX_SEGS);
			break;
		case IFLA_GSO_MAX_SIZE:
			printf("Maximum GSO Size: %u\n", *(unsigned int *)RTA_DATA(attr));
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: IFLA_GSO_MAX_SIZE (0x%04X)\n", IFLA_GSO_MAX_SIZE);
			break;
		case IFLA_PAD:
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: IFLA_PAD (0x%04X)\n", IFLA_PAD);
			break;
		case IFLA_XDP: {
			unsigned char *data = (unsigned char *)RTA_DATA(attr);
			unsigned int len = attr->rta_len - sizeof(struct rtattr);
			int xdp_len = len;
			struct rtattr *xdp_attr = (struct rtattr *)NULL;

			printf("IFLA_XDP:\n");
			printf("  Len: %u\n", attr->rta_len);
			printf("  Type: IFLA_XDP (0x%04X)\n", IFLA_XDP);
			printf("  Data: ");
			print_hex(data, len);

			for (xdp_attr = (struct rtattr *)RTA_DATA(attr); RTA_OK(xdp_attr, xdp_len); xdp_attr = RTA_NEXT(xdp_attr, xdp_len)) {
				printf("    XDP Attr Len : %u\n", xdp_attr->rta_len);
				printf("    XDP Attr Type: 0x%04X\n", xdp_attr->rta_type);
				switch (xdp_attr->rta_type) {
					case IFLA_XDP_UNSPEC:
						printf("    IFLA_XDP_UNSPEC\n");
						break;
					case IFLA_XDP_FD:
						printf("    IFLA_XDP_FD: %d\n", *(int *)RTA_DATA(xdp_attr));
						break;
					case IFLA_XDP_ATTACHED: {
						unsigned char attached = *(unsigned char *)RTA_DATA(xdp_attr);

						printf("    IFLA_XDP_ATTACHED: %u", attached);
						switch (attached) {
							case 0:
								printf(" (XDP_ATTACHED_NONE)\n");
								break;
							case 1:
								printf(" (XDP_ATTACHED_DRV)\n");
								break;
							case 2:
								printf(" (XDP_ATTACHED_SKB)\n");
								break;
							case 3:
								printf(" (XDP_ATTACHED_HW)\n");
								break;
							case 4:
								printf(" (XDP_ATTACHED_MULTI)\n");
								break;
							default:
								printf(" (UNKNOWN)\n");
								break;
						}
						break;
					}
					case IFLA_XDP_FLAGS:
						printf("    IFLA_XDP_FLAGS: 0x%08X\n", *(unsigned int *)RTA_DATA(xdp_attr));
						break;
					case IFLA_XDP_PROG_ID:
						printf("    IFLA_XDP_PROG_ID: %u\n", *(unsigned int *)RTA_DATA(xdp_attr));
						break;
					case IFLA_XDP_DRV_PROG_ID:
						printf("    IFLA_XDP_DRV_PROG_ID: %u\n", *(unsigned int *)RTA_DATA(xdp_attr));
						break;
					case IFLA_XDP_SKB_PROG_ID:
						printf("    IFLA_XDP_SKB_PROG_ID: %u\n", *(unsigned int *)RTA_DATA(xdp_attr));
						break;
					case IFLA_XDP_HW_PROG_ID:
						printf("    IFLA_XDP_HW_PROG_ID: %u\n", *(unsigned int *)RTA_DATA(xdp_attr));
						break;
					default:
						printf("    Unknown XDP Attr\n");
						break;
				}
			}
			break;
		}
		case IFLA_EVENT:{
			unsigned int event = *(unsigned int *)RTA_DATA(attr);
			const char *str;

			switch (event) {
				case 0: str = "NONE";             break;
				case 1: str = "REBOOT";           break;
				case 2: str = "FEATURES";         break;
				case 3: str = "BONDING_FAILOVER"; break;
				case 4: str = "NOTIFY_PEERS";     break;
				case 5: str = "IGMP_RESEND";      break;
				case 6: str = "BONDING_OPTIONS";  break;
				default: str = "UNKNOWN";         break;
			}

			printf("IFLA_EVENT: %s (%u)\n", str, event);
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: IFLA_EVENT (0x%04X)\n", IFLA_EVENT);
			break;
		}
		case IFLA_NEW_NETNSID:
			printf("IFLA_NEW_NETNSID: %d \n", *(int *)RTA_DATA(attr));
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: IFLA_NEW_NETNSID (0x%04X)\n", IFLA_NEW_NETNSID);
			break;
		case IFLA_IF_NETNSID:
			printf("IFLA_IF_NETNSID: %d \n", *(int *)RTA_DATA(attr));
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: IFLA_TARGET_NETNSID | IFLA_IF_NETNSID (0x%04X)\n", IFLA_IF_NETNSID);
			break;
		case IFLA_CARRIER_UP_COUNT:
			printf("Carrier Up Count: %u\n", *(unsigned int *)RTA_DATA(attr));
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: IFLA_CARRIER_UP_COUNT (0x%04X)\n", IFLA_CARRIER_UP_COUNT);
			break;
		case IFLA_CARRIER_DOWN_COUNT:
			printf("Carrier Down Count: %u\n", *(unsigned int *)RTA_DATA(attr));
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: IFLA_CARRIER_DOWN_COUNT (0x%04X)\n", IFLA_CARRIER_DOWN_COUNT);
			break;
		case IFLA_NEW_IFINDEX: {
			unsigned int ifindex = *(unsigned int *)RTA_DATA(attr);
			char ifname[IF_NAMESIZE];

			memset(ifname, '\0', sizeof(char) * IF_NAMESIZE);
			printf("(IFLA_IF_NETNSID): %s (%u)\n", (if_indextoname(ifindex, ifname))? ifname: "Unknown", ifindex);
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: IFLA_NEW_IFINDEX (0x%04X)\n", IFLA_NEW_IFINDEX);
			break;
		}
		case IFLA_MIN_MTU:
			printf("Minimum MTU: %u\n", *(unsigned int *)RTA_DATA(attr));
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: IFLA_MIN_MTU (0x%04X)\n", IFLA_MIN_MTU);
			break;
		case IFLA_MAX_MTU:
			printf("Maximum MTU: %u\n", *(unsigned int *)RTA_DATA(attr));
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: IFLA_MAX_MTU (0x%04X)\n", IFLA_MAX_MTU);
			break;
		default:
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: Unknown Attribute Type?? (0x%04X)\n", attr->rta_type);
			break;
	}
	return;
}
