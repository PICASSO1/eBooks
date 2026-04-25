/**
 * File Name: main.c
 * 
 * CopyLeft (C) 2026.
 *
 * Author: Pablo Picasso G.
 *
 * Version: 1.0.1.build042526
 *
 * Date: 2026 / 04 / 25
 *
 * Description: 
 *
 * 01. 第一次撰寫 NetLink / nl80211 的通訊協定；這是一個極其複雜的 Framework，第一次寫加了不少的註解
 * 02. 開發及驗證環境如 README 所描述
 * 03. Wireless 比 Ethernet 複雜許多，像 `enum nl80211_commands` 這一類的 Command ID 轉成 String 也沒有寫，不想花時間
 * 04. LOGs 的輸出排版也不花時間在這上面，因為 Attributes 實在是太多了！
 * 05. 其餘的程式碼架構在註解上都已經寫得很清楚了。
 * 06. NetLink Framework 極其重要，除了 Wireless 之外，BlueTooth (BLE)、ZigBee 都能夠支援，多理解是件好事
 * 07. ioctl 已經是上個世代的通訊界面，擴展性很差，難以維護 / 移植。
 * 08. 喔，對了！因為 nl80211 需要搭配 Libary ，所以 Makefile 還需要加上：
 *    `gcc -Wall -Wextra -O2 -o main main.c $(pkg-config --cflags --libs libnl-3.0 libnl-genl-3.0)`
 * 
 * 09. 修正 Makefile ，新增 $(DEBUG) 參數：-ggdb -Wall -Wextra -O2
 * 10. 把一些函式加上回傳值的判斷，方便日後的 debug。
 *
(*)?*/

#include "get_interface.h"

static int init_nl80211(struct nl80211_state *);
static void deinit_nl80211(struct nl80211_state *);
static int dump_interfaces(struct nl80211_state *);

static int valid_handler(struct nl_msg *, void *);

int 
main(argc, argv, envp)
int argc;
char *argv[];
char **envp;
{
    (void)argc;
    (void)argv;
    (void)envp;
    int ret = -1;
    struct nl80211_state state;

    bzero(&state, sizeof(struct nl80211_state));
    if (init_nl80211(&state) < 0) {
        exit(EXIT_FAILURE);
    }

    ret = dump_interfaces(&state);
    deinit_nl80211(&state);

    if (ret < 0) {
        exit(EXIT_FAILURE);
    }

    return 0;
}

static int 
init_nl80211(state)
struct nl80211_state *state;
{
    int ret = -1;

    /* nl_socket_alloc(); 的工作是配置一個 libnl 用的 socket wrapper object，後面可以拿它去 connect、send、receive netlink message。 */
    state->sock = nl_socket_alloc();
    if (state->sock == (struct nl_sock *)NULL) {
        fprintf(stderr, "nl_socket_alloc() failed. \n");

        return -ENOMEM;
    }

    /* genl_connect(); 做的是把剛剛那個 nl_sock 連到 Generic Netlink。因為 nl80211 本身就是一個 Generic Netlink Family。 */
    ret = genl_connect(state->sock);
    if (ret < 0) {
        fprintf(stderr, "genl_connect() failed: %s (%d). \n", nl_geterror(ret), ret);
        deinit_nl80211(state);

        return ret;
    }

    /* genl_ctrl_resolve(); 詢問 Generic Netlink： "nl80211" 這個子集合的 Family ID 是多少？ */
    state->nl80211_id = genl_ctrl_resolve(state->sock, "nl80211");
    if (state->nl80211_id < 0) {
        fprintf(stderr, "genl_ctrl_resolve(\"nl80211\") failed: %s (%d)\n", nl_geterror(state->nl80211_id), state->nl80211_id);
        deinit_nl80211(state);

        return state->nl80211_id;
    }

    return 0;
}

static int 
dump_interfaces(state)
struct nl80211_state *state;
{
    int ret = -1;
    struct nl_msg *msg = NULL;
    struct nl_cb *cb = NULL;
    struct cb_context ctx;

