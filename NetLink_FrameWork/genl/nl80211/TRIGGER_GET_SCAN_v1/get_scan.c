#include "main.h"
#include "get_scan.h"

int
get_error_handler(nla, err, arg)
struct sockaddr_nl *nla;
struct nlmsgerr *err;
void *arg;
{
    (void)nla;
    struct get_cb_context *ctx = (struct get_cb_context *)arg;

    ctx->err = err->error;
    ctx->done = 1;

    return NL_STOP;
}

int
get_valid_handler(msg, arg)
struct nl_msg *msg;
void *arg;
{
    (void)arg;
    struct nlmsghdr *nlh = NULL;
    struct genlmsghdr *gnlh = NULL;
    int ret = -1;

    struct nlattr *tb[NL80211_ATTR_MAX + 1] = { 0 };
    struct nlattr *bss[NL80211_BSS_MAX + 1] = { 0 };
    static struct nla_policy bss_policy[NL80211_BSS_MAX + 1] = {
        [NL80211_BSS_BSSID]                 = { .type = NLA_UNSPEC },
        [NL80211_BSS_FREQUENCY]             = { .type = NLA_U32 },
#ifdef NL80211_BSS_FREQUENCY_OFFSET
        [NL80211_BSS_FREQUENCY_OFFSET]      = { .type = NLA_U32 },
#endif  /* NL80211_BSS_FREQUENCY_OFFSET */
        [NL80211_BSS_TSF]                   = { .type = NLA_U64 },
        [NL80211_BSS_BEACON_INTERVAL]       = { .type = NLA_U16 },
        [NL80211_BSS_CAPABILITY]            = { .type = NLA_U16 },
        [NL80211_BSS_INFORMATION_ELEMENTS]  = { .type = NLA_UNSPEC },
        [NL80211_BSS_SIGNAL_MBM]            = { .type = NLA_U32 },
        [NL80211_BSS_SIGNAL_UNSPEC]         = { .type = NLA_U8 },
        [NL80211_BSS_STATUS]                = { .type = NLA_U32 },
        [NL80211_BSS_SEEN_MS_AGO]           = { .type = NLA_U32 },
        [NL80211_BSS_BEACON_IES]            = { .type = NLA_UNSPEC },
        [NL80211_BSS_CHAN_WIDTH]            = { .type = NLA_U32 },
        [NL80211_BSS_BEACON_TSF]            = { .type = NLA_U64 },
        [NL80211_BSS_PRESP_DATA]            = { .type = NLA_FLAG },
        [NL80211_BSS_LAST_SEEN_BOOTTIME]    = { .type = NLA_U64 },
        [NL80211_BSS_PARENT_TSF]            = { .type = NLA_U64 },
        [NL80211_BSS_PARENT_BSSID]          = { .type = NLA_UNSPEC },
        [NL80211_BSS_CHAIN_SIGNAL]          = { .type = NLA_NESTED },
    };
    static struct nla_policy scan_policy[NL80211_ATTR_MAX + 1] = {
        [NL80211_ATTR_IFINDEX]              = { .type = NLA_U32 },
        [NL80211_ATTR_IFNAME]               = { .type = NLA_STRING },
        [NL80211_ATTR_IFTYPE]               = { .type = NLA_U32 },
        [NL80211_ATTR_WIPHY]                = { .type = NLA_U32 },
        [NL80211_ATTR_WDEV]                 = { .type = NLA_U64 },
        [NL80211_ATTR_MAC]                  = { .type = NLA_UNSPEC },
        [NL80211_ATTR_GENERATION]           = { .type = NLA_U32 },
        [NL80211_ATTR_4ADDR]                = { .type = NLA_U8  },
        [NL80211_ATTR_WIPHY_FREQ]           = { .type = NLA_U32 },
        [NL80211_ATTR_WIPHY_CHANNEL_TYPE]   = { .type = NLA_U32 },
        [NL80211_ATTR_CHANNEL_WIDTH]        = { .type = NLA_U32 },
        [NL80211_ATTR_CENTER_FREQ1]         = { .type = NLA_U32 },
        [NL80211_ATTR_WIPHY_TX_POWER_LEVEL] = { .type = NLA_U32 },
        [NL80211_ATTR_SSID]                 = { .type = NLA_UNSPEC },
        [NL80211_ATTR_BSS]                  = { .type = NLA_NESTED },
    };

    nlh = nlmsg_hdr(msg);
    if (!nlh) {
        fprintf(stderr, "nlmsg_hdr() failed.\n");

        return NL_SKIP;
    }

    gnlh = genlmsg_hdr(nlh);
/*  gnlh = nlmsg_data(nlh); */
    if (!gnlh) {
        fprintf(stderr, "nlmsg_data() failed.\n");

        return NL_SKIP;
    }

    printf("Generic Family ID: %s (0x%04X) \n", "nl80211", nlh->nlmsg_type);
    printf("  Length  : %u\n", nlh->nlmsg_len);
    printf("  Flags   : 0x%04x\n", nlh->nlmsg_flags);
    printf("    Request          : %X \n", !!((nlh->nlmsg_flags) & NLM_F_REQUEST));
    printf("    Multipart Message: %X \n", !!((nlh->nlmsg_flags) & NLM_F_MULTI));
    printf("    Ack:             : %X \n", !!((nlh->nlmsg_flags) & NLM_F_ACK));
    printf("    Echo:            : %X \n", !!((nlh->nlmsg_flags) & NLM_F_ECHO));
    printf("    Dump Inconsistent: %X \n", !!((nlh->nlmsg_flags) & 0x0010));
    printf("    Dump Filtered    : %X \n", !!((nlh->nlmsg_flags) & 0x0020));
    printf("  Sequence: %u\n", nlh->nlmsg_seq);
    printf("  Port ID : %u\n", nlh->nlmsg_pid);
    /* 開始分析 Generic Netlink Header 的資料 */
    printf("  Command : %s (%u)\n", cmd_to_string(gnlh->cmd), gnlh->cmd);
    printf("  Version : %u\n", gnlh->version);
    printf("  Reserved: %u\n", gnlh->reserved);

    ret = nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0), genlmsg_attrlen(gnlh, 0), scan_policy);
    if (ret < 0) {
        fprintf(stderr, "nla_parse() failed: %s (%d).\n", nl_geterror(ret), ret);

        return NL_SKIP;
    }

    if (tb[NL80211_ATTR_IFINDEX]) {
        printf("NL80211_ATTR_IFINDEX (%d): %u \n", NL80211_ATTR_IFINDEX, nla_get_u32(tb[NL80211_ATTR_IFINDEX]));
    }

    if (tb[NL80211_ATTR_IFNAME]) {
        printf("NL80211_ATTR_IFNAME (%d) : %s \n", NL80211_ATTR_IFNAME, nla_get_string(tb[NL80211_ATTR_IFNAME]));
    }

    if (tb[NL80211_ATTR_WIPHY]) {
        printf("NL80211_ATTR_WIPHY (%d): wiphy#%u \n", NL80211_ATTR_WIPHY, nla_get_u32(tb[NL80211_ATTR_WIPHY]));
    }

    if (tb[NL80211_ATTR_IFTYPE]) {
        enum nl80211_iftype iftype = nla_get_u32(tb[NL80211_ATTR_IFTYPE]);
        printf("NL80211_ATTR_IFTYPE (%d) : %u (%s) \n", NL80211_ATTR_IFTYPE, iftype, iftype_to_string(iftype));
    }

    if (tb[NL80211_ATTR_WDEV]) {
        printf("NL80211_ATTR_WDEV (%d): 0x%016llx (%llu) \n", NL80211_ATTR_WDEV, \
            (unsigned long long)nla_get_u64(tb[NL80211_ATTR_WDEV]), (unsigned long long)nla_get_u64(tb[NL80211_ATTR_WDEV]));
    }

    if (tb[NL80211_ATTR_MAC]) {
        printf("NL80211_ATTR_MAC (%d): ", NL80211_ATTR_MAC );
        print_mac(nla_data(tb[NL80211_ATTR_MAC]), nla_len(tb[NL80211_ATTR_MAC]));
        printf("\n");
    }

    if (tb[NL80211_ATTR_GENERATION]) {
        uint32_t generation = nla_get_u32(tb[NL80211_ATTR_GENERATION]);
        printf("NL80211_ATTR_GENERATION (%d) : %u\n", NL80211_ATTR_GENERATION, generation);
    }

    if (tb[NL80211_ATTR_4ADDR]) {
        uint8_t use_4addr = nla_get_u8(tb[NL80211_ATTR_4ADDR]);
        printf("NL80211_ATTR_4ADDR (%d) : %u\n", NL80211_ATTR_4ADDR, use_4addr);
    }

    if (tb[NL80211_ATTR_WIPHY_FREQ]) {
        uint32_t freq = nla_get_u32(tb[NL80211_ATTR_WIPHY_FREQ]);
        printf("NL80211_ATTR_WIPHY_FREQ (%d) : %u MHz\n", NL80211_ATTR_WIPHY_FREQ, freq);
    }

    if (tb[NL80211_ATTR_WIPHY_CHANNEL_TYPE]) {
        uint32_t chan_type = nla_get_u32(tb[NL80211_ATTR_WIPHY_CHANNEL_TYPE]);
        printf("NL80211_ATTR_WIPHY_CHANNEL_TYPE (%d) : %u (%s)\n", NL80211_ATTR_WIPHY_CHANNEL_TYPE, chan_type, channel_type_to_string(chan_type));
    }

    if (tb[NL80211_ATTR_CHANNEL_WIDTH]) {
        uint32_t width = nla_get_u32(tb[NL80211_ATTR_CHANNEL_WIDTH]);
        printf("NL80211_ATTR_CHANNEL_WIDTH (%d) : %u (%s)\n", NL80211_ATTR_CHANNEL_WIDTH, width, channel_width_to_string(width));
    }

    if (tb[NL80211_ATTR_CENTER_FREQ1]) {
        uint32_t center1 = nla_get_u32(tb[NL80211_ATTR_CENTER_FREQ1]);
        printf("NL80211_ATTR_CENTER_FREQ1 (%d) : %u MHz\n", NL80211_ATTR_CENTER_FREQ1, center1);
    }

    if (tb[NL80211_ATTR_WIPHY_TX_POWER_LEVEL]) {
        uint32_t tx_power = nla_get_u32(tb[NL80211_ATTR_WIPHY_TX_POWER_LEVEL]);
        printf("NL80211_ATTR_WIPHY_TX_POWER_LEVEL (%d) : %u (mBm) / %.2f dBm \n", NL80211_ATTR_WIPHY_TX_POWER_LEVEL, tx_power, tx_power / 100.0);
    }

    if (tb[NL80211_ATTR_SSID]) {
        int ssid_len = nla_len(tb[NL80211_ATTR_SSID]);
        unsigned char *ssid = nla_data(tb[NL80211_ATTR_SSID]);

        printf("NL80211_ATTR_SSID (%d) : ", NL80211_ATTR_SSID);
        print_ssid(ssid, ssid_len);
        printf("\n");
    }

    /* 因為回傳的 BSS 是巢狀屬性，在解譯前需判斷是否有東西？ */
    if (tb[NL80211_ATTR_BSS]) {
        printf("NL80211_ATTR_BSS (%d): Len: %d \n", NL80211_ATTR_BSS, nla_len(tb[NL80211_ATTR_BSS]));
    }
    else {
        fprintf(stderr, "NL80211_ATTR_BSS not found \n");

        return NL_SKIP;
    }

    /* 假如有東西才開始解譯巢狀裡的資料 */
    ret = nla_parse_nested(bss, NL80211_BSS_MAX, tb[NL80211_ATTR_BSS], bss_policy);
    if (ret < 0) {
        fprintf(stderr, "nla_parse_nested() failed: %s (%d) \n", nl_geterror(ret), ret);

        return NL_SKIP;
    }

    if (bss[NL80211_BSS_BSSID]) {
        printf("  NL80211_BSS_BSSID (%d): ", NL80211_BSS_BSSID);
        print_mac(nla_data(bss[NL80211_BSS_BSSID]), nla_len(bss[NL80211_BSS_BSSID]));
        printf("\n");
    }

    if (bss[NL80211_BSS_FREQUENCY]) {
        printf("  NL80211_BSS_FREQUENCY (%d): %u MHz\n", NL80211_BSS_FREQUENCY, nla_get_u32(bss[NL80211_BSS_FREQUENCY]));
    }

