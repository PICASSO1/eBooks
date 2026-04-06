#include "rtm_getaddr.h"

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
parse_address_type(ifa_family)
__u8 ifa_family;
{
	size_t i = 0;

	printf("Address Type: ");
	for (i = 0; i < sizeof(address_type_map) / sizeof(struct address_type); i++) {
		if (address_type_map[i].type == ifa_family) {
			printf("%s (%u) \n", address_type_map[i].description, ifa_family);
			return;
		}
	}
	printf("Unknown address type (%u) \n", ifa_family);

	return;
}

/* Refer from linux/if_addr.h */
void
parse_address_flags(ifa_flags)
__u32 ifa_flags;
{
	int first = 1;

	printf("Address Flags: ");
#define PRINT_FLAGS(FLAG, STR) \
	do { \
		if (ifa_flags & (FLAG)) { \
			if (!first) \
				printf(", "); \
			printf(STR); \
			first = 0; \
		} \
	} while (0)
	PRINT_FLAGS(IFA_F_SECONDARY,      "secondary/temporary");
	PRINT_FLAGS(IFA_F_NODAD,          "nodad");
	PRINT_FLAGS(IFA_F_OPTIMISTIC,     "optimistic");
	PRINT_FLAGS(IFA_F_DADFAILED,      "dadfailed");
	PRINT_FLAGS(IFA_F_HOMEADDRESS,    "homeaddress");
	PRINT_FLAGS(IFA_F_DEPRECATED,     "deprecated");
	PRINT_FLAGS(IFA_F_TENTATIVE,      "tentative");
	PRINT_FLAGS(IFA_F_PERMANENT,      "permanent");
	PRINT_FLAGS(IFA_F_MANAGETEMPADDR, "managetempaddr");
	PRINT_FLAGS(IFA_F_NOPREFIXROUTE,  "noprefixroute");
	PRINT_FLAGS(IFA_F_MCAUTOJOIN,     "mcautojoin");
	PRINT_FLAGS(IFA_F_STABLE_PRIVACY, "stable-privacy");
#undef PRINT_FLAGS
	printf(" (0x%08X)\n", ifa_flags);
	return;
}

void
print_hex(data, len)
const unsigned char *data;
unsigned int len;
{
	size_t i;

	for (i = 0; i < len; i++)
		printf("%02x", data[i]);

	printf("\n");

	return;
}

void
print_ip_address(addr, family)
const void *addr;
__u8 family;
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
parse_rta_attributes(attr, ifa_family)
struct rtattr *attr;
__u8 ifa_family;
{
	switch (attr->rta_type) {
		case IFA_UNSPEC:
			printf("IFA_UNSPEC: Ignore / Debug \n");
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: IFA_UNSPEC (0x%04X)\n", IFA_UNSPEC);
			break;
		case IFA_ADDRESS:
			printf("Interface Address: ");
			print_ip_address(RTA_DATA(attr), ifa_family);
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: IFA_ADDRESS (0x%04X)\n", IFA_ADDRESS);
			break;
		case IFA_LOCAL:
			printf("Local Address: ");
			print_ip_address(RTA_DATA(attr), ifa_family);
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: IFA_LOCAL (0x%04X)\n", IFA_LOCAL);
			break;
		case IFA_LABEL:
			printf("Name of Interface: %s \n", (char *)RTA_DATA(attr));
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: IFA_LABEL (0x%04X)\n", IFA_LABEL);
			break;
		case IFA_BROADCAST:
			printf("Broadcast Address: ");
			print_ip_address(RTA_DATA(attr), ifa_family);
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: IFA_BROADCAST (0x%04X)\n", IFA_BROADCAST);
			break;
		case IFA_ANYCAST:
			printf("Anycast Address: ");
			print_ip_address(RTA_DATA(attr), ifa_family);
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: IFA_ANYCAST (0x%04X)\n", IFA_ANYCAST);
			break;
		case IFA_CACHEINFO: {
			struct ifa_cacheinfo ci;

			memset(&ci, 0x00, sizeof(struct ifa_cacheinfo));
			memcpy(&ci, RTA_DATA(attr), sizeof(struct ifa_cacheinfo));
			printf("Address Information: \n");
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: IFA_CACHEINFO (0x%04X)\n", IFA_CACHEINFO);
			printf("  Data: ");
			print_hex((unsigned char *)&ci, sizeof(struct ifa_cacheinfo));
			printf("    Preferred Lifetime: %u \n", ci.ifa_prefered);
			printf("    Valid Lifetime: %u \n", ci.ifa_valid);
			printf("    Created (cstamp): %u \n", ci.cstamp);
			printf("    Updated (tstamp): %u \n", ci.tstamp);
			break;
		}
		case IFA_MULTICAST:
			printf("Multicast Address: ");
			print_ip_address(RTA_DATA(attr), ifa_family);
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: IFA_MULTICAST (0x%04X)\n", IFA_MULTICAST);
			break;
		case IFA_FLAGS: {
			unsigned int flag = 0U;

			memcpy(&flag, RTA_DATA(attr), sizeof(unsigned int));
			parse_address_flags(flag);
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: IFA_FLAGS (0x%04X)\n", IFA_FLAGS);
			break;
		}
		case IFA_RT_PRIORITY:
			printf("IFA_RT_PRIORITY: %u \n", *(unsigned int *)RTA_DATA(attr));
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: IFA_RT_PRIORITY (0x%04X)\n", IFA_RT_PRIORITY);
			break;
		case IFA_TARGET_NETNSID:
			printf("IFA_TARGET_NETNSID: %d \n", *(int *)RTA_DATA(attr));
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: IFA_TARGET_NETNSID (0x%04X)\n", IFA_TARGET_NETNSID);
			break;
		default:
			printf("  Len: %u \n", attr->rta_len);
			printf("  Type: Unknown Attribute Type?? (0x%04X)\n", attr->rta_type);
			break;
	}
	return;
}