    bzero(&ctx, sizeof(struct cb_context));

    /* 替 msg 配置一塊記憶體空間 */
    msg = nlmsg_alloc();
    if (msg == NULL) {
        fprintf(stderr, "nlmsg_alloc() failed. \n");

        return -ENOMEM;
    }

    /* Defined in: /usr/include/libnl3/netlink/handlers.h */
    cb = nl_cb_alloc(NL_CB_DEFAULT);
    if (cb == NULL) {
        fprintf(stderr, "nl_cb_alloc() failed. \n");
        nlmsg_free(msg);
        msg = NULL;

        return -ENOMEM;
    }

    /* Create Netlink Header + Generic Netlink header
     * void *
     * genlmsg_put(msg, port, seq, family, hdrlen, flags, cmd, version)
     * struct nl_msg *msg;
     * uint32_t port;         // Port ID
     * uint32_t seq;          // Sequence
     * int family;            // Family ID: 0x22 (nl80211)
     * int hdrlen;
     * int flags;
     * uint8_t cmd;           // like: NL80211_CMD_GET_INTERFACE (5)
     * uint8_t version;       // Family Version
     */
    if (!genlmsg_put(msg, NL_AUTO_PORT, NL_AUTO_SEQ, state->nl80211_id, 0, NLM_F_DUMP, NL80211_CMD_GET_INTERFACE, 0)) {
        fprintf(stderr, "genlmsg_put() failed. \n");
        nl_cb_put(cb);
        cb = NULL;
        nlmsg_free(msg);
        msg = NULL;

        return -ENOBUFS;
    }

    /* 當收到不同種類的 netlink message 時，要呼叫那些對應的 callback function，並傳入甚麼 context */
    /* int nl_cb_set(cb, type, kind, func, arg)
     * struct nl_cb *cb;            // Allocate by nl_cb_alloc();
     * enum nl_cb_type type;        // Defined in: /usr/include/libnl3/netlink/handlers.h
     * enum nl_cb_kind kind;        // Defined in: /usr/include/libnl3/netlink/handlers.h
     * nl_recvmsg_msg_cb_t func;    // Callback function
     * void *arg;                   // Callback function arguments
     */
    ret = nl_cb_err(cb, NL_CB_CUSTOM, error_handler, &ctx);
    if (ret < 0) {
        fprintf(stderr, "nl_cb_err(error_handler) failed: %s (%d) \n", nl_geterror(ret), ret);
        nl_cb_put(cb);
        cb = NULL;
        nlmsg_free(msg);
        msg = NULL;

        return ret;
    }

    ret = nl_cb_set(cb, NL_CB_VALID,     NL_CB_CUSTOM, valid_handler, (void *)&ctx);
    if (ret < 0) {
        fprintf(stderr, "nl_cb_set(valid_handler) failed: %s (%d) \n", nl_geterror(ret), ret);
        nl_cb_put(cb);
        cb = NULL;
        nlmsg_free(msg);
        msg = NULL;

        return ret;
    }

    ret = nl_cb_set(cb, NL_CB_FINISH,    NL_CB_CUSTOM, finish_handler, (void *)&ctx);
    if (ret < 0) {
        fprintf(stderr, "nl_cb_set(finish_handler) failed: %s (%d) \n", nl_geterror(ret), ret);
        nl_cb_put(cb);
        cb = NULL;
        nlmsg_free(msg);
        msg = NULL;

        return ret;
    }

    ret = nl_cb_set(cb, NL_CB_ACK,       NL_CB_CUSTOM,  ack_handler, (void *)&ctx);
    if (ret < 0) {
        fprintf(stderr, "nl_cb_set(ack_handler) failed: %s (%d) \n", nl_geterror(ret), ret);
        nl_cb_put(cb);
        cb = NULL;
        nlmsg_free(msg);
        msg = NULL;

        return ret;
    }

