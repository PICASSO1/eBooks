/**
 * File Name: main.c
 * 
 * CopyLeft (C) 2026.
 *
 * Author: Pablo Picasso G.
 *
 * Version: 1.0.0.build040326
 *
 * Date: 2026 / 04 / 03
 *
 * Description: `nlh->nlmsg_type = RTM_GETADDR;`
 *
 * 1. 
 *
(*)?*/

#include "rtm_getaddr.h"

int 
main(argc, argv, envp)
int argc; 
char *argv[];
char **envp;
{
	int fd = -1, seq = 0, len = -1, one = 1;
	struct sockaddr_nl sa;
	struct ifaddrmsg *ifa = (struct ifaddrmsg *)NULL;
	char buffer[BUFSIZE];
	struct nlmsghdr *nlh = (struct nlmsghdr *)NULL;

	/* 建立 Netlink Socket；且子類別為 NETLINK_ROUTE (0) */
	fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
	if (fd < 0) {
		herror("socket");
		exit(EXIT_FAILURE);
	}

	/* 假如 kernel 有支援 Extended Ack，就會把更詳細的訊息給附加回來 */
	if (setsockopt(fd, SOL_NETLINK, NETLINK_EXT_ACK, &one, sizeof(one)) < 0) {
		herror("setsockopt(NETLINK_EXT_ACK)");
		exit(EXIT_FAILURE);
	}

	/* 這個設定讓成功的 不再回一整份原始的 Requested Payload，比較省 */
	if (setsockopt(fd, SOL_NETLINK, NETLINK_CAP_ACK, &one, sizeof(one)) < 0) {
		herror("setsockopt(NETLINK_EXT_ACK)");
		exit(EXIT_FAILURE);
	}

	/* Linux netlink (cooked header)
	 * Link-layer address type: Netlink (824)
	 * Family: Route (0x0000)
	 */
	bzero(&sa, sizeof(struct sockaddr_nl));
	sa.nl_family = AF_NETLINK;
	sa.nl_pad = 0;
	sa.nl_pid = getpid();
	sa.nl_groups = 0;                                                          /* 只接收 kernel / driver 回覆的 Unicast Reply */
	if (bind(fd, (struct sockaddr *)&sa, sizeof(struct sockaddr_nl)) < 0) {
		herror("bind");
		close(fd);
		exit(EXIT_FAILURE);
	}

	/* 準備 Netlink header message .... */
	nlh = (struct nlmsghdr *)buffer;
	nlh->nlmsg_len = NLMSG_LENGTH(sizeof(struct ifaddrmsg));
	nlh->nlmsg_type = RTM_GETADDR;                                             /* Message type: Get IP Address (22) */
	nlh->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK | NLM_F_DUMP;
	nlh->nlmsg_seq = ++seq;
	nlh->nlmsg_pid = getpid();

	/* NLMSG_DATA() 巨集的用意是將 nlh 原本的記憶體位址，加上 Netlink Header Length 之後，回傳給 ifa */
	ifa = (struct ifaddrmsg *)NLMSG_DATA(nlh);
	memset(ifa, 0x00, sizeof(struct ifaddrmsg));
	/* ifaddrmsg structure 有很多個成員，並非每一個都需要設定；視 nlh->nlmsg_flags 來決定 */
	ifa->ifa_family = AF_UNSPEC;

	/* 傳送訊息給 Kernel */
	if (send(fd, nlh, nlh->nlmsg_len, 0) < 0) {
		herror("send");
		close(fd);
		exit(EXIT_FAILURE);
	}

	/* 為回傳結果配置空間 */
	struct iovec iov = { buffer, sizeof(char) * BUFSIZE };
	struct msghdr msg = {
		.msg_name = &sa,
		.msg_namelen = sizeof(struct sockaddr_nl),
		.msg_iov = &iov,
		.msg_iovlen = 1,
	};

	/* 因為 kernel / driver 的回傳結果並不一定只有一條，所以使用 while (1) 來持續接收 */
	while (1) {
		len = recvmsg(fd, &msg, 0);
		if (len < 0) {
			herror("recvmsg");
			break;
		}

		/* 將 buffer 的記憶體位址，設定給 nlh；
		 * NLMSG_OK() 判斷是否已經讀取到最後乙筆 (NLMSG_DONE: 0x03) message；
		 * 假如不是最後乙筆 message，就繼續讀取下乙筆
		 */
		for (nlh = (struct nlmsghdr *)buffer; NLMSG_OK(nlh, len); nlh = NLMSG_NEXT(nlh, len)) {
			switch (nlh->nlmsg_type) {
				case NLMSG_NOOP:
					break;
				case NLMSG_ERROR:
					parse_nlmsg_err(nlh);
					goto out;
				case NLMSG_DONE:
					/* 接收完成，跳出 for loop & while loop*/
					goto out;
				case NLMSG_OVERRUN:
				default:
				break;
			}

			/* print nlmsg type */
			parse_nlmsg_type(nlh->nlmsg_type);
			puts("");

			/* print nlmsg flags */
			printf("Flags: 0x%04X \n", nlh->nlmsg_flags);
			printf("  Request: %X\n",            (nlh->nlmsg_flags & NLM_F_REQUEST) >> 0);
			printf("  Multipart Message: %X \n", (nlh->nlmsg_flags & NLM_F_MULTI) >> 1);
			printf("  Ack: %X \n",               (nlh->nlmsg_flags & NLM_F_ACK) >> 2);
			printf("  Echo: %X \n",              (nlh->nlmsg_flags & NLM_F_ECHO) >> 3);
			printf("  Dump Inconsistent: %X \n", (nlh->nlmsg_flags & NLM_F_DUMP_INTR) >> 4);
			printf("  Dump Filtered: %X \n",     (nlh->nlmsg_flags & NLM_F_DUMP_FILTERED) >> 5);

			/* 接收完乙筆 Netlink Route Protocol，開始解譯接下來的屬性 (attribute) */
			struct ifaddrmsg *ifa = (struct ifaddrmsg *)NLMSG_DATA(nlh);
			struct rtattr *attr = IFA_RTA(ifa);
			int attrlen = IFA_PAYLOAD(nlh);

			/* print addres type */
			parse_address_type(ifa->ifa_family);

			/* print address prefixlength */
			printf("Address prefixlength: %u \n", ifa->ifa_prefixlen);

			/* print address flags */
			parse_address_flags(ifa->ifa_flags);

			/* print address scope */
			printf("Address Scope: %u \n", ifa->ifa_scope);

			/* print interface index */
			printf("Interface Index: %u \n", ifa->ifa_index);

			/* RTA_OK() 判斷是否已經讀取到最後乙筆 attribute；否則就繼續讀取下乙筆屬性 RTA_NEXT() */
			for (; RTA_OK(attr, attrlen); attr = RTA_NEXT(attr, attrlen))
				parse_rta_attributes(attr, ifa->ifa_family);

			puts("");
		}
	}

out:
	close(fd);

	return 0;
}
