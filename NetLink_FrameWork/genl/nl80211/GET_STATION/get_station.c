#include "main.h"
#include "get_station.h"

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
    static struct nla_policy station_policy[NL80211_ATTR_MAX + 1] = {
        [NL80211_ATTR_IFINDEX]    = { .type = NLA_U32 },
        [NL80211_ATTR_MAC]        = { .type = NLA_UNSPEC },
        [NL80211_ATTR_GENERATION] = { .type = NLA_U32 },
        [NL80211_ATTR_STA_INFO]   = { .type = NLA_NESTED },
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

    ret = nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0), genlmsg_attrlen(gnlh, 0), station_policy);
    if (ret < 0) {
        fprintf(stderr, "nla_parse() failed: %s (%d).\n", nl_geterror(ret), ret);

        return NL_SKIP;
    }

    if (tb[NL80211_ATTR_IFINDEX]) {
        printf("NL80211_ATTR_IFINDEX (%d): %u\n", NL80211_ATTR_IFINDEX, nla_get_u32(tb[NL80211_ATTR_IFINDEX]));
    }

    if (tb[NL80211_ATTR_MAC]) {
        unsigned char *mac = nla_data(tb[NL80211_ATTR_MAC]);

        printf("NL80211_ATTR_MAC (%d): %02X:%02X:%02X:%02X:%02X:%02X\n", NL80211_ATTR_MAC, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    }

    if (tb[NL80211_ATTR_GENERATION]) {
        printf("NL80211_ATTR_GENERATION (%d): %u\n", NL80211_ATTR_GENERATION, nla_get_u32(tb[NL80211_ATTR_GENERATION]));
    }

    if (tb[NL80211_ATTR_STA_INFO]) {
        struct nlattr *sta_info[NL80211_STA_INFO_MAX + 1];
    /*  struct nlattr *attr = NULL;
        int rem = -1;
     */
        static struct nla_policy sta_info_policy[NL80211_STA_INFO_MAX + 1] = {
            [NL80211_STA_INFO_INACTIVE_TIME]       = { .type = NLA_U32 },
            [NL80211_STA_INFO_RX_BYTES]            = { .type = NLA_U32 },
            [NL80211_STA_INFO_TX_BYTES]            = { .type = NLA_U32 },
            [NL80211_STA_INFO_LLID]                = { .type = NLA_U16 },
            [NL80211_STA_INFO_PLID]                = { .type = NLA_U16 },
            [NL80211_STA_INFO_PLINK_STATE]         = { .type = NLA_U8 },
            [NL80211_STA_INFO_SIGNAL]              = { .type = NLA_U8 },
            [NL80211_STA_INFO_TX_BITRATE]          = { .type = NLA_NESTED },
            [NL80211_STA_INFO_RX_PACKETS]          = { .type = NLA_U32 },
            [NL80211_STA_INFO_TX_PACKETS]          = { .type = NLA_U32 },
            [NL80211_STA_INFO_TX_RETRIES]          = { .type = NLA_U32 },
            [NL80211_STA_INFO_TX_FAILED]           = { .type = NLA_U32 },
            [NL80211_STA_INFO_SIGNAL_AVG]          = { .type = NLA_U8 },
            [NL80211_STA_INFO_RX_BITRATE]          = { .type = NLA_NESTED },
            [NL80211_STA_INFO_BSS_PARAM]           = { .type = NLA_NESTED },
            [NL80211_STA_INFO_CONNECTED_TIME]      = { .type = NLA_U32 },
            [NL80211_STA_INFO_STA_FLAGS]           = { .type = NLA_NESTED },
            [NL80211_STA_INFO_BEACON_LOSS]         = { .type = NLA_U32 },
            [NL80211_STA_INFO_T_OFFSET]            = { .type = NLA_U64 },
            [NL80211_STA_INFO_LOCAL_PM]            = { .type = NLA_U32 },
            [NL80211_STA_INFO_PEER_PM]             = { .type = NLA_U32 },
            [NL80211_STA_INFO_NONPEER_PM]          = { .type = NLA_U32 },
            [NL80211_STA_INFO_RX_BYTES64]          = { .type = NLA_U64 },
            [NL80211_STA_INFO_TX_BYTES64]          = { .type = NLA_U64 },
            [NL80211_STA_INFO_CHAIN_SIGNAL]        = { .type = NLA_NESTED },
            [NL80211_STA_INFO_CHAIN_SIGNAL_AVG]    = { .type = NLA_NESTED },
            [NL80211_STA_INFO_EXPECTED_THROUGHPUT] = { .type = NLA_U32 },
            [NL80211_STA_INFO_RX_DROP_MISC]        = { .type = NLA_U64 },
            [NL80211_STA_INFO_BEACON_RX]           = { .type = NLA_U64 },
            [NL80211_STA_INFO_BEACON_SIGNAL_AVG]   = { .type = NLA_U8 },
            [NL80211_STA_INFO_TID_STATS]           = { .type = NLA_NESTED },
            [NL80211_STA_INFO_RX_DURATION]         = { .type = NLA_U64 },
            [NL80211_STA_INFO_PAD]                 = { .type = NLA_UNSPEC },
            [NL80211_STA_INFO_ACK_SIGNAL]          = { .type = NLA_U8 },
            [NL80211_STA_INFO_ACK_SIGNAL_AVG]      = { .type = NLA_U8 },
            [NL80211_STA_INFO_RX_MPDUS]            = { .type = NLA_U32 },
            [NL80211_STA_INFO_FCS_ERROR_COUNT]     = { .type = NLA_U32 },
            [NL80211_STA_INFO_CONNECTED_TO_GATE]   = { .type = NLA_FLAG },
            [NL80211_STA_INFO_TX_DURATION]         = { .type = NLA_U64 },
            [NL80211_STA_INFO_AIRTIME_WEIGHT]      = { .type = NLA_U16 },
            [NL80211_STA_INFO_AIRTIME_LINK_METRIC] = { .type = NLA_U32 },
            [NL80211_STA_INFO_ASSOC_AT_BOOTTIME]   = { .type = NLA_U64 },
        };

        printf("NL80211_ATTR_STA_INFO (%d): Len = %d\n", NL80211_ATTR_STA_INFO, nla_len(tb[NL80211_ATTR_STA_INFO]));
        /* Debug 用：只列出有哪些 STA_INFO child attributes */
    /*  nla_for_each_nested(attr, tb[NL80211_ATTR_STA_INFO], rem) {
            printf("  STA_INFO[%d]: Len = %d\n", nla_type(attr), nla_len(attr));
        }
    */
        memset(sta_info, 0, sizeof(sta_info));
        /* 假如有東西才開始解譯巢狀裡的資料 */
        ret = nla_parse_nested(sta_info, NL80211_STA_INFO_MAX, tb[NL80211_ATTR_STA_INFO], sta_info_policy);
        if (ret < 0) {
            printf("nla_parse_nested(STA_INFO) failed: %s (%d)\n", nl_geterror(ret), ret);

            return NL_SKIP;
        }

        if (sta_info[NL80211_STA_INFO_INACTIVE_TIME]) {
            printf("  NL80211_STA_INFO_INACTIVE_TIME (%d): %u ms\n", NL80211_STA_INFO_INACTIVE_TIME, nla_get_u32(sta_info[NL80211_STA_INFO_INACTIVE_TIME]));
        }

        if (sta_info[NL80211_STA_INFO_RX_BYTES]) {
            printf("  NL80211_STA_INFO_RX_BYTES (%d): %u Bytes\n", NL80211_STA_INFO_RX_BYTES, nla_get_u32(sta_info[NL80211_STA_INFO_RX_BYTES]));
        }

        if (sta_info[NL80211_STA_INFO_TX_BYTES]) {
            printf("  NL80211_STA_INFO_TX_BYTES (%d): %u Bytes\n", NL80211_STA_INFO_TX_BYTES, nla_get_u32(sta_info[NL80211_STA_INFO_TX_BYTES]));
        }

        if (sta_info[NL80211_STA_INFO_LLID]) {
            printf("  NL80211_STA_INFO_LLID (%d): %u\n", NL80211_STA_INFO_LLID, nla_get_u16(sta_info[NL80211_STA_INFO_LLID]));
        }
        
        if (sta_info[NL80211_STA_INFO_PLID]) {
            printf("  NL80211_STA_INFO_PLID (%d): %u\n", NL80211_STA_INFO_PLID, nla_get_u16(sta_info[NL80211_STA_INFO_PLID]));
        }

        if (sta_info[NL80211_STA_INFO_PLINK_STATE]) {
            printf("  NL80211_STA_INFO_PLINK_STATE (%d): %u\n", NL80211_STA_INFO_PLINK_STATE, nla_get_u8(sta_info[NL80211_STA_INFO_PLINK_STATE]));
        }

        if (sta_info[NL80211_STA_INFO_SIGNAL]) {
            printf("  NL80211_STA_INFO_SIGNAL (%d): %d dBm\n", NL80211_STA_INFO_SIGNAL, (int8_t)nla_get_u8(sta_info[NL80211_STA_INFO_SIGNAL]));
        }

        if (sta_info[NL80211_STA_INFO_TX_BITRATE]) {
            struct nlattr *rate[NL80211_RATE_INFO_MAX + 1];
            static struct nla_policy rate_info_policy[NL80211_RATE_INFO_MAX + 1] = {
                [NL80211_RATE_INFO_BITRATE]        = { .type = NLA_U16 },
                [NL80211_RATE_INFO_MCS]            = { .type = NLA_U8 },
                [NL80211_RATE_INFO_40_MHZ_WIDTH]   = { .type = NLA_FLAG },
                [NL80211_RATE_INFO_SHORT_GI]       = { .type = NLA_FLAG },
                [NL80211_RATE_INFO_BITRATE32]      = { .type = NLA_U32 },
                [NL80211_RATE_INFO_VHT_MCS]        = { .type = NLA_U8 },
                [NL80211_RATE_INFO_VHT_NSS]        = { .type = NLA_U8 },
                [NL80211_RATE_INFO_80_MHZ_WIDTH]   = { .type = NLA_FLAG },
                [NL80211_RATE_INFO_80P80_MHZ_WIDTH]= { .type = NLA_FLAG },
                [NL80211_RATE_INFO_160_MHZ_WIDTH]  = { .type = NLA_FLAG },
                [NL80211_RATE_INFO_10_MHZ_WIDTH]   = { .type = NLA_FLAG },
                [NL80211_RATE_INFO_5_MHZ_WIDTH]    = { .type = NLA_FLAG },
                [NL80211_RATE_INFO_HE_MCS]         = { .type = NLA_U8 },
                [NL80211_RATE_INFO_HE_NSS]         = { .type = NLA_U8 },
                [NL80211_RATE_INFO_HE_GI]          = { .type = NLA_U8 },
                [NL80211_RATE_INFO_HE_DCM]         = { .type = NLA_U8 },
                [NL80211_RATE_INFO_HE_RU_ALLOC]    = { .type = NLA_U8 },
#ifdef NL80211_RATE_INFO_EHT_MCS
                [NL80211_RATE_INFO_EHT_MCS]        = { .type = NLA_U8 },
#endif
#ifdef NL80211_RATE_INFO_EHT_NSS
                [NL80211_RATE_INFO_EHT_NSS]        = { .type = NLA_U8 },
#endif
#ifdef NL80211_RATE_INFO_EHT_GI
                [NL80211_RATE_INFO_EHT_GI]         = { .type = NLA_U8 },
#endif
#ifdef NL80211_RATE_INFO_EHT_RU_ALLOC
                [NL80211_RATE_INFO_EHT_RU_ALLOC]   = { .type = NLA_U8 },
#endif
            };

            printf("  NL80211_STA_INFO_TX_BITRATE (%d): Len = %d\n", NL80211_STA_INFO_TX_BITRATE, nla_len(sta_info[NL80211_STA_INFO_TX_BITRATE]));
            memset(rate, 0, sizeof(rate));
            ret = nla_parse_nested(rate, NL80211_RATE_INFO_MAX, sta_info[NL80211_STA_INFO_TX_BITRATE], rate_info_policy);
            if (ret < 0) {
                printf("  NL80211_RATE_INFO_MAX parse failed: %s (%d)\n", nl_geterror(ret), ret);

                return NL_SKIP;
            }

            if (rate[NL80211_RATE_INFO_BITRATE]) {
                uint16_t br = nla_get_u16(rate[NL80211_RATE_INFO_BITRATE]);

                printf("    NL80211_RATE_INFO_BITRATE (%d): %u.%u MBit/s\n", NL80211_RATE_INFO_BITRATE, br / 10, br % 10);
            }

            if (rate[NL80211_RATE_INFO_MCS]) {
                printf("    NL80211_RATE_INFO_MCS (%d): %u\n", NL80211_RATE_INFO_MCS, nla_get_u8(rate[NL80211_RATE_INFO_MCS]));
            }

            printf("    NL80211_RATE_INFO_40_MHZ_WIDTH (%d): %s\n",    NL80211_RATE_INFO_40_MHZ_WIDTH,    rate[NL80211_RATE_INFO_40_MHZ_WIDTH]? "YES": "NO");
            printf("    NL80211_RATE_INFO_SHORT_GI (%d): %s\n",        NL80211_RATE_INFO_SHORT_GI,        rate[NL80211_RATE_INFO_SHORT_GI]? "YES": "NO");

            if (rate[NL80211_RATE_INFO_BITRATE32]) {
                uint32_t br = nla_get_u32(rate[NL80211_RATE_INFO_BITRATE32]);

                printf("    NL80211_RATE_INFO_BITRATE32 (%d): %u.%u MBit/s\n", NL80211_RATE_INFO_BITRATE32, br / 10, br % 10);
            }

            if (rate[NL80211_RATE_INFO_VHT_MCS]) {
                printf("    NL80211_RATE_INFO_VHT_MCS (%d): %u\n", NL80211_RATE_INFO_VHT_MCS, nla_get_u8(rate[NL80211_RATE_INFO_VHT_MCS]));
            }

            if (rate[NL80211_RATE_INFO_VHT_NSS]) {
                printf("    NL80211_RATE_INFO_VHT_NSS (%d): %u\n", NL80211_RATE_INFO_VHT_NSS, nla_get_u8(rate[NL80211_RATE_INFO_VHT_NSS]));
            }

            printf("    NL80211_RATE_INFO_80_MHZ_WIDTH (%d): %s\n",    NL80211_RATE_INFO_80_MHZ_WIDTH,    rate[NL80211_RATE_INFO_80_MHZ_WIDTH]? "YES": "NO");
            printf("    NL80211_RATE_INFO_80P80_MHZ_WIDTH (%d): %s\n", NL80211_RATE_INFO_80P80_MHZ_WIDTH, rate[NL80211_RATE_INFO_80P80_MHZ_WIDTH]? "YES": "NO");
            printf("    NL80211_RATE_INFO_160_MHZ_WIDTH (%d): %s\n",   NL80211_RATE_INFO_160_MHZ_WIDTH,   rate[NL80211_RATE_INFO_160_MHZ_WIDTH]? "YES": "NO");
            printf("    NL80211_RATE_INFO_10_MHZ_WIDTH (%d): %s\n",    NL80211_RATE_INFO_10_MHZ_WIDTH,    rate[NL80211_RATE_INFO_10_MHZ_WIDTH]? "YES": "NO");
            printf("    NL80211_RATE_INFO_5_MHZ_WIDTH (%d): %s\n",     NL80211_RATE_INFO_5_MHZ_WIDTH,     rate[NL80211_RATE_INFO_5_MHZ_WIDTH]? "YES": "NO");

            if (rate[NL80211_RATE_INFO_HE_MCS]) {
                printf("    NL80211_RATE_INFO_HE_MCS (%d): %u\n", NL80211_RATE_INFO_HE_MCS, nla_get_u8(rate[NL80211_RATE_INFO_HE_MCS]));
            }

            if (rate[NL80211_RATE_INFO_HE_NSS]) {
                printf("    NL80211_RATE_INFO_HE_NSS (%d): %u\n", NL80211_RATE_INFO_HE_NSS, nla_get_u8(rate[NL80211_RATE_INFO_HE_NSS]));
            }

            if (rate[NL80211_RATE_INFO_HE_GI]) {
                printf("    NL80211_RATE_INFO_HE_GI (%d): %u\n", NL80211_RATE_INFO_HE_GI, nla_get_u8(rate[NL80211_RATE_INFO_HE_GI]));
            }

            if (rate[NL80211_RATE_INFO_HE_DCM]) {
                printf("    NL80211_RATE_INFO_HE_DCM (%d): %u\n", NL80211_RATE_INFO_HE_DCM, nla_get_u8(rate[NL80211_RATE_INFO_HE_DCM]));
            }

            if (rate[NL80211_RATE_INFO_HE_RU_ALLOC]) {
                printf("    NL80211_RATE_INFO_HE_RU_ALLOC (%d): %u\n", NL80211_RATE_INFO_HE_RU_ALLOC, nla_get_u8(rate[NL80211_RATE_INFO_HE_RU_ALLOC]));
            }
        }

        if (sta_info[NL80211_STA_INFO_RX_PACKETS]) {
            printf("  NL80211_STA_INFO_RX_PACKETS (%d): %u Packets\n", NL80211_STA_INFO_RX_PACKETS, nla_get_u32(sta_info[NL80211_STA_INFO_RX_PACKETS]));
        }

        if (sta_info[NL80211_STA_INFO_TX_PACKETS]) {
            printf("  NL80211_STA_INFO_TX_PACKETS (%d): %u Packets\n", NL80211_STA_INFO_TX_PACKETS, nla_get_u32(sta_info[NL80211_STA_INFO_TX_PACKETS]));
        }

        if (sta_info[NL80211_STA_INFO_TX_RETRIES]) {
            printf("  NL80211_STA_INFO_TX_RETRIES (%d): %u\n", NL80211_STA_INFO_TX_RETRIES, nla_get_u32(sta_info[NL80211_STA_INFO_TX_RETRIES]));
        }

        if (sta_info[NL80211_STA_INFO_TX_FAILED]) {
            printf("  NL80211_STA_INFO_TX_FAILED (%d): %u\n", NL80211_STA_INFO_TX_FAILED, nla_get_u32(sta_info[NL80211_STA_INFO_TX_FAILED]));
        }

        if (sta_info[NL80211_STA_INFO_SIGNAL_AVG]) {
            printf("  NL80211_STA_INFO_SIGNAL_AVG (%d): %d dBm\n", NL80211_STA_INFO_SIGNAL_AVG, (int8_t)nla_get_u8(sta_info[NL80211_STA_INFO_SIGNAL_AVG]));
        }

        if (sta_info[NL80211_STA_INFO_RX_BITRATE]) {
            struct nlattr *rate[NL80211_RATE_INFO_MAX + 1];
            static struct nla_policy rate_info_policy[NL80211_RATE_INFO_MAX + 1] = {
                [NL80211_RATE_INFO_BITRATE]        = { .type = NLA_U16 },
                [NL80211_RATE_INFO_MCS]            = { .type = NLA_U8 },
                [NL80211_RATE_INFO_40_MHZ_WIDTH]   = { .type = NLA_FLAG },
                [NL80211_RATE_INFO_SHORT_GI]       = { .type = NLA_FLAG },
                [NL80211_RATE_INFO_BITRATE32]      = { .type = NLA_U32 },
                [NL80211_RATE_INFO_VHT_MCS]        = { .type = NLA_U8 },
                [NL80211_RATE_INFO_VHT_NSS]        = { .type = NLA_U8 },
                [NL80211_RATE_INFO_80_MHZ_WIDTH]   = { .type = NLA_FLAG },
                [NL80211_RATE_INFO_80P80_MHZ_WIDTH]= { .type = NLA_FLAG },
                [NL80211_RATE_INFO_160_MHZ_WIDTH]  = { .type = NLA_FLAG },
                [NL80211_RATE_INFO_10_MHZ_WIDTH]   = { .type = NLA_FLAG },
                [NL80211_RATE_INFO_5_MHZ_WIDTH]    = { .type = NLA_FLAG },
                [NL80211_RATE_INFO_HE_MCS]         = { .type = NLA_U8 },
                [NL80211_RATE_INFO_HE_NSS]         = { .type = NLA_U8 },
                [NL80211_RATE_INFO_HE_GI]          = { .type = NLA_U8 },
                [NL80211_RATE_INFO_HE_DCM]         = { .type = NLA_U8 },
                [NL80211_RATE_INFO_HE_RU_ALLOC]    = { .type = NLA_U8 },
#ifdef NL80211_RATE_INFO_EHT_MCS
                [NL80211_RATE_INFO_EHT_MCS]        = { .type = NLA_U8 },
#endif
#ifdef NL80211_RATE_INFO_EHT_NSS
                [NL80211_RATE_INFO_EHT_NSS]        = { .type = NLA_U8 },
#endif
#ifdef NL80211_RATE_INFO_EHT_GI
                [NL80211_RATE_INFO_EHT_GI]         = { .type = NLA_U8 },
#endif
#ifdef NL80211_RATE_INFO_EHT_RU_ALLOC
                [NL80211_RATE_INFO_EHT_RU_ALLOC]   = { .type = NLA_U8 },
#endif
            };

            printf("  NL80211_STA_INFO_RX_BITRATE (%d): Len = %d\n", NL80211_STA_INFO_RX_BITRATE, nla_len(sta_info[NL80211_STA_INFO_RX_BITRATE]));
            memset(rate, 0, sizeof(rate));
            ret = nla_parse_nested(rate, NL80211_RATE_INFO_MAX, sta_info[NL80211_STA_INFO_RX_BITRATE], rate_info_policy);
            if (ret < 0) {
                printf("  NL80211_RATE_INFO_MAX parse failed: %s (%d)\n", nl_geterror(ret), ret);

                return NL_SKIP;
            }

            if (rate[NL80211_RATE_INFO_BITRATE]) {
                uint16_t br = nla_get_u16(rate[NL80211_RATE_INFO_BITRATE]);

                printf("    NL80211_RATE_INFO_BITRATE (%d): %u.%u MBit/s\n", NL80211_RATE_INFO_BITRATE, br / 10, br % 10);
            }

            if (rate[NL80211_RATE_INFO_MCS]) {
                printf("    NL80211_RATE_INFO_MCS (%d): %u\n", NL80211_RATE_INFO_MCS, nla_get_u8(rate[NL80211_RATE_INFO_MCS]));
            }

            printf("    NL80211_RATE_INFO_40_MHZ_WIDTH (%d): %s\n",    NL80211_RATE_INFO_40_MHZ_WIDTH,    rate[NL80211_RATE_INFO_40_MHZ_WIDTH]? "YES": "NO");
            printf("    NL80211_RATE_INFO_SHORT_GI (%d): %s\n",        NL80211_RATE_INFO_SHORT_GI,        rate[NL80211_RATE_INFO_SHORT_GI]? "YES": "NO");

            if (rate[NL80211_RATE_INFO_BITRATE32]) {
                uint32_t br = nla_get_u32(rate[NL80211_RATE_INFO_BITRATE32]);

                printf("    NL80211_RATE_INFO_BITRATE32 (%d): %u.%u MBit/s\n", NL80211_RATE_INFO_BITRATE32, br / 10, br % 10);
            }

            if (rate[NL80211_RATE_INFO_VHT_MCS]) {
                printf("    NL80211_RATE_INFO_VHT_MCS (%d): %u\n", NL80211_RATE_INFO_VHT_MCS, nla_get_u8(rate[NL80211_RATE_INFO_VHT_MCS]));
            }

            if (rate[NL80211_RATE_INFO_VHT_NSS]) {
                printf("    NL80211_RATE_INFO_VHT_NSS (%d): %u\n", NL80211_RATE_INFO_VHT_NSS, nla_get_u8(rate[NL80211_RATE_INFO_VHT_NSS]));
            }

            printf("    NL80211_RATE_INFO_80_MHZ_WIDTH (%d): %s\n",    NL80211_RATE_INFO_80_MHZ_WIDTH,    rate[NL80211_RATE_INFO_80_MHZ_WIDTH]? "YES": "NO");
            printf("    NL80211_RATE_INFO_80P80_MHZ_WIDTH (%d): %s\n", NL80211_RATE_INFO_80P80_MHZ_WIDTH, rate[NL80211_RATE_INFO_80P80_MHZ_WIDTH]? "YES": "NO");
            printf("    NL80211_RATE_INFO_160_MHZ_WIDTH (%d): %s\n",   NL80211_RATE_INFO_160_MHZ_WIDTH,   rate[NL80211_RATE_INFO_160_MHZ_WIDTH]? "YES": "NO");
            printf("    NL80211_RATE_INFO_10_MHZ_WIDTH (%d): %s\n",    NL80211_RATE_INFO_10_MHZ_WIDTH,    rate[NL80211_RATE_INFO_10_MHZ_WIDTH]? "YES": "NO");
            printf("    NL80211_RATE_INFO_5_MHZ_WIDTH (%d): %s\n",     NL80211_RATE_INFO_5_MHZ_WIDTH,     rate[NL80211_RATE_INFO_5_MHZ_WIDTH]? "YES": "NO");

            if (rate[NL80211_RATE_INFO_HE_MCS]) {
                printf("    NL80211_RATE_INFO_HE_MCS (%d): %u\n", NL80211_RATE_INFO_HE_MCS, nla_get_u8(rate[NL80211_RATE_INFO_HE_MCS]));
            }

            if (rate[NL80211_RATE_INFO_HE_NSS]) {
                printf("    NL80211_RATE_INFO_HE_NSS (%d): %u\n", NL80211_RATE_INFO_HE_NSS, nla_get_u8(rate[NL80211_RATE_INFO_HE_NSS]));
            }

            if (rate[NL80211_RATE_INFO_HE_GI]) {
                printf("    NL80211_RATE_INFO_HE_GI (%d): %u\n", NL80211_RATE_INFO_HE_GI, nla_get_u8(rate[NL80211_RATE_INFO_HE_GI]));
            }

            if (rate[NL80211_RATE_INFO_HE_DCM]) {
                printf("    NL80211_RATE_INFO_HE_DCM (%d): %u\n", NL80211_RATE_INFO_HE_DCM, nla_get_u8(rate[NL80211_RATE_INFO_HE_DCM]));
            }

            if (rate[NL80211_RATE_INFO_HE_RU_ALLOC]) {
                printf("    NL80211_RATE_INFO_HE_RU_ALLOC (%d): %u\n", NL80211_RATE_INFO_HE_RU_ALLOC, nla_get_u8(rate[NL80211_RATE_INFO_HE_RU_ALLOC]));
            }
        }

        if (sta_info[NL80211_STA_INFO_BSS_PARAM]) {
            struct nlattr *bss[NL80211_STA_BSS_PARAM_MAX + 1];
            static struct nla_policy bss_param_policy[NL80211_STA_BSS_PARAM_MAX + 1] = {
                [NL80211_STA_BSS_PARAM_CTS_PROT]        = { .type = NLA_FLAG },
                [NL80211_STA_BSS_PARAM_SHORT_PREAMBLE]  = { .type = NLA_FLAG },
                [NL80211_STA_BSS_PARAM_SHORT_SLOT_TIME] = { .type = NLA_FLAG },
                [NL80211_STA_BSS_PARAM_DTIM_PERIOD]     = { .type = NLA_U8 },
                [NL80211_STA_BSS_PARAM_BEACON_INTERVAL] = { .type = NLA_U16 },
            };

            printf("  NL80211_STA_INFO_BSS_PARAM (%d): Len = %d\n", NL80211_STA_INFO_BSS_PARAM, nla_len(sta_info[NL80211_STA_INFO_BSS_PARAM]));
            memset(bss, 0, sizeof(bss));
            ret = nla_parse_nested(bss, NL80211_STA_BSS_PARAM_MAX, sta_info[NL80211_STA_INFO_BSS_PARAM], bss_param_policy);
            if (ret < 0) {
                printf("  NL80211_STA_INFO_BSS_PARAM parse failed: %s (%d)\n", nl_geterror(ret), ret);

                return NL_SKIP;
            }

            printf("    NL80211_STA_BSS_PARAM_CTS_PROT (%d): %s\n",        NL80211_STA_BSS_PARAM_CTS_PROT,        bss[NL80211_STA_BSS_PARAM_CTS_PROT]? "YES": "NO");
            printf("    NL80211_STA_BSS_PARAM_SHORT_PREAMBLE (%d): %s\n",  NL80211_STA_BSS_PARAM_SHORT_PREAMBLE,  bss[NL80211_STA_BSS_PARAM_SHORT_PREAMBLE]? "YES": "NO");
            printf("    NL80211_STA_BSS_PARAM_SHORT_SLOT_TIME (%d): %s\n", NL80211_STA_BSS_PARAM_SHORT_SLOT_TIME, bss[NL80211_STA_BSS_PARAM_SHORT_SLOT_TIME]? "YES": "NO");

            if (bss[NL80211_STA_BSS_PARAM_DTIM_PERIOD]) {
                printf("    NL80211_STA_BSS_PARAM_DTIM_PERIOD (%d): %u µs\n", NL80211_STA_BSS_PARAM_DTIM_PERIOD, nla_get_u8(bss[NL80211_STA_BSS_PARAM_DTIM_PERIOD]));
            }

            if (bss[NL80211_STA_BSS_PARAM_BEACON_INTERVAL]) {
                printf("    NL80211_STA_BSS_PARAM_BEACON_INTERVAL (%d): %u µs\n", NL80211_STA_BSS_PARAM_BEACON_INTERVAL, nla_get_u16(bss[NL80211_STA_BSS_PARAM_BEACON_INTERVAL]));
            }
        }

        if (sta_info[NL80211_STA_INFO_CONNECTED_TIME]) {
            printf("  NL80211_STA_INFO_CONNECTED_TIME (%d): %u seconds\n", NL80211_STA_INFO_CONNECTED_TIME, nla_get_u32(sta_info[NL80211_STA_INFO_CONNECTED_TIME]));
        }

        if (sta_info[NL80211_STA_INFO_STA_FLAGS]) {
            struct nlattr *flags[NL80211_STA_FLAG_MAX + 1];
            static struct nla_policy sta_flag_policy[NL80211_STA_FLAG_MAX + 1] = {
                [NL80211_STA_FLAG_AUTHORIZED]     = { .type = NLA_FLAG },
                [NL80211_STA_FLAG_SHORT_PREAMBLE] = { .type = NLA_FLAG },
                [NL80211_STA_FLAG_WME]            = { .type = NLA_FLAG },
                [NL80211_STA_FLAG_MFP]            = { .type = NLA_FLAG },
                [NL80211_STA_FLAG_AUTHENTICATED]  = { .type = NLA_FLAG },
                [NL80211_STA_FLAG_TDLS_PEER]      = { .type = NLA_FLAG },
                [NL80211_STA_FLAG_ASSOCIATED]     = { .type = NLA_FLAG },
            };

            /* Display as same as WireShark */
            uint64_t raw = 0;
            int len = nla_len(sta_info[NL80211_STA_INFO_STA_FLAGS]);
            size_t copy_len;

            copy_len = (len < (int)sizeof(raw)) ? (size_t)len : sizeof(raw);
            memcpy(&raw, nla_data(sta_info[NL80211_STA_INFO_STA_FLAGS]), copy_len);
            printf("  NL80211_STA_INFO_STA_FLAGS (%d): 0x%016llx (%llu)\n", NL80211_STA_INFO_STA_FLAGS, (unsigned long long)raw, (unsigned long long)raw);

            memset(flags, 0, sizeof(flags));
            ret = nla_parse_nested(flags, NL80211_STA_FLAG_MAX, sta_info[NL80211_STA_INFO_STA_FLAGS], sta_flag_policy);
            if (ret < 0) {
                printf("  NL80211_STA_INFO_STA_FLAGS parse failed: %s (%d)\n", nl_geterror(ret), ret);

                return NL_SKIP;
            }

            printf("    NL80211_STA_FLAG_AUTHORIZED (%d): %s\n",     NL80211_STA_FLAG_AUTHORIZED,     (flags[NL80211_STA_FLAG_AUTHORIZED])? "NO": "YES");
            printf("    NL80211_STA_FLAG_SHORT_PREAMBLE (%d): %s\n", NL80211_STA_FLAG_SHORT_PREAMBLE, (flags[NL80211_STA_FLAG_SHORT_PREAMBLE])? "Short": "Long");
            printf("    NL80211_STA_FLAG_WME (%d): %s\n",            NL80211_STA_FLAG_WME,            (flags[NL80211_STA_FLAG_WME])? "NO": "YES");
            printf("    NL80211_STA_FLAG_MFP (%d): %s\n",            NL80211_STA_FLAG_MFP,            (flags[NL80211_STA_FLAG_MFP])? "YES": "NO");
            printf("    NL80211_STA_FLAG_AUTHENTICATED (%d): %s\n",  NL80211_STA_FLAG_AUTHENTICATED,  (flags[NL80211_STA_FLAG_AUTHENTICATED])? "NO": "YES");
            printf("    NL80211_STA_FLAG_TDLS_PEER (%d): %s\n",      NL80211_STA_FLAG_TDLS_PEER,      (flags[NL80211_STA_FLAG_TDLS_PEER])? "YES": "NO");
            printf("    NL80211_STA_FLAG_ASSOCIATED (%d): %s\n",     NL80211_STA_FLAG_ASSOCIATED,     (flags[NL80211_STA_FLAG_ASSOCIATED])? "NO": "YES");
        }

        if (sta_info[NL80211_STA_INFO_BEACON_LOSS]) {
            printf("  NL80211_STA_INFO_BEACON_LOSS (%d): %u count\n", NL80211_STA_INFO_BEACON_LOSS, nla_get_u32(sta_info[NL80211_STA_INFO_BEACON_LOSS]));
        }

        if (sta_info[NL80211_STA_INFO_T_OFFSET]) {
            printf("  NL80211_STA_INFO_T_OFFSET (%d): %lld (timing offset)\n", NL80211_STA_INFO_T_OFFSET, (unsigned long long)nla_get_u64(sta_info[NL80211_STA_INFO_T_OFFSET]));
        }

        if (sta_info[NL80211_STA_INFO_LOCAL_PM]) {
            printf("  NL80211_STA_INFO_LOCAL_PM (%d): %u (power mode enum)\n", NL80211_STA_INFO_LOCAL_PM, nla_get_u32(sta_info[NL80211_STA_INFO_LOCAL_PM]));
        }

        if (sta_info[NL80211_STA_INFO_PEER_PM]) {
            printf("  NL80211_STA_INFO_PEER_PM (%d): %u (power mode enum)\n", NL80211_STA_INFO_PEER_PM, nla_get_u32(sta_info[NL80211_STA_INFO_PEER_PM]));
        }

        if (sta_info[NL80211_STA_INFO_NONPEER_PM]) {
            printf("  NL80211_STA_INFO_NONPEER_PM (%d): %u (power mode enum))\n", NL80211_STA_INFO_NONPEER_PM, nla_get_u32(sta_info[NL80211_STA_INFO_NONPEER_PM]));
        }

        if (sta_info[NL80211_STA_INFO_RX_BYTES64]) {
            printf("  NL80211_STA_INFO_RX_BYTES64 (%d): %llu Bytes\n", NL80211_STA_INFO_RX_BYTES64, (unsigned long long)nla_get_u64(sta_info[NL80211_STA_INFO_RX_BYTES64]));
        }

        if (sta_info[NL80211_STA_INFO_TX_BYTES64]) {
            printf("  NL80211_STA_INFO_TX_BYTES64 (%d): %llu Bytes\n", NL80211_STA_INFO_TX_BYTES64, (unsigned long long)nla_get_u64(sta_info[NL80211_STA_INFO_TX_BYTES64]));
        }

        if (sta_info[NL80211_STA_INFO_CHAIN_SIGNAL]) {
            struct nlattr *chain = NULL;
            int rem_chain = -1;

            printf("  NL80211_STA_INFO_CHAIN_SIGNAL (%d): Len = %d\n", NL80211_STA_INFO_CHAIN_SIGNAL, nla_len(sta_info[NL80211_STA_INFO_CHAIN_SIGNAL]));
            nla_for_each_nested(chain, sta_info[NL80211_STA_INFO_CHAIN_SIGNAL], rem_chain) {
                printf("    Chain[%d]: %d dBm\n", nla_type(chain), (int8_t)nla_get_u8(chain));
            }
        }

        if (sta_info[NL80211_STA_INFO_CHAIN_SIGNAL_AVG]) {
            struct nlattr *chain = NULL;
            int rem_chain = -1;

            printf("  NL80211_STA_INFO_CHAIN_SIGNAL_AVG (%d): Len = %d\n", NL80211_STA_INFO_CHAIN_SIGNAL_AVG, nla_len(sta_info[NL80211_STA_INFO_CHAIN_SIGNAL_AVG]));
            nla_for_each_nested(chain, sta_info[NL80211_STA_INFO_CHAIN_SIGNAL_AVG], rem_chain) {
                printf("    Chain[%d]: %d dBm\n", nla_type(chain), (int8_t)nla_get_u8(chain));
            }
        }

        if (sta_info[NL80211_STA_INFO_EXPECTED_THROUGHPUT]) {
            printf("  NL80211_STA_INFO_EXPECTED_THROUGHPUT (%d): %u \n", NL80211_STA_INFO_EXPECTED_THROUGHPUT, nla_get_u32(sta_info[NL80211_STA_INFO_EXPECTED_THROUGHPUT]));
        }

        if (sta_info[NL80211_STA_INFO_RX_DROP_MISC]) {
            printf("  NL80211_STA_INFO_RX_DROP_MISC (%d): %llu\n", NL80211_STA_INFO_RX_DROP_MISC, (unsigned long long)nla_get_u64(sta_info[NL80211_STA_INFO_RX_DROP_MISC]));
        }

        if (sta_info[NL80211_STA_INFO_BEACON_RX]) {
            printf("  NL80211_STA_INFO_BEACON_RX (%d): %llu\n", NL80211_STA_INFO_BEACON_RX, (unsigned long long)nla_get_u64(sta_info[NL80211_STA_INFO_BEACON_RX]));
        }

        if (sta_info[NL80211_STA_INFO_BEACON_SIGNAL_AVG]) {
            printf("  NL80211_STA_INFO_BEACON_SIGNAL_AVG (%d): %d dBm\n", NL80211_STA_INFO_BEACON_SIGNAL_AVG, (int8_t)nla_get_u8(sta_info[NL80211_STA_INFO_BEACON_SIGNAL_AVG]));
        }

        if (sta_info[NL80211_STA_INFO_TID_STATS]) {
            struct nlattr *tid = NULL;
            static struct nla_policy tid_stats_policy[NL80211_TID_STATS_MAX + 1] = {
                [NL80211_TID_STATS_RX_MSDU]         = { .type = NLA_U64 },
                [NL80211_TID_STATS_TX_MSDU]         = { .type = NLA_U64 },
                [NL80211_TID_STATS_TX_MSDU_RETRIES] = { .type = NLA_U64 },
                [NL80211_TID_STATS_TX_MSDU_FAILED]  = { .type = NLA_U64 },
                [NL80211_TID_STATS_PAD]             = { .type = NLA_UNSPEC },
                [NL80211_TID_STATS_TXQ_STATS]       = { .type = NLA_NESTED },
            };
            int rem_tid = -1;

            printf("  NL80211_STA_INFO_TID_STATS (%d): Len = %d\n", NL80211_STA_INFO_TID_STATS, nla_len(sta_info[NL80211_STA_INFO_TID_STATS]));
            nla_for_each_nested(tid, sta_info[NL80211_STA_INFO_TID_STATS], rem_tid) {
                struct nlattr *tid_stats[NL80211_TID_STATS_MAX + 1];

                memset(tid_stats, 0, sizeof(tid_stats));
                ret = nla_parse_nested(tid_stats, NL80211_TID_STATS_MAX, tid, tid_stats_policy);
                if (ret < 0) {
                    printf("    TID[%d]: nla_parse_nested() failed: %s (%d)\n", nla_type(tid), nl_geterror(ret), ret);
                    continue;
                }

                printf("    TID[%d]: Len = %d\n", nla_type(tid), nla_len(tid));
                if (tid_stats[NL80211_TID_STATS_RX_MSDU]) {
                    printf("      NL80211_TID_STATS_RX_MSDU (%d): %llu\n", NL80211_TID_STATS_RX_MSDU, (unsigned long long)nla_get_u64(tid_stats[NL80211_TID_STATS_RX_MSDU]));
                }

                if (tid_stats[NL80211_TID_STATS_TX_MSDU]) {
                    printf("      NL80211_TID_STATS_TX_MSDU (%d): %llu\n", NL80211_TID_STATS_TX_MSDU, (unsigned long long)nla_get_u64(tid_stats[NL80211_TID_STATS_TX_MSDU]));
                }

                if (tid_stats[NL80211_TID_STATS_TX_MSDU_RETRIES]) {
                    printf("      NL80211_TID_STATS_TX_MSDU_RETRIES (%d): %llu\n", NL80211_TID_STATS_TX_MSDU_RETRIES, (unsigned long long)nla_get_u64(tid_stats[NL80211_TID_STATS_TX_MSDU_RETRIES]));
                }

                if (tid_stats[NL80211_TID_STATS_TX_MSDU_FAILED]) {
                    printf("      NL80211_TID_STATS_TX_MSDU_FAILED (%d): %llu\n", NL80211_TID_STATS_TX_MSDU_FAILED, (unsigned long long)nla_get_u64(tid_stats[NL80211_TID_STATS_TX_MSDU_FAILED]));
                }

                if (tid_stats[NL80211_TID_STATS_PAD]) {
                    printf("      NL80211_TID_STATS_PAD (%d): Len = %d\n", NL80211_TID_STATS_PAD, nla_len(tid_stats[NL80211_TID_STATS_PAD]));
                }

                if (tid_stats[NL80211_TID_STATS_TXQ_STATS]) {
                    static struct nla_policy txq_stats_policy[NL80211_TXQ_STATS_MAX + 1] = {
                        [NL80211_TXQ_STATS_BACKLOG_BYTES]   = { .type = NLA_U32 },
                        [NL80211_TXQ_STATS_BACKLOG_PACKETS] = { .type = NLA_U32 },
                        [NL80211_TXQ_STATS_FLOWS]           = { .type = NLA_U32 },
                        [NL80211_TXQ_STATS_DROPS]           = { .type = NLA_U32 },
                        [NL80211_TXQ_STATS_ECN_MARKS]       = { .type = NLA_U32 },
                        [NL80211_TXQ_STATS_OVERLIMIT]       = { .type = NLA_U32 },
                        [NL80211_TXQ_STATS_OVERMEMORY]      = { .type = NLA_U32 },
                        [NL80211_TXQ_STATS_COLLISIONS]      = { .type = NLA_U32 },
                        [NL80211_TXQ_STATS_TX_BYTES]        = { .type = NLA_U32 },
                        [NL80211_TXQ_STATS_TX_PACKETS]      = { .type = NLA_U32 },
                        [NL80211_TXQ_STATS_MAX_FLOWS]       = { .type = NLA_U32 },
                    };
                    struct nlattr *txq_stats[NL80211_TXQ_STATS_MAX + 1];

                    memset(txq_stats, 0, sizeof(txq_stats));
                    printf("      NL80211_TID_STATS_TXQ_STATS (%d): Len = %d\n", NL80211_TID_STATS_TXQ_STATS, nla_len(tid_stats[NL80211_TID_STATS_TXQ_STATS]));
                    ret = nla_parse_nested(txq_stats, NL80211_TXQ_STATS_MAX, tid_stats[NL80211_TID_STATS_TXQ_STATS], txq_stats_policy);
                    if (ret < 0) {
                        printf("      NL80211_TID_STATS_TXQ_STATS parse failed: %s (%d)\n", nl_geterror(ret), ret);

                        return NL_SKIP;
                    }

                    if (txq_stats[NL80211_TXQ_STATS_BACKLOG_BYTES]) {
                        printf("        NL80211_TXQ_STATS_BACKLOG_BYTES (%d): %u\n", NL80211_TXQ_STATS_BACKLOG_BYTES, nla_get_u32(txq_stats[NL80211_TXQ_STATS_BACKLOG_BYTES]));
                    }

                    if (txq_stats[NL80211_TXQ_STATS_BACKLOG_PACKETS]) {
                        printf("        NL80211_TXQ_STATS_BACKLOG_PACKETS (%d): %u\n", NL80211_TXQ_STATS_BACKLOG_PACKETS, nla_get_u32(txq_stats[NL80211_TXQ_STATS_BACKLOG_PACKETS]));
                    }

                    if (txq_stats[NL80211_TXQ_STATS_FLOWS]) {
                        printf("        NL80211_TXQ_STATS_FLOWS (%d): %u\n", NL80211_TXQ_STATS_FLOWS, nla_get_u32(txq_stats[NL80211_TXQ_STATS_FLOWS]));
                    }

                    if (txq_stats[NL80211_TXQ_STATS_DROPS]) {
                        printf("        NL80211_TXQ_STATS_DROPS (%d): %u\n", NL80211_TXQ_STATS_DROPS, nla_get_u32(txq_stats[NL80211_TXQ_STATS_DROPS]));
                    }

                    if (txq_stats[NL80211_TXQ_STATS_ECN_MARKS]) {
                        printf("        NL80211_TXQ_STATS_ECN_MARKS (%d): %u\n", NL80211_TXQ_STATS_ECN_MARKS, nla_get_u32(txq_stats[NL80211_TXQ_STATS_ECN_MARKS]));
                    }

                    if (txq_stats[NL80211_TXQ_STATS_OVERLIMIT]) {
                        printf("        NL80211_TXQ_STATS_OVERLIMIT (%d): %u\n", NL80211_TXQ_STATS_OVERLIMIT, nla_get_u32(txq_stats[NL80211_TXQ_STATS_OVERLIMIT]));
                    }

                    if (txq_stats[NL80211_TXQ_STATS_OVERMEMORY]) {
                        printf("        NL80211_TXQ_STATS_OVERMEMORY (%d): %u\n", NL80211_TXQ_STATS_OVERMEMORY, nla_get_u32(txq_stats[NL80211_TXQ_STATS_OVERMEMORY]));
                    }

                    if (txq_stats[NL80211_TXQ_STATS_COLLISIONS]) {
                        printf("        NL80211_TXQ_STATS_COLLISIONS (%d): %u\n", NL80211_TXQ_STATS_COLLISIONS, nla_get_u32(txq_stats[NL80211_TXQ_STATS_COLLISIONS]));
                    }

                    if (txq_stats[NL80211_TXQ_STATS_TX_BYTES]) {
                        printf("        NL80211_TXQ_STATS_TX_BYTES (%d): %u\n", NL80211_TXQ_STATS_TX_BYTES, nla_get_u32(txq_stats[NL80211_TXQ_STATS_TX_BYTES]));
                    }

                    if (txq_stats[NL80211_TXQ_STATS_TX_PACKETS]) {
                        printf("        NL80211_TXQ_STATS_TX_PACKETS (%d): %u\n", NL80211_TXQ_STATS_TX_PACKETS, nla_get_u32(txq_stats[NL80211_TXQ_STATS_TX_PACKETS]));
                    }

                    if (txq_stats[NL80211_TXQ_STATS_MAX_FLOWS]) {
                        printf("        NL80211_TXQ_STATS_MAX_FLOWS (%d): %u\n", NL80211_TXQ_STATS_MAX_FLOWS, nla_get_u32(txq_stats[NL80211_TXQ_STATS_MAX_FLOWS]));
                    }
                }
            }
        }

        if (sta_info[NL80211_STA_INFO_RX_DURATION]) {
            printf("  NL80211_STA_INFO_RX_DURATION (%d): %llu µs\n", NL80211_STA_INFO_RX_DURATION, (unsigned long long)nla_get_u64(sta_info[NL80211_STA_INFO_RX_DURATION]));
        }

        if (sta_info[NL80211_STA_INFO_PAD]) {
            printf("  NL80211_STA_INFO_PAD (%d): len = %d\n", NL80211_STA_INFO_PAD, nla_len(sta_info[NL80211_STA_INFO_PAD]));
        }

        if (sta_info[NL80211_STA_INFO_ACK_SIGNAL]) {
            printf("  NL80211_STA_INFO_ACK_SIGNAL (%d): %d dBm\n", NL80211_STA_INFO_ACK_SIGNAL, (int8_t)nla_get_u8(sta_info[NL80211_STA_INFO_ACK_SIGNAL]));
        }

        if (sta_info[NL80211_STA_INFO_ACK_SIGNAL_AVG]) {
            printf("  NL80211_STA_INFO_ACK_SIGNAL_AVG (%d): %d dBb\n", NL80211_STA_INFO_ACK_SIGNAL_AVG, (int8_t)nla_get_u8(sta_info[NL80211_STA_INFO_ACK_SIGNAL_AVG]));
        }

        if (sta_info[NL80211_STA_INFO_RX_MPDUS]) {
            printf("  NL80211_STA_INFO_RX_MPDUS (%d): %u count\n", NL80211_STA_INFO_RX_MPDUS, nla_get_u32(sta_info[NL80211_STA_INFO_RX_MPDUS]));
        }

        if (sta_info[NL80211_STA_INFO_FCS_ERROR_COUNT]) {
            printf("  NL80211_STA_INFO_FCS_ERROR_COUNT (%d): %u count\n", NL80211_STA_INFO_FCS_ERROR_COUNT, nla_get_u32(sta_info[NL80211_STA_INFO_FCS_ERROR_COUNT]));
        }

        if (sta_info[NL80211_STA_INFO_CONNECTED_TO_GATE]) {
            printf("  NL80211_STA_INFO_CONNECTED_TO_GATE (%d): Present\n", NL80211_STA_INFO_CONNECTED_TO_GATE);
        }

        if (sta_info[NL80211_STA_INFO_TX_DURATION]) {
            printf("  NL80211_STA_INFO_TX_DURATION (%d): %llu µs\n", NL80211_STA_INFO_TX_DURATION, (unsigned long long)nla_get_u64(sta_info[NL80211_STA_INFO_TX_DURATION]));
        }

        if (sta_info[NL80211_STA_INFO_AIRTIME_WEIGHT]) {
            printf("  NL80211_STA_INFO_AIRTIME_WEIGHT (%d): %u weight\n", NL80211_STA_INFO_AIRTIME_WEIGHT, nla_get_u16(sta_info[NL80211_STA_INFO_AIRTIME_WEIGHT]));
        }

        if(sta_info[NL80211_STA_INFO_AIRTIME_LINK_METRIC]) {
            printf("  NL80211_STA_INFO_AIRTIME_LINK_METRIC (%d): %u Metric\n", NL80211_STA_INFO_AIRTIME_LINK_METRIC, nla_get_u32(sta_info[NL80211_STA_INFO_AIRTIME_LINK_METRIC]));
        }

        if (sta_info[NL80211_STA_INFO_ASSOC_AT_BOOTTIME]) {
            printf("  NL80211_STA_INFO_ASSOC_AT_BOOTTIME (%d): %llu ns\n", NL80211_STA_INFO_ASSOC_AT_BOOTTIME, (unsigned long long)nla_get_u64(sta_info[NL80211_STA_INFO_ASSOC_AT_BOOTTIME]));
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
get_reg(state, ifname)
struct nl80211_state *state;
const char *ifname;
{
    int ret = -1;
    struct nl_msg *msg = NULL;
    struct nl_cb *cb = NULL;
    struct cb_context ctx;
    unsigned int ifindex = 0;

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

    if (!genlmsg_put(msg, NL_AUTO_PORT, NL_AUTO_SEQ, state->nl80211_id, 0, NLM_F_DUMP, NL80211_CMD_GET_STATION, 0)) {
        fprintf(stderr, "genlmsg_put() failed. \n");
        ret = -ENOBUFS;

        goto finish;
    }

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
