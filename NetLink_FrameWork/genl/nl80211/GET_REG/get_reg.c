#include "main.h"
#include "get_reg.h"

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
    static struct nla_policy reg_policy[NL80211_ATTR_MAX + 1] = {
        [NL80211_ATTR_REG_ALPHA2]  = { .type = NLA_STRING },
        [NL80211_ATTR_DFS_REGION]  = { .type = NLA_U8 },
        [NL80211_ATTR_REG_RULES]   = { .type = NLA_NESTED },
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

    ret = nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0), genlmsg_attrlen(gnlh, 0), reg_policy);
    if (ret < 0) {
        fprintf(stderr, "nla_parse() failed: %s (%d).\n", nl_geterror(ret), ret);

        return NL_SKIP;
    }

    if (tb[NL80211_ATTR_REG_ALPHA2]) {
        printf("NL80211_ATTR_REG_ALPHA2 (%d): %s\n", NL80211_ATTR_REG_ALPHA2, nla_get_string(tb[NL80211_ATTR_REG_ALPHA2]));
    }

    if (tb[NL80211_ATTR_DFS_REGION]) {
        printf("NL80211_ATTR_DFS_REGION (%d): %s (%u)\n", NL80211_ATTR_DFS_REGION, dfs_regions_to_string(nla_get_u8(tb[NL80211_ATTR_DFS_REGION])), nla_get_u8(tb[NL80211_ATTR_DFS_REGION]));
    }

    if (tb[NL80211_ATTR_REG_RULES]) {
        struct nlattr *rule = NULL;
        int rem_rule = -1;
        static struct nla_policy reg_rule_policy[NL80211_ATTR_MAX + 1] = {
            [NL80211_ATTR_REG_RULE_FLAGS]          = { .type = NLA_U32 },
            [NL80211_ATTR_FREQ_RANGE_START]        = { .type = NLA_U32 },
            [NL80211_ATTR_FREQ_RANGE_END]          = { .type = NLA_U32 },
            [NL80211_ATTR_FREQ_RANGE_MAX_BW]       = { .type = NLA_U32 },
            [NL80211_ATTR_POWER_RULE_MAX_ANT_GAIN] = { .type = NLA_U32 },
            [NL80211_ATTR_POWER_RULE_MAX_EIRP]     = { .type = NLA_U32 },
            [NL80211_ATTR_DFS_CAC_TIME]            = { .type = NLA_U32 },
        };

        printf("NL80211_ATTR_REG_RULES (%d): len = %d\n", NL80211_ATTR_REG_RULES, nla_len(tb[NL80211_ATTR_REG_RULES]));
        nla_for_each_nested(rule, tb[NL80211_ATTR_REG_RULES], rem_rule) {
            struct nlattr *rule_attrs[NL80211_ATTR_MAX + 1];
            int ret = -1;

            memset(rule_attrs, 0, sizeof(rule_attrs));
            ret = nla_parse_nested(rule_attrs, NL80211_ATTR_MAX, rule, reg_rule_policy);
            if (ret < 0) {
                printf("  Rule[%02d]: nla_parse_nested() failed: %s (%d)\n", nla_type(rule), nl_geterror(ret), ret);
                continue;
            }

            printf("  Rule[%02d]: len = %d\n", nla_type(rule), nla_len(rule));

            if (rule_attrs[NL80211_ATTR_REG_RULE_FLAGS]) {
                printf("    NL80211_ATTR_REG_RULE_FLAGS (%d): 0x%08x\n", NL80211_ATTR_REG_RULE_FLAGS, nla_get_u32(rule_attrs[NL80211_ATTR_REG_RULE_FLAGS]));
            }

            if (rule_attrs[NL80211_ATTR_FREQ_RANGE_START]) {
                printf("    NL80211_ATTR_FREQ_RANGE_START (%d): %u KHz / %.3f MHz\n", NL80211_ATTR_FREQ_RANGE_START, \
                    nla_get_u32(rule_attrs[NL80211_ATTR_FREQ_RANGE_START]), nla_get_u32(rule_attrs[NL80211_ATTR_FREQ_RANGE_START]) / 1000.0);
            }

            if (rule_attrs[NL80211_ATTR_FREQ_RANGE_END]) {
                printf("    NL80211_ATTR_FREQ_RANGE_END (%d): %u KHz / %.3f MHz\n", NL80211_ATTR_FREQ_RANGE_END, \
                    nla_get_u32(rule_attrs[NL80211_ATTR_FREQ_RANGE_END]), nla_get_u32(rule_attrs[NL80211_ATTR_FREQ_RANGE_END]) / 1000.0);
            }

            if (rule_attrs[NL80211_ATTR_FREQ_RANGE_MAX_BW]) {
                printf("    NL80211_ATTR_FREQ_RANGE_MAX_BW (%d): %u KHz / %.3f MHz\n", NL80211_ATTR_FREQ_RANGE_MAX_BW, \
                    nla_get_u32(rule_attrs[NL80211_ATTR_FREQ_RANGE_MAX_BW]), nla_get_u32(rule_attrs[NL80211_ATTR_FREQ_RANGE_MAX_BW]) / 1000.0);
            }

            if (rule_attrs[NL80211_ATTR_POWER_RULE_MAX_ANT_GAIN]) {
                printf("    NL80211_ATTR_POWER_RULE_MAX_ANT_GAIN (%d): %.2f dBi (%u mBi)\n", NL80211_ATTR_POWER_RULE_MAX_ANT_GAIN, \
                    nla_get_u32(rule_attrs[NL80211_ATTR_POWER_RULE_MAX_ANT_GAIN]) / 100.0, nla_get_u32(rule_attrs[NL80211_ATTR_POWER_RULE_MAX_ANT_GAIN]));
            }

            if (rule_attrs[NL80211_ATTR_POWER_RULE_MAX_EIRP]) {
                printf("    NL80211_ATTR_POWER_RULE_MAX_EIRP (%d): %.2f dBm (%u mBm)\n", NL80211_ATTR_POWER_RULE_MAX_EIRP, \
                    nla_get_u32(rule_attrs[NL80211_ATTR_POWER_RULE_MAX_EIRP]) / 100.0, nla_get_u32(rule_attrs[NL80211_ATTR_POWER_RULE_MAX_EIRP]));
            }

            if (rule_attrs[NL80211_ATTR_DFS_CAC_TIME]) {
                printf("    NL80211_ATTR_DFS_CAC_TIME (%d): %u ms\n", NL80211_ATTR_DFS_CAC_TIME, nla_get_u32(rule_attrs[NL80211_ATTR_DFS_CAC_TIME]));
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
get_reg(state)
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

    if (!genlmsg_put(msg, NL_AUTO_PORT, NL_AUTO_SEQ, state->nl80211_id, 0, NLM_F_DUMP, NL80211_CMD_GET_REG, 0)) {
        fprintf(stderr, "genlmsg_put() failed. \n");
        ret = -ENOBUFS;

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