    ret = nl_send_auto(state->sock, msg);
    if (ret < 0) {
        fprintf(stderr, "nl_send_auto() failed: %s (%d) \n", nl_geterror(ret), ret);
        nl_cb_put(cb);
        cb = NULL;
        nlmsg_free(msg);
        msg = NULL;

        return ret;
    }

    /* 假如還沒有回傳到最後一筆資料，就持續地接收封包；而接收回來的封包會在 NL_CB_VALID 所註冊的 callback 函式處理 */
    while (ctx.finish == 0) {
        ret = nl_recvmsgs(state->sock, cb);
        if (ret < 0) {
            fprintf(stderr, "nl_recvmsgs() failed: %s (%d)\n", nl_geterror(ret), ret);
            nl_cb_put(cb);
            cb = NULL;
            nlmsg_free(msg);
            msg = NULL;
            break;
        }
    }

    if (ctx.err) {
        fprintf(stderr, "netlink error: %s (%d)\n", nl_geterror(ctx.err), ctx.err);
        ret = ctx.err;
    }

    nl_cb_put(cb);
    cb = NULL;
    nlmsg_free(msg);
    msg = NULL;

    return ret;
}

static int
valid_handler(msg, arg)
struct nl_msg *msg;
void *arg;
{   /* In order to avoid the compiler: "warning: unused parameter ‘msg’" when CFLAGs including "-Wall -Wextra"  */
    (void)arg;
    struct nlmsghdr *nlh = NULL;
    struct genlmsghdr *gnlh = NULL;
    int ret = -1;

    /* 準備一個 attribute 指標表：宣告一個名為 tb 的陣列，而陣列裡的每一個元素都是指到 (struct nlattr *) 的指標 */
    struct nlattr *tb[NL80211_ATTR_MAX + 1] = { 0 };
    /* 宣告一個 attribute 的「驗證規則表」，裡面的每一成員都是 struct nla_policy */
    static struct nla_policy policy[NL80211_ATTR_MAX + 1] = {
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
        [NL80211_ATTR_SSID]                 = { .type = NLA_UNSPEC }
    };

    /* 取得 Netlink message 的 Header 位址；這是 Netlink Protocol 最外層的表頭 */
    nlh = nlmsg_hdr(msg);
    if (!nlh) {
        fprintf(stderr, "nlmsg_hdr() failed.\n");

        return NL_SKIP;
    }

    /* 因為 Netlink Protocol 的 Data (Payload)，就是子協定 Generic Netlink 表頭；官方建議直接使用 genlmsg_hdr(); 較為直覺 */
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

    /* 已經知道了 Generic Netlink 的資料在哪兒了，開始解譯 attribute */
    ret = nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0), genlmsg_attrlen(gnlh, 0), policy);
    if (ret < 0) {
        fprintf(stderr, "nla_parse() failed: %s (%d).\n", nl_geterror(ret), ret);

        return NL_SKIP;
    }

    if (tb[NL80211_ATTR_IFINDEX])
        printf("NL80211_ATTR_IFINDEX (%d): %u \n", NL80211_ATTR_IFINDEX, nla_get_u32(tb[NL80211_ATTR_IFINDEX]));

    if (tb[NL80211_ATTR_IFNAME])
        printf("NL80211_ATTR_IFNAME (%d) : %s \n", NL80211_ATTR_IFNAME, nla_get_string(tb[NL80211_ATTR_IFNAME]));

    if (tb[NL80211_ATTR_WIPHY])
        printf("NL80211_ATTR_WIPHY (%d): wiphy#%u \n", NL80211_ATTR_WIPHY, nla_get_u32(tb[NL80211_ATTR_WIPHY]));

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

    printf("\n");

    return NL_SKIP;
}

static void
deinit_nl80211(state)
struct nl80211_state *state;
{
    if (state && state->sock) {
        nl_socket_free(state->sock);
        state->sock = NULL;
    }
	return;
}