#ifdef NL80211_BSS_FREQUENCY_OFFSET
    if (bss[NL80211_BSS_FREQUENCY_OFFSET]) {
        printf("  NL80211_BSS_FREQUENCY_OFFSET (%d): %u\n", NL80211_BSS_FREQUENCY_OFFSET, nla_get_u32(bss[NL80211_BSS_FREQUENCY_OFFSET]));
    }
#endif  /* NL80211_BSS_FREQUENCY_OFFSET */

    if (bss[NL80211_BSS_TSF]) {
        printf("  NL80211_BSS_TSF (%d): 0x%llX (%llu)\n", NL80211_BSS_TSF, (unsigned long long)nla_get_u64(bss[NL80211_BSS_TSF]), (unsigned long long)nla_get_u64(bss[NL80211_BSS_TSF]));
    }

    if (bss[NL80211_BSS_BEACON_INTERVAL]) {
        printf("  NL80211_BSS_BEACON_INTERVAL (%d): %u TUs\n", NL80211_BSS_BEACON_INTERVAL, nla_get_u16(bss[NL80211_BSS_BEACON_INTERVAL]));
    }

    if (bss[NL80211_BSS_CAPABILITY]) {
        printf("  NL80211_BSS_CAPABILITY (%d): 0x%04x\n", NL80211_BSS_CAPABILITY, nla_get_u16(bss[NL80211_BSS_CAPABILITY]));
    }

    /* NL80211_BSS_INFORMATION_ELEMENTS 是一個巢狀屬性 */
    if (bss[NL80211_BSS_INFORMATION_ELEMENTS]) {
        printf("  NL80211_BSS_INFORMATION_ELEMENTS (%d): Len: %d \n", NL80211_BSS_INFORMATION_ELEMENTS, nla_len(bss[NL80211_BSS_INFORMATION_ELEMENTS]));
    }

    if (bss[NL80211_BSS_SIGNAL_MBM]) {
        int32_t signal_mbm = (int32_t)nla_get_u32(bss[NL80211_BSS_SIGNAL_MBM]);

        printf("  NL80211_BSS_SIGNAL_MBM (%d): %.2f dBm\n", NL80211_BSS_SIGNAL_MBM, signal_mbm / 100.0);
    }

    if (bss[NL80211_BSS_SIGNAL_UNSPEC]) {
        uint8_t signal_unspec = nla_get_u8(bss[NL80211_BSS_SIGNAL_UNSPEC]);

        printf("  NL80211_BSS_SIGNAL_UNSPEC (%d): %u/100 relative\n", NL80211_BSS_SIGNAL_UNSPEC, signal_unspec);
    }

    if (bss[NL80211_BSS_STATUS]) {
        printf("  NL80211_BSS_STATUS (%d): %u\n", NL80211_BSS_STATUS, nla_get_u32(bss[NL80211_BSS_STATUS]));
    }

    if (bss[NL80211_BSS_SEEN_MS_AGO]) {
        printf("  NL80211_BSS_SEEN_MS_AGO (%d): %u ms\n", NL80211_BSS_SEEN_MS_AGO, nla_get_u32(bss[NL80211_BSS_SEEN_MS_AGO]));
    }

    /* NL80211_BSS_BEACON_IES 也是一個巢狀屬性 */
    if (bss[NL80211_BSS_BEACON_IES]) {
        printf("  NL80211_BSS_BEACON_IES (%d): Len: %d \n", NL80211_BSS_BEACON_IES, nla_len(bss[NL80211_BSS_BEACON_IES]));
    }

    if (bss[NL80211_BSS_CHAN_WIDTH]) {
        printf("  NL80211_BSS_CHAN_WIDTH (%d): %s (%u)\n", NL80211_BSS_CHAN_WIDTH, bss_width_to_string(nla_get_u32(bss[NL80211_BSS_CHAN_WIDTH])), nla_get_u32(bss[NL80211_BSS_CHAN_WIDTH]));
    }

    if (bss[NL80211_BSS_BEACON_TSF]) {
        printf("  NL80211_BSS_BEACON_TSF (%d): 0x%llx (%llu)\n", NL80211_BSS_BEACON_TSF, (unsigned long long)nla_get_u64(bss[NL80211_BSS_BEACON_TSF]), \
            (unsigned long long)nla_get_u64(bss[NL80211_BSS_BEACON_TSF]));
    }

    if (bss[NL80211_BSS_PRESP_DATA]) {
        printf("  NL80211_BSS_PRESP_DATA (%d): \n", NL80211_BSS_PRESP_DATA);
    }

    if (bss[NL80211_BSS_LAST_SEEN_BOOTTIME]) {
        printf("  NL80211_BSS_LAST_SEEN_BOOTTIME (%d): 0x%llX (%llu)\n", NL80211_BSS_LAST_SEEN_BOOTTIME, (unsigned long long)nla_get_u64(bss[NL80211_BSS_LAST_SEEN_BOOTTIME]), \
            (unsigned long long)nla_get_u64(bss[NL80211_BSS_LAST_SEEN_BOOTTIME]));
    }

    if (bss[NL80211_BSS_PAD]) {
        printf("  NL80211_BSS_PAD (%d): present, len=%d\n", NL80211_BSS_PAD, nla_len(bss[NL80211_BSS_PAD]));
    }

    if (bss[NL80211_BSS_PARENT_TSF]) {
        printf("  NL80211_BSS_PARENT_TSF (%d): %llu\n", NL80211_BSS_PARENT_TSF, (unsigned long long)nla_get_u64(bss[NL80211_BSS_PARENT_TSF]));
    }

    if (bss[NL80211_BSS_PARENT_BSSID]) {
        printf("  NL80211_BSS_PARENT_BSSID (%d): ", NL80211_BSS_PARENT_BSSID); print_mac(nla_data(bss[NL80211_BSS_PARENT_BSSID]), nla_len(bss[NL80211_BSS_PARENT_BSSID]));
        printf("\n");
    }

    if (bss[NL80211_BSS_CHAIN_SIGNAL]) {
        struct nlattr *chain = NULL;
        int rem;

        printf("  NL80211_BSS_CHAIN_SIGNAL (%d):\n", NL80211_BSS_CHAIN_SIGNAL);
        nla_for_each_nested(chain, bss[NL80211_BSS_CHAIN_SIGNAL], rem) {
            int chain_id = nla_type(chain);
            int8_t signal = (int8_t)nla_get_u8(chain);

            printf("    chain[%d] signal: %d dBm\n", chain_id, signal);
        }
    }

    return NL_SKIP;
}

