#include "main.h"
#include "get_wiphy.h"

int
error_handler(nla, err, arg)
struct sockaddr_nl *nla;
struct nlmsgerr *err;
void *arg;
{
    (void)nla;
    (void)err;
    struct cb_context *ctx = (struct cb_context *)arg;

    ctx->err = err->error;
    ctx->done = 1;

    return NL_STOP;
}

int
valid_handler(msg, arg)
struct nl_msg *msg;
void *arg;
{
    (void)arg;
    struct nlmsghdr *nlh = NULL;
    struct genlmsghdr *gnlh = NULL;
    int ret = -1;

    struct nlattr *tb[NL80211_ATTR_MAX + 1] = { 0 };
    static struct nla_policy wiphy_policy[NL80211_ATTR_MAX + 1] = {
        [NL80211_ATTR_WIPHY]                          = { .type = NLA_U32 },
        [NL80211_ATTR_WIPHY_NAME]                     = { .type = NLA_STRING },
        [NL80211_ATTR_GENERATION]                     = { .type = NLA_U32 },

        [NL80211_ATTR_WIPHY_RETRY_SHORT]              = { .type = NLA_U8 },
        [NL80211_ATTR_WIPHY_RETRY_LONG]               = { .type = NLA_U8 },
        [NL80211_ATTR_WIPHY_FRAG_THRESHOLD]           = { .type = NLA_U32 },
        [NL80211_ATTR_WIPHY_RTS_THRESHOLD]            = { .type = NLA_U32 },
        [NL80211_ATTR_WIPHY_COVERAGE_CLASS]           = { .type = NLA_U8 },

        [NL80211_ATTR_MAX_NUM_SCAN_SSIDS]             = { .type = NLA_U8 },
        [NL80211_ATTR_MAX_NUM_SCHED_SCAN_SSIDS]       = { .type = NLA_U8 },
        [NL80211_ATTR_MAX_SCAN_IE_LEN]                = { .type = NLA_U16 },
        [NL80211_ATTR_MAX_SCHED_SCAN_IE_LEN]          = { .type = NLA_U16 },
        [NL80211_ATTR_MAX_MATCH_SETS]                 = { .type = NLA_U8 },

        [NL80211_ATTR_SUPPORT_IBSS_RSN]               = { .type = NLA_FLAG },
        [NL80211_ATTR_SUPPORT_MESH_AUTH]              = { .type = NLA_FLAG },
        [NL80211_ATTR_SUPPORT_AP_UAPSD]               = { .type = NLA_FLAG },
        [NL80211_ATTR_TDLS_SUPPORT]                   = { .type = NLA_FLAG },
        [NL80211_ATTR_TDLS_EXTERNAL_SETUP]            = { .type = NLA_FLAG },

        [NL80211_ATTR_CIPHER_SUITES]                  = { .type = NLA_UNSPEC },
        [NL80211_ATTR_MAX_NUM_PMKIDS]                 = { .type = NLA_U8 },
        [NL80211_ATTR_CONTROL_PORT_ETHERTYPE]         = { .type = NLA_FLAG },

        [NL80211_ATTR_WIPHY_ANTENNA_AVAIL_TX]         = { .type = NLA_U32 },
        [NL80211_ATTR_WIPHY_ANTENNA_AVAIL_RX]         = { .type = NLA_U32 },

        [NL80211_ATTR_MAX_REMAIN_ON_CHANNEL_DURATION] = { .type = NLA_U32 },
        [NL80211_ATTR_OFFCHANNEL_TX_OK]               = { .type = NLA_FLAG },
        [NL80211_ATTR_FEATURE_FLAGS]                  = { .type = NLA_U32 },
        [NL80211_ATTR_HT_CAPABILITY_MASK]             = { .type = NLA_UNSPEC },

        [NL80211_ATTR_SOFTWARE_IFTYPES]               = { .type = NLA_NESTED },
        [NL80211_ATTR_SUPPORTED_COMMANDS]             = { .type = NLA_NESTED },
        [NL80211_ATTR_SUPPORTED_IFTYPES]              = { .type = NLA_NESTED },
        [NL80211_ATTR_INTERFACE_COMBINATIONS]         = { .type = NLA_NESTED },
        [NL80211_ATTR_WIPHY_BANDS]                    = { .type = NLA_NESTED },
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

    ret = nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0), genlmsg_attrlen(gnlh, 0), wiphy_policy);
    if (ret < 0) {
        fprintf(stderr, "nla_parse() failed: %s (%d).\n", nl_geterror(ret), ret);

        return NL_SKIP;
    }

    if (tb[NL80211_ATTR_WIPHY]) {
        printf("NL80211_ATTR_WIPHY (%d): 0x%08X (%u)\n", NL80211_ATTR_WIPHY, nla_get_u32(tb[NL80211_ATTR_WIPHY]), nla_get_u32(tb[NL80211_ATTR_WIPHY]));
    }

    if (tb[NL80211_ATTR_WIPHY_NAME]) {
        printf("NL80211_ATTR_WIPHY_NAME (%d): %s\n", NL80211_ATTR_WIPHY_NAME, nla_get_string(tb[NL80211_ATTR_WIPHY_NAME]));
    }

    if (tb[NL80211_ATTR_GENERATION]) {
        printf("NL80211_ATTR_GENERATION (%d): 0x%08X (%u)\n", NL80211_ATTR_GENERATION, nla_get_u32(tb[NL80211_ATTR_GENERATION]), nla_get_u32(tb[NL80211_ATTR_GENERATION]));
    }

    if (tb[NL80211_ATTR_WIPHY_RETRY_SHORT]) {
        printf("NL80211_ATTR_WIPHY_RETRY_SHORT (%d): 0x%08X (%u)\n", NL80211_ATTR_WIPHY_RETRY_SHORT, nla_get_u8(tb[NL80211_ATTR_WIPHY_RETRY_SHORT]), nla_get_u8(tb[NL80211_ATTR_WIPHY_RETRY_SHORT]));
    }

    if (tb[NL80211_ATTR_WIPHY_RETRY_LONG]) {
        printf("NL80211_ATTR_WIPHY_RETRY_LONG (%d): 0x%08X (%u)\n", NL80211_ATTR_WIPHY_RETRY_LONG, nla_get_u8(tb[NL80211_ATTR_WIPHY_RETRY_LONG]), nla_get_u8(tb[NL80211_ATTR_WIPHY_RETRY_LONG]));
    }

    if (tb[NL80211_ATTR_WIPHY_FRAG_THRESHOLD]) {
        printf("NL80211_ATTR_WIPHY_FRAG_THRESHOLD (%d): 0x%08X (%u)\n", NL80211_ATTR_WIPHY_FRAG_THRESHOLD, \
            nla_get_u32(tb[NL80211_ATTR_WIPHY_FRAG_THRESHOLD]), nla_get_u32(tb[NL80211_ATTR_WIPHY_FRAG_THRESHOLD]));
    }

    if (tb[NL80211_ATTR_WIPHY_RTS_THRESHOLD]) {
        printf("NL80211_ATTR_WIPHY_RTS_THRESHOLD (%d): 0x%08X (%u)\n", NL80211_ATTR_WIPHY_RTS_THRESHOLD, \
            nla_get_u32(tb[NL80211_ATTR_WIPHY_RTS_THRESHOLD]), nla_get_u32(tb[NL80211_ATTR_WIPHY_RTS_THRESHOLD]));
    }

    if (tb[NL80211_ATTR_WIPHY_COVERAGE_CLASS]) {
        printf("NL80211_ATTR_WIPHY_COVERAGE_CLASS (%d): 0x%08X (%u)\n", NL80211_ATTR_WIPHY_COVERAGE_CLASS, nla_get_u8(tb[NL80211_ATTR_WIPHY_COVERAGE_CLASS]), nla_get_u8(tb[NL80211_ATTR_WIPHY_COVERAGE_CLASS]));
    }

    if (tb[NL80211_ATTR_MAX_NUM_SCAN_SSIDS]) {
        printf("NL80211_ATTR_MAX_NUM_SCAN_SSIDS (%d): 0x%08X (%u)\n", NL80211_ATTR_MAX_NUM_SCAN_SSIDS, nla_get_u8(tb[NL80211_ATTR_MAX_NUM_SCAN_SSIDS]), nla_get_u8(tb[NL80211_ATTR_MAX_NUM_SCAN_SSIDS]));
    }

    if (tb[NL80211_ATTR_MAX_NUM_SCHED_SCAN_SSIDS]) {
        printf("NL80211_ATTR_MAX_NUM_SCHED_SCAN_SSIDS (%d): 0x%08X (%u)\n", NL80211_ATTR_MAX_NUM_SCHED_SCAN_SSIDS, nla_get_u8(tb[NL80211_ATTR_MAX_NUM_SCHED_SCAN_SSIDS]), nla_get_u8(tb[NL80211_ATTR_MAX_NUM_SCHED_SCAN_SSIDS]));
    }

    if (tb[NL80211_ATTR_MAX_SCAN_IE_LEN]) {
        printf("NL80211_ATTR_MAX_SCAN_IE_LEN (%d): 0x%08X (%u)\n", NL80211_ATTR_MAX_SCAN_IE_LEN, nla_get_u16(tb[NL80211_ATTR_MAX_SCAN_IE_LEN]), nla_get_u16(tb[NL80211_ATTR_MAX_SCAN_IE_LEN]));
    }

    if (tb[NL80211_ATTR_MAX_SCHED_SCAN_IE_LEN]) {
        printf("NL80211_ATTR_MAX_SCHED_SCAN_IE_LEN (%d): 0x%08X (%u)\n", NL80211_ATTR_MAX_SCHED_SCAN_IE_LEN, nla_get_u16(tb[NL80211_ATTR_MAX_SCHED_SCAN_IE_LEN]), nla_get_u16(tb[NL80211_ATTR_MAX_SCHED_SCAN_IE_LEN]));
    }    

    if (tb[NL80211_ATTR_MAX_MATCH_SETS]) {
        printf("NL80211_ATTR_MAX_MATCH_SETS (%d): 0x%08X (%u)\n", NL80211_ATTR_MAX_MATCH_SETS, nla_get_u8(tb[NL80211_ATTR_MAX_MATCH_SETS]), nla_get_u8(tb[NL80211_ATTR_MAX_MATCH_SETS]));
    }

    if (tb[NL80211_ATTR_SUPPORT_IBSS_RSN]) {
        printf("NL80211_ATTR_SUPPORT_IBSS_RSN (%d): Present\n", NL80211_ATTR_SUPPORT_IBSS_RSN);
    }

    if (tb[NL80211_ATTR_SUPPORT_MESH_AUTH]) {
        printf("NL80211_ATTR_SUPPORT_MESH_AUTH (%d): Present\n", NL80211_ATTR_SUPPORT_MESH_AUTH);
    }

    if (tb[NL80211_ATTR_SUPPORT_AP_UAPSD]) {
        printf("NL80211_ATTR_SUPPORT_AP_UAPSD (%d): Present\n", NL80211_ATTR_SUPPORT_AP_UAPSD);
    }

    if (tb[NL80211_ATTR_TDLS_SUPPORT]) {
        printf("NL80211_ATTR_TDLS_SUPPORT (%d): Present\n", NL80211_ATTR_TDLS_SUPPORT);
    }

    if (tb[NL80211_ATTR_TDLS_EXTERNAL_SETUP]) {
        printf("NL80211_ATTR_TDLS_EXTERNAL_SETUP (%d): Present\n", NL80211_ATTR_TDLS_EXTERNAL_SETUP);
    }

    if (tb[NL80211_ATTR_CONTROL_PORT_ETHERTYPE]) {
        printf("NL80211_ATTR_CONTROL_PORT_ETHERTYPE (%d): Present\n", NL80211_ATTR_CONTROL_PORT_ETHERTYPE);
    }

    if (tb[NL80211_ATTR_OFFCHANNEL_TX_OK]) {
        printf("NL80211_ATTR_OFFCHANNEL_TX_OK (%d): Present\n", NL80211_ATTR_OFFCHANNEL_TX_OK);
    }

    if (tb[NL80211_ATTR_MAX_NUM_PMKIDS]) {
        printf("NL80211_ATTR_MAX_NUM_PMKIDS (%d): 0x%08X (%u)\n", NL80211_ATTR_MAX_NUM_PMKIDS, nla_get_u8(tb[NL80211_ATTR_MAX_NUM_PMKIDS]), nla_get_u8(tb[NL80211_ATTR_MAX_NUM_PMKIDS]));
    }

    if (tb[NL80211_ATTR_WIPHY_ANTENNA_AVAIL_TX]) {
        printf("NL80211_ATTR_WIPHY_ANTENNA_AVAIL_TX (%d): 0x%08X (%u)\n", NL80211_ATTR_WIPHY_ANTENNA_AVAIL_TX, \
            nla_get_u32(tb[NL80211_ATTR_WIPHY_ANTENNA_AVAIL_TX]), nla_get_u32(tb[NL80211_ATTR_WIPHY_ANTENNA_AVAIL_TX]));
    }

    if (tb[NL80211_ATTR_WIPHY_ANTENNA_AVAIL_RX]) {
        printf("NL80211_ATTR_WIPHY_ANTENNA_AVAIL_RX (%d): 0x%08X (%u)\n", NL80211_ATTR_WIPHY_ANTENNA_AVAIL_RX, \
            nla_get_u32(tb[NL80211_ATTR_WIPHY_ANTENNA_AVAIL_RX]), nla_get_u32(tb[NL80211_ATTR_WIPHY_ANTENNA_AVAIL_RX]));
    }

    if (tb[NL80211_ATTR_MAX_REMAIN_ON_CHANNEL_DURATION]) {
        printf("NL80211_ATTR_MAX_REMAIN_ON_CHANNEL_DURATION (%d): 0x%08X (%u ms)\n", NL80211_ATTR_MAX_REMAIN_ON_CHANNEL_DURATION, \
            nla_get_u32(tb[NL80211_ATTR_MAX_REMAIN_ON_CHANNEL_DURATION]), nla_get_u32(tb[NL80211_ATTR_MAX_REMAIN_ON_CHANNEL_DURATION]));
    }

    if (tb[NL80211_ATTR_FEATURE_FLAGS]) {
        printf("NL80211_ATTR_FEATURE_FLAGS (%d): 0x%08X (%u)\n", NL80211_ATTR_FEATURE_FLAGS, nla_get_u32(tb[NL80211_ATTR_FEATURE_FLAGS]), nla_get_u32(tb[NL80211_ATTR_FEATURE_FLAGS]));
    }

    if (tb[NL80211_ATTR_CIPHER_SUITES]) {
        printf("NL80211_ATTR_CIPHER_SUITES (%d): Len=%d\n", NL80211_ATTR_CIPHER_SUITES, nla_len(tb[NL80211_ATTR_CIPHER_SUITES]));
    }

    if (tb[NL80211_ATTR_HT_CAPABILITY_MASK]) {
        printf("NL80211_ATTR_HT_CAPABILITY_MASK (%d): Len=%d\n", NL80211_ATTR_HT_CAPABILITY_MASK, nla_len(tb[NL80211_ATTR_HT_CAPABILITY_MASK]));
    }

    /* 這個是 Nested Flag List，表示哪些 Interface Yype 是 Software Iftype。 */
    if (tb[NL80211_ATTR_SOFTWARE_IFTYPES]) {
        struct nlattr *ift = NULL;
        int rem = -1;

        printf("NL80211_ATTR_SOFTWARE_IFTYPES (%d):\n", NL80211_ATTR_SOFTWARE_IFTYPES);
        nla_for_each_nested(ift, tb[NL80211_ATTR_SOFTWARE_IFTYPES], rem) {
            int iftype = nla_type(ift);

            printf("  %s (%d)\n", iftype_to_string(iftype), iftype);
        }
    }

    /* 這個也是 Nested Attribute List，裡面每個 child attr 的 value 是一個 Command ID。 */
    if (tb[NL80211_ATTR_SUPPORTED_COMMANDS]) {
        struct nlattr *cmd = NULL;
        int rem = -1;

        printf("NL80211_ATTR_SUPPORTED_COMMANDS (%d):\n", NL80211_ATTR_SUPPORTED_COMMANDS);
        nla_for_each_nested(cmd, tb[NL80211_ATTR_SUPPORTED_COMMANDS], rem) {
            uint32_t cmd_id = nla_get_u32(cmd);

            printf("  CMDs[%d] = %s (%u)\n", nla_type(cmd), cmd_to_string(cmd_id), cmd_id);
        }
    }

    /* 這張 wiphy / radio 支援建立哪些 Interface Type。 */
    if (tb[NL80211_ATTR_SUPPORTED_IFTYPES]) {
        struct nlattr *ift = NULL;
        int rem = -1;

        printf("NL80211_ATTR_SUPPORTED_IFTYPES (%d):\n", NL80211_ATTR_SUPPORTED_IFTYPES);
        nla_for_each_nested(ift, tb[NL80211_ATTR_SUPPORTED_IFTYPES], rem) {
            int iftype = nla_type(ift);

            printf("  %s (%d)\n", iftype_to_string(iftype), iftype);
        }
    }

    /* INTERFACE_COMBINATIONS 是 wiphy 的「多介面共存規則」，比 SUPPORTED_IFTYPES 更進一步；SUPPORTED_IFTYPES 只說支援哪些 Mode，INTERFACE_COMBINATIONS 說這些 Mode 能怎麼同時存在。 */
    if (tb[NL80211_ATTR_INTERFACE_COMBINATIONS]) {
        struct nlattr *comb = NULL;
        int rem_comb = -1;
        static struct nla_policy iface_comb_policy[NUM_NL80211_IFACE_COMB + 1] = {
            [NL80211_IFACE_COMB_LIMITS]               = { .type = NLA_NESTED },
            [NL80211_IFACE_COMB_MAXNUM]               = { .type = NLA_U32 },
            [NL80211_IFACE_COMB_STA_AP_BI_MATCH]      = { .type = NLA_FLAG },
            [NL80211_IFACE_COMB_NUM_CHANNELS]         = { .type = NLA_U32 },
            [NL80211_IFACE_COMB_RADAR_DETECT_WIDTHS]  = { .type = NLA_U32 },
            [NL80211_IFACE_COMB_RADAR_DETECT_REGIONS] = { .type = NLA_U32 },
            [NL80211_IFACE_COMB_BI_MIN_GCD]           = { .type = NLA_U32 },
        };

        printf("NL80211_ATTR_INTERFACE_COMBINATIONS (%d):\n", NL80211_ATTR_INTERFACE_COMBINATIONS);
        nla_for_each_nested(comb, tb[NL80211_ATTR_INTERFACE_COMBINATIONS], rem_comb) {
            struct nlattr *attrs[NUM_NL80211_IFACE_COMB + 1];
            int ret = -1;
            static struct nla_policy iface_limit_policy[NUM_NL80211_IFACE_LIMIT + 1] = {
                [NL80211_IFACE_LIMIT_TYPES] = { .type = NLA_NESTED },
                [NL80211_IFACE_LIMIT_MAX]   = { .type = NLA_U32 },
            };

            printf("  Combination[%d]: len = %d\n", nla_type(comb), nla_len(comb));
            memset(attrs, 0, sizeof(attrs));
            ret = nla_parse_nested(attrs, NUM_NL80211_IFACE_COMB, comb, iface_comb_policy);
            if (ret < 0) {
                printf("  nla_parse_nested(combination) failed: %s (%d)\n", nl_geterror(ret), ret);
                continue;
            }

            if (attrs[NL80211_IFACE_COMB_UNSPEC]) {
                printf("  NL80211_IFACE_COMB_UNSPEC (%d): len = %d\n", NL80211_IFACE_COMB_UNSPEC, nla_len(attrs[NL80211_IFACE_COMB_UNSPEC]));
            }
            
            /* 進入第二層巢狀 */
            if (attrs[NL80211_IFACE_COMB_LIMITS]) {
                struct nlattr *limit = NULL;
                int rem_limit = -1;

                printf("  NL80211_IFACE_COMB_LIMITS (%d): len = %d\n", NL80211_IFACE_COMB_LIMITS, nla_len(attrs[NL80211_IFACE_COMB_LIMITS]));
                nla_for_each_nested(limit, attrs[NL80211_IFACE_COMB_LIMITS], rem_limit) {
                    struct nlattr *lim[NUM_NL80211_IFACE_LIMIT + 1];
                    int ret = -1;

                    memset(lim, 0, sizeof(lim));
                    ret = nla_parse_nested(lim, NUM_NL80211_IFACE_LIMIT, limit, iface_limit_policy);
                    if (ret < 0) {
                        printf("    Limit[%d]: nla_parse_nested() failed: %s (%d)\n", nla_type(limit), nl_geterror(ret), ret);
                        continue;
                    }

                    /* 進入第三層巢狀 */
                    printf("    Limit[%d]:\n", nla_type(limit));
                    if (lim[NL80211_IFACE_LIMIT_MAX]) {
                        printf("      NL80211_IFACE_LIMIT_MAX (%d): %u\n", NL80211_IFACE_LIMIT_MAX, nla_get_u32(lim[NL80211_IFACE_LIMIT_MAX]));
                    }

                    if (lim[NL80211_IFACE_LIMIT_TYPES]) {
                        struct nlattr *ift = NULL;
                        int rem_ift = -1;

                        printf("      NL80211_IFACE_LIMIT_TYPES (%d):\n", NL80211_IFACE_LIMIT_TYPES);
                        nla_for_each_nested(ift, lim[NL80211_IFACE_LIMIT_TYPES], rem_ift) {
                            int iftype = nla_type(ift);

                            /* 進入第四層巢狀 */
                            printf("        %s (%d)\n", iftype_to_string(iftype), iftype);
                        }
                    }
                }
            }

            if (attrs[NL80211_IFACE_COMB_MAXNUM]) {
                printf("  NL80211_IFACE_COMB_MAXNUM (%d): %u\n", NL80211_IFACE_COMB_MAXNUM, nla_get_u32(attrs[NL80211_IFACE_COMB_MAXNUM]));
            }

            if (attrs[NL80211_IFACE_COMB_STA_AP_BI_MATCH]) {
                printf("  NL80211_IFACE_COMB_STA_AP_BI_MATCH (%d): Present\n", NL80211_IFACE_COMB_STA_AP_BI_MATCH);
            }

            if (attrs[NL80211_IFACE_COMB_NUM_CHANNELS]) {
                printf("  NL80211_IFACE_COMB_NUM_CHANNELS (%d): %u\n", NL80211_IFACE_COMB_NUM_CHANNELS, nla_get_u32(attrs[NL80211_IFACE_COMB_NUM_CHANNELS]));
            }

            if (attrs[NL80211_IFACE_COMB_RADAR_DETECT_WIDTHS]) {
                printf("  NL80211_IFACE_COMB_RADAR_DETECT_WIDTHS (%d): 0x%08x\n", NL80211_IFACE_COMB_RADAR_DETECT_WIDTHS, nla_get_u32(attrs[NL80211_IFACE_COMB_RADAR_DETECT_WIDTHS]));
            }

            if (attrs[NL80211_IFACE_COMB_RADAR_DETECT_REGIONS]) {
                printf("  NL80211_IFACE_COMB_RADAR_DETECT_REGIONS (%d): 0x%08x\n", NL80211_IFACE_COMB_RADAR_DETECT_REGIONS, nla_get_u32(attrs[NL80211_IFACE_COMB_RADAR_DETECT_REGIONS]));
            }

            if (attrs[NL80211_IFACE_COMB_BI_MIN_GCD]) {
                printf("  NL80211_IFACE_COMB_BI_MIN_GCD (%d): %u\n", NL80211_IFACE_COMB_BI_MIN_GCD, nla_get_u32(attrs[NL80211_IFACE_COMB_BI_MIN_GCD]));
            }
        }
    }

    if (tb[NL80211_ATTR_WIPHY_BANDS]) {
        struct nlattr *band;
        int rem_band = -1;
        static struct nla_policy band_policy[NL80211_BAND_ATTR_MAX + 1] = {
            [NL80211_BAND_ATTR_FREQS]              = { .type = NLA_NESTED },
            [NL80211_BAND_ATTR_RATES]              = { .type = NLA_NESTED },
            [NL80211_BAND_ATTR_HT_MCS_SET]         = { .type = NLA_UNSPEC },
            [NL80211_BAND_ATTR_HT_CAPA]            = { .type = NLA_U16 },
            [NL80211_BAND_ATTR_HT_AMPDU_FACTOR]    = { .type = NLA_U8 },
            [NL80211_BAND_ATTR_HT_AMPDU_DENSITY]   = { .type = NLA_U8 },
            [NL80211_BAND_ATTR_VHT_MCS_SET]        = { .type = NLA_UNSPEC },
            [NL80211_BAND_ATTR_VHT_CAPA]           = { .type = NLA_U32 },
        };

        printf("NL80211_ATTR_WIPHY_BANDS (%d):\n", NL80211_ATTR_WIPHY_BANDS);

        nla_for_each_nested(band, tb[NL80211_ATTR_WIPHY_BANDS], rem_band) {
            struct nlattr *band_attrs[NL80211_BAND_ATTR_MAX + 1];
            int ret = -1;
            static struct nla_policy bitrate_policy[NL80211_BITRATE_ATTR_MAX + 1] = {
                [NL80211_BITRATE_ATTR_RATE]                = { .type = NLA_U32 },
                [NL80211_BITRATE_ATTR_2GHZ_SHORTPREAMBLE]  = { .type = NLA_FLAG },
            };

            memset(band_attrs, 0, sizeof(band_attrs));
            printf("  Band[%d]: len = %d\n", nla_type(band), nla_len(band));

            ret = nla_parse_nested(band_attrs, NL80211_BAND_ATTR_MAX, band, band_policy);
            if (ret < 0) {
                printf("    nla_parse_nested(band) failed: %s (%d)\n", nl_geterror(ret), ret);
                continue;
            }

            if (band_attrs[NL80211_BAND_ATTR_FREQS]) {
                struct nlattr *freq = NULL;
                int rem_freq = -1;
                static struct nla_policy freq_policy[NL80211_FREQUENCY_ATTR_MAX + 1] = {
                    [NL80211_FREQUENCY_ATTR_FREQ]          = { .type = NLA_U32 },
                    [NL80211_FREQUENCY_ATTR_DISABLED]      = { .type = NLA_FLAG },
#ifdef NL80211_FREQUENCY_ATTR_OFFSET
                    [NL80211_FREQUENCY_ATTR_OFFSET]        = { .type = NLA_U32 },
#endif
                    [NL80211_FREQUENCY_ATTR_NO_IR]         = { .type = NLA_FLAG },
                    [__NL80211_FREQUENCY_ATTR_NO_IBSS]     = { .type = NLA_FLAG },
                    [NL80211_FREQUENCY_ATTR_RADAR]         = { .type = NLA_FLAG },
                    [NL80211_FREQUENCY_ATTR_MAX_TX_POWER]  = { .type = NLA_U32 },
                    [NL80211_FREQUENCY_ATTR_DFS_STATE]     = { .type = NLA_U32 },
                    [NL80211_FREQUENCY_ATTR_DFS_TIME]      = { .type = NLA_U32 },
                    [NL80211_FREQUENCY_ATTR_NO_HT40_MINUS] = { .type = NLA_FLAG },
                    [NL80211_FREQUENCY_ATTR_NO_HT40_PLUS]  = { .type = NLA_FLAG },
                    [NL80211_FREQUENCY_ATTR_NO_80MHZ]      = { .type = NLA_FLAG },
                    [NL80211_FREQUENCY_ATTR_NO_160MHZ]     = { .type = NLA_FLAG },
                    [NL80211_FREQUENCY_ATTR_DFS_CAC_TIME]  = { .type = NLA_U32 },
                    [NL80211_FREQUENCY_ATTR_INDOOR_ONLY]   = { .type = NLA_FLAG },
                    [NL80211_FREQUENCY_ATTR_IR_CONCURRENT] = { .type = NLA_FLAG },
                    [NL80211_FREQUENCY_ATTR_NO_20MHZ]      = { .type = NLA_FLAG },
                    [NL80211_FREQUENCY_ATTR_NO_10MHZ]      = { .type = NLA_FLAG },
                    [NL80211_FREQUENCY_ATTR_WMM]           = { .type = NLA_NESTED },
                };

                printf("    NL80211_BAND_ATTR_FREQS (%d): len = %d\n", NL80211_BAND_ATTR_FREQS, nla_len(band_attrs[NL80211_BAND_ATTR_FREQS]));
                nla_for_each_nested(freq, band_attrs[NL80211_BAND_ATTR_FREQS], rem_freq) {
                    struct nlattr *freq_attrs[NL80211_FREQUENCY_ATTR_MAX + 1];
                    int ret = -1;

                    memset(freq_attrs, 0, sizeof(freq_attrs));
                    ret = nla_parse_nested(freq_attrs, NL80211_FREQUENCY_ATTR_MAX, freq, freq_policy);
                    if (ret < 0) {
                        printf("      Freq[%d]: nla_parse_nested() failed: %s (%d)\n", nla_type(freq), nl_geterror(ret), ret);
                        continue;
                    }

                    printf("      Freq[%d]: len = %d\n", nla_type(freq), nla_len(freq));
                    if (freq_attrs[NL80211_FREQUENCY_ATTR_FREQ]) {
                        printf("        NL80211_FREQUENCY_ATTR_FREQ (%d): %u MHz\n", NL80211_FREQUENCY_ATTR_FREQ, nla_get_u32(freq_attrs[NL80211_FREQUENCY_ATTR_FREQ]));
                    }

                    if (freq_attrs[NL80211_FREQUENCY_ATTR_DISABLED]) {
                        printf("        NL80211_FREQUENCY_ATTR_DISABLED (%d): Present\n", NL80211_FREQUENCY_ATTR_DISABLED);
                    }

#ifdef NL80211_FREQUENCY_ATTR_OFFSET
                    if (freq_attrs[NL80211_FREQUENCY_ATTR_OFFSET]) {
                        printf("        NL80211_FREQUENCY_ATTR_OFFSET (%d): %u\n", NL80211_FREQUENCY_ATTR_OFFSET, nla_get_u32(freq_attrs[NL80211_FREQUENCY_ATTR_OFFSET]));
                    }
#endif

                    if (freq_attrs[NL80211_FREQUENCY_ATTR_NO_IR]) {
                        printf("        NL80211_FREQUENCY_ATTR_NO_IR (%d): Present\n", NL80211_FREQUENCY_ATTR_NO_IR);
                    }

                    if (freq_attrs[__NL80211_FREQUENCY_ATTR_NO_IBSS]) {
                        printf("        __NL80211_FREQUENCY_ATTR_NO_IBSS (%d): Present\n", __NL80211_FREQUENCY_ATTR_NO_IBSS);
                    }

                    if (freq_attrs[NL80211_FREQUENCY_ATTR_RADAR]) {
                        printf("        NL80211_FREQUENCY_ATTR_RADAR (%d): Present\n", NL80211_FREQUENCY_ATTR_RADAR);
                    }

                    if (freq_attrs[NL80211_FREQUENCY_ATTR_MAX_TX_POWER]) {
                        uint32_t pwr = nla_get_u32(freq_attrs[NL80211_FREQUENCY_ATTR_MAX_TX_POWER]);

                        printf("        NL80211_FREQUENCY_ATTR_MAX_TX_POWER (%d): %.2f dBm (%u mBm)\n", NL80211_FREQUENCY_ATTR_MAX_TX_POWER, pwr / 100.0, pwr);
                    }

                    if (freq_attrs[NL80211_FREQUENCY_ATTR_DFS_STATE]) {
                        printf("        NL80211_FREQUENCY_ATTR_DFS_STATE (%d): %u\n", NL80211_FREQUENCY_ATTR_DFS_STATE, nla_get_u32(freq_attrs[NL80211_FREQUENCY_ATTR_DFS_STATE]));
                    }

                    if (freq_attrs[NL80211_FREQUENCY_ATTR_DFS_TIME]) {
                        printf("        NL80211_FREQUENCY_ATTR_DFS_TIME (%d): %u ms\n", NL80211_FREQUENCY_ATTR_DFS_TIME, nla_get_u32(freq_attrs[NL80211_FREQUENCY_ATTR_DFS_TIME]));
                    }

                    if (freq_attrs[NL80211_FREQUENCY_ATTR_NO_HT40_MINUS]) {
                        printf("        NL80211_FREQUENCY_ATTR_NO_HT40_MINUS (%d): Present\n", NL80211_FREQUENCY_ATTR_NO_HT40_MINUS);
                    }

                    if (freq_attrs[NL80211_FREQUENCY_ATTR_NO_HT40_PLUS]) {
                        printf("        NL80211_FREQUENCY_ATTR_NO_HT40_PLUS (%d): Present\n", NL80211_FREQUENCY_ATTR_NO_HT40_PLUS);
                    }

                    if (freq_attrs[NL80211_FREQUENCY_ATTR_NO_80MHZ]) {
                        printf("        NL80211_FREQUENCY_ATTR_NO_80MHZ (%d): Present\n", NL80211_FREQUENCY_ATTR_NO_80MHZ);
                    }

                    if (freq_attrs[NL80211_FREQUENCY_ATTR_NO_160MHZ]) {
                        printf("        NL80211_FREQUENCY_ATTR_NO_160MHZ (%d): Present\n", NL80211_FREQUENCY_ATTR_NO_160MHZ);
                    }

                    if (freq_attrs[NL80211_FREQUENCY_ATTR_DFS_CAC_TIME]) {
                        printf("        NL80211_FREQUENCY_ATTR_DFS_CAC_TIME (%d): %u ms\n", NL80211_FREQUENCY_ATTR_DFS_CAC_TIME, nla_get_u32(freq_attrs[NL80211_FREQUENCY_ATTR_DFS_CAC_TIME]));
                    }

                    if (freq_attrs[NL80211_FREQUENCY_ATTR_INDOOR_ONLY]) {
                        printf("        NL80211_FREQUENCY_ATTR_INDOOR_ONLY (%d): Present\n", NL80211_FREQUENCY_ATTR_INDOOR_ONLY);
                    }

                    if (freq_attrs[NL80211_FREQUENCY_ATTR_IR_CONCURRENT]) {
                        printf("        NL80211_FREQUENCY_ATTR_IR_CONCURRENT (%d): Present\n", NL80211_FREQUENCY_ATTR_IR_CONCURRENT);
                    }

                    if (freq_attrs[NL80211_FREQUENCY_ATTR_NO_20MHZ]) {
                        printf("        NL80211_FREQUENCY_ATTR_NO_20MHZ (%d): Present\n", NL80211_FREQUENCY_ATTR_NO_20MHZ);
                    }

                    if (freq_attrs[NL80211_FREQUENCY_ATTR_NO_10MHZ]) {
                        printf("        NL80211_FREQUENCY_ATTR_NO_10MHZ (%d): Present\n", NL80211_FREQUENCY_ATTR_NO_10MHZ);
                    }

                    if (freq_attrs[NL80211_FREQUENCY_ATTR_WMM]) {
                        printf("        NL80211_FREQUENCY_ATTR_WMM (%d): len = %d\n", NL80211_FREQUENCY_ATTR_WMM, nla_len(freq_attrs[NL80211_FREQUENCY_ATTR_WMM]));
                    }
                }
            }

            if (band_attrs[NL80211_BAND_ATTR_RATES]) {
                struct nlattr *rate = NULL;
                int rem_rate = -1;

                printf("    NL80211_BAND_ATTR_RATES (%d): len = %d\n", NL80211_BAND_ATTR_RATES, nla_len(band_attrs[NL80211_BAND_ATTR_RATES]));
                nla_for_each_nested(rate, band_attrs[NL80211_BAND_ATTR_RATES], rem_rate) {
                    struct nlattr *rate_attrs[NL80211_BITRATE_ATTR_MAX + 1];
                    int ret = -1;

                    memset(rate_attrs, 0, sizeof(rate_attrs));
                    ret = nla_parse_nested(rate_attrs, NL80211_BITRATE_ATTR_MAX, rate, bitrate_policy);
                    if (ret < 0) {
                        printf("      Rate[%d]: nla_parse_nested() failed: %s (%d)\n", nla_type(rate), nl_geterror(ret), ret);
                        continue;
                    }

                    printf("      Rate[%d]:\n", nla_type(rate));
                    if (rate_attrs[NL80211_BITRATE_ATTR_RATE]) {
                        uint32_t r = nla_get_u32(rate_attrs[NL80211_BITRATE_ATTR_RATE]);

                        printf("        NL80211_BITRATE_ATTR_RATE (%d): %u.%u Mbps\n", NL80211_BITRATE_ATTR_RATE, r / 10, r % 10);
                    }

                    if (rate_attrs[NL80211_BITRATE_ATTR_2GHZ_SHORTPREAMBLE]) {
                        printf("        NL80211_BITRATE_ATTR_2GHZ_SHORTPREAMBLE (%d): Present\n", NL80211_BITRATE_ATTR_2GHZ_SHORTPREAMBLE);
                    }
                }
            }

            if (band_attrs[NL80211_BAND_ATTR_HT_MCS_SET]) {
                printf("    NL80211_BAND_ATTR_HT_MCS_SET (%d): ", NL80211_BAND_ATTR_HT_MCS_SET);
                print_hex(nla_data(band_attrs[NL80211_BAND_ATTR_HT_MCS_SET]), nla_len(band_attrs[NL80211_BAND_ATTR_HT_MCS_SET]));
                printf("\n");
            }

            if (band_attrs[NL80211_BAND_ATTR_HT_CAPA]) {
                printf("    NL80211_BAND_ATTR_HT_CAPA (%d): 0x%08X\n", NL80211_BAND_ATTR_HT_CAPA, nla_get_u16(band_attrs[NL80211_BAND_ATTR_HT_CAPA]));
            }

            if (band_attrs[NL80211_BAND_ATTR_HT_AMPDU_FACTOR]) {
                printf("    NL80211_BAND_ATTR_HT_AMPDU_FACTOR (%d): %u\n", NL80211_BAND_ATTR_HT_AMPDU_FACTOR, nla_get_u8(band_attrs[NL80211_BAND_ATTR_HT_AMPDU_FACTOR]));
            }

            if (band_attrs[NL80211_BAND_ATTR_HT_AMPDU_DENSITY]) {
                printf("    NL80211_BAND_ATTR_HT_AMPDU_DENSITY (%d): %u\n", NL80211_BAND_ATTR_HT_AMPDU_DENSITY, nla_get_u8(band_attrs[NL80211_BAND_ATTR_HT_AMPDU_DENSITY]));
            }

            if (band_attrs[NL80211_BAND_ATTR_VHT_MCS_SET]) {
                printf("    NL80211_BAND_ATTR_VHT_MCS_SET (%d): ", NL80211_BAND_ATTR_VHT_MCS_SET);
                print_hex(nla_data(band_attrs[NL80211_BAND_ATTR_VHT_MCS_SET]), nla_len(band_attrs[NL80211_BAND_ATTR_VHT_MCS_SET]));
                printf("\n");
            }

            if (band_attrs[NL80211_BAND_ATTR_VHT_CAPA]) {
                printf("    NL80211_BAND_ATTR_VHT_CAPA (%d): 0x%08X\n", NL80211_BAND_ATTR_VHT_CAPA, nla_get_u32(band_attrs[NL80211_BAND_ATTR_VHT_CAPA]));
            }

            if (band_attrs[NL80211_BAND_ATTR_IFTYPE_DATA]) {
                printf("    NL80211_BAND_ATTR_IFTYPE_DATA (%d): 0x%08X\n", NL80211_BAND_ATTR_IFTYPE_DATA, nla_get_u32(band_attrs[NL80211_BAND_ATTR_IFTYPE_DATA]));
            }

            if (band_attrs[NL80211_BAND_ATTR_EDMG_CHANNELS]) {
                printf("    NL80211_BAND_ATTR_EDMG_CHANNELS (%d): 0x%08X\n", NL80211_BAND_ATTR_EDMG_CHANNELS, nla_get_u32(band_attrs[NL80211_BAND_ATTR_EDMG_CHANNELS]));
            }

            if (band_attrs[NL80211_BAND_ATTR_EDMG_BW_CONFIG]) {
                printf("    NL80211_BAND_ATTR_EDMG_BW_CONFIG (%d): 0x%08X\n", NL80211_BAND_ATTR_EDMG_BW_CONFIG, nla_get_u32(band_attrs[NL80211_BAND_ATTR_EDMG_BW_CONFIG]));
            }
        }
    }

    printf("\n");

    return NL_SKIP;
}