int
get_finish_handler(msg, arg)
struct nl_msg *msg;
void *arg;
{
    (void)msg;
    struct get_cb_context *ctx = (struct get_cb_context *)arg;

    ctx->done = 1;

    return NL_SKIP;
}

int
get_ack_handler(msg, arg)
struct nl_msg *msg;
void *arg;
{
    (void)msg;
    struct get_cb_context *ctx = arg;

    ctx->done = 1;

    return NL_STOP;
}

int 
nl80211_get_scan(state, ifname)
struct nl80211_state *state;
const char *ifname;
{
    int ret = -1;
    struct nl_msg *msg = NULL;
    struct nl_cb *cb = NULL;
    struct get_cb_context ctx;
    unsigned int ifindex = 0;

    bzero(&ctx, sizeof(struct get_cb_context));

    ifindex = if_nametoindex (ifname);
    if (ifindex == 0) {
        fprintf(stderr, "if_nametoindex() failed!! \n");
        ret = -ENODEV;

        goto finish;
    }

    msg = nlmsg_alloc();
    if (msg == NULL) {
        fprintf(stderr, "nlmsg_alloc() failed. \n");
        ret = -ENOMEM;

        goto finish;
    }

    cb = nl_cb_alloc(NL_CB_DEFAULT);
    if (cb == NULL) {
        fprintf(stderr, "nl_cb_alloc() failed. \n");
        ret = -ENOMEM;

        goto finish;
    }

    /* NL80211_CMD_GET_SCAN 屬於 dump request；所以第６個參數必須設定 NLM_F_DUMP */
    if (!genlmsg_put(msg, NL_AUTO_PORT, NL_AUTO_SEQ, state->nl80211_id, 0, NLM_F_DUMP, NL80211_CMD_GET_SCAN, 0)) {
        fprintf(stderr, "genlmsg_put() failed. \n");
        ret = -ENOBUFS;

        goto finish;
    }

    ret = nla_put_u32(msg, NL80211_ATTR_IFINDEX, (uint32_t)ifindex);
    if (ret < 0) {
        fprintf(stderr, "nla_put_u32(NL80211_ATTR_IFINDEX) failed: %s (%d) \n", nl_geterror(ret), ret);

        goto finish;
    }

    ret = nl_cb_err(cb, NL_CB_CUSTOM, get_error_handler, &ctx);
    if (ret < 0) {
        fprintf(stderr, "nl_cb_err(get_error_handler) failed: %s (%d) \n", nl_geterror(ret), ret);

        goto finish;
    }

    ret = nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, get_valid_handler, (void *)NULL);
    if (ret < 0 ) {
        fprintf(stderr, "nl_cb_set(get_valid_handler) failed: %s (%d) \n", nl_geterror(ret), ret);

        goto finish;        
    }

    ret = nl_cb_set(cb, NL_CB_FINISH,    NL_CB_CUSTOM, get_finish_handler, (void *)&ctx);
    if (ret < 0) {
        fprintf(stderr, "nl_cb_set(get_finish_handler) failed: %s (%d) \n", nl_geterror(ret), ret);

        goto finish;
    }

    ret = nl_cb_set(cb, NL_CB_ACK, NL_CB_CUSTOM, get_ack_handler, &ctx);
    if (ret < 0) {
        fprintf(stderr, "nl_cb_set(get_ack_handler) failed: %s (%d) \n", nl_geterror(ret), ret);

        goto finish;
    }

    ret = nl_send_auto(state->sock, msg);
    if (ret < 0) {
        fprintf(stderr, "nl_send_auto() failed: %s (%d) \n", nl_geterror(ret), ret);

        goto finish;
    }

    while (ctx.done != 1) {
        ret = nl_recvmsgs(state->sock, cb);
        if (ret < 0) {
            fprintf(stderr, "nl_recvmsgs() failed: %s (%d)\n", nl_geterror(ret), ret);
            break;
        }
    }

    if (ctx.err != 0) {
        fprintf(stderr, "netlink error: %s (%d)\n", strerror(-ctx.err), ctx.err);
        ret = ctx.err;
    }

finish:
    if (cb != NULL) {
        nl_cb_put(cb);
        cb = NULL;
    }

    if (msg != NULL) {
        nlmsg_free(msg);
        msg = NULL;
    }

    return ret;
}