int
finish_handler(msg, arg)
struct nl_msg *msg;
void *arg;
{
    (void)msg;
    struct cb_context *ctx = (struct cb_context *)arg;

    ctx->done = 1;

    return NL_SKIP;
}

int
ack_handler(msg, arg)
struct nl_msg *msg;
void *arg;
{
    (void)msg;
    struct cb_context *ctx = arg;

    ctx->done = 1;

    return NL_STOP;
}

int 
get_wiphy(state)
struct nl80211_state *state;
{
    int ret = -1;
    struct nl_msg *msg = NULL;
    struct nl_cb *cb = NULL;
    struct cb_context ctx;

    bzero(&ctx, sizeof(struct cb_context));

    msg = nlmsg_alloc();
    if (msg == NULL) {
        fprintf(stderr, "nlmsg_alloc() failed. \n");

        return -ENOMEM;
    }

    cb = nl_cb_alloc(NL_CB_DEFAULT);
    if (cb == NULL) {
        fprintf(stderr, "nl_cb_alloc() failed. \n");
        ret = -ENOMEM;

        goto finish;
    }

    if (!genlmsg_put(msg, NL_AUTO_PORT, NL_AUTO_SEQ, state->nl80211_id, 0, NLM_F_DUMP, NL80211_CMD_GET_WIPHY, 0)) {
        fprintf(stderr, "genlmsg_put() failed. \n");
        ret = -ENOBUFS;

        goto finish;
    }
/*
    ifindex = if_nametoindex (ifname);
    if (ifindex == 0) {
        fprintf(stderr, "if_nametoindex() failed!! \n");
        ret = -ENODEV;

        goto finish;
    }

    ret = nla_put_u32(msg, NL80211_ATTR_IFINDEX, (unsigned int)ifindex);
    if (ret < 0) {
        fprintf(stderr, "nla_put_u32(NL80211_ATTR_IFINDEX) failed: %s (%d) \n", nl_geterror(ret), ret);

        goto finish;
    }
*/
    ret = nl_cb_err(cb, NL_CB_CUSTOM, error_handler, &ctx);
    if (ret < 0) {
        fprintf(stderr, "nl_cb_err(error_handler) failed: %s (%d) \n", nl_geterror(ret), ret);

        goto finish;
    }

    ret = nl_cb_set(cb, NL_CB_VALID,     NL_CB_CUSTOM, valid_handler, (void *)&ctx);
    if (ret < 0) {
        fprintf(stderr, "nl_cb_set(valid_handler) failed: %s (%d) \n", nl_geterror(ret), ret);

        goto finish;
    }

    ret = nl_cb_set(cb, NL_CB_FINISH,    NL_CB_CUSTOM, finish_handler, (void *)&ctx);
    if (ret < 0) {
        fprintf(stderr, "nl_cb_set(finish_handler) failed: %s (%d) \n", nl_geterror(ret), ret);

        goto finish;
    }

    ret = nl_cb_set(cb, NL_CB_ACK,       NL_CB_CUSTOM,  ack_handler, (void *)&ctx);
    if (ret < 0) {
        fprintf(stderr, "nl_cb_set(ack_handler) failed: %s (%d) \n", nl_geterror(ret), ret);

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
        fprintf(stderr, "netlink error: %s (%d)\n", strerror(ctx.err), ctx.err);
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
