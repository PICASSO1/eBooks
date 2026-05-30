#include "main.h"
#include "get_survey.h"

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
    struct nlattr *survey[NL80211_SURVEY_INFO_MAX + 1] = { 0 };
    static struct nla_policy survey_policy[NL80211_SURVEY_INFO_MAX + 1] = {
        [NL80211_SURVEY_INFO_FREQUENCY]     = { .type = NLA_U32 },
        [NL80211_SURVEY_INFO_NOISE]         = { .type = NLA_U8  },
        [NL80211_SURVEY_INFO_IN_USE]        = { .type = NLA_FLAG },
        [NL80211_SURVEY_INFO_TIME]          = { .type = NLA_U64 },
        [NL80211_SURVEY_INFO_TIME_BUSY]     = { .type = NLA_U64 },
        [NL80211_SURVEY_INFO_TIME_EXT_BUSY] = { .type = NLA_U64 },
        [NL80211_SURVEY_INFO_TIME_RX]       = { .type = NLA_U64 },
        [NL80211_SURVEY_INFO_TIME_TX]       = { .type = NLA_U64 },
        [NL80211_SURVEY_INFO_TIME_SCAN]     = { .type = NLA_U64 },
        [NL80211_SURVEY_INFO_PAD]           = { .type = NLA_UNSPEC },
#ifdef NL80211_SURVEY_INFO_TIME_BSS_RX
        [NL80211_SURVEY_INFO_TIME_BSS_RX]   = { .type = NLA_U64 },
#endif
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

    memset(survey, 0, sizeof(survey));
    ret = nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0), genlmsg_attrlen(gnlh, 0), NULL);
    if (ret < 0) {
        printf("nla_parse_nested(SURVEY) failed: %s (%d)\n", nl_geterror(ret), ret);

        return NL_SKIP;
    }

    printf("NL80211_ATTR_SURVEY_INFO (%d): Len = %d\n", NL80211_ATTR_SURVEY_INFO, nla_len(tb[NL80211_ATTR_SURVEY_INFO]));
    if (!tb[NL80211_ATTR_SURVEY_INFO]) {
        printf("NL80211_ATTR_SURVEY_INFO not found\n");

        return NL_SKIP;
    }

    ret = nla_parse_nested(survey, NL80211_SURVEY_INFO_MAX, tb[NL80211_ATTR_SURVEY_INFO], survey_policy);
    if (ret < 0) {
        printf("nla_parse_nested(SURVEY_INFO) failed: %s (%d)\n", nl_geterror(ret), ret);

        return NL_SKIP;
    }

    if (survey[NL80211_SURVEY_INFO_FREQUENCY]) {
        printf("  NL80211_SURVEY_INFO_FREQUENCY (%d): %u MHz (0x%016llX)\n", NL80211_SURVEY_INFO_FREQUENCY, \
            nla_get_u32(survey[NL80211_SURVEY_INFO_FREQUENCY]), nla_raw_u64(survey[NL80211_SURVEY_INFO_FREQUENCY]));
    }

    if (survey[NL80211_SURVEY_INFO_NOISE]) {
        printf("  NL80211_SURVEY_INFO_NOISE (%d): %d dBm  (0x%016llX)\n", NL80211_SURVEY_INFO_NOISE, \
            (int8_t)nla_get_u8(survey[NL80211_SURVEY_INFO_NOISE]), nla_raw_u64(survey[NL80211_SURVEY_INFO_NOISE]));
    }

    if (survey[NL80211_SURVEY_INFO_IN_USE]) {
        printf("  NL80211_SURVEY_INFO_IN_USE (%d): YES\n", NL80211_SURVEY_INFO_IN_USE);
    }

    if (survey[NL80211_SURVEY_INFO_TIME]) {
        printf("  NL80211_SURVEY_INFO_TIME (%d): %llu us (0x%016llX)\n", NL80211_SURVEY_INFO_TIME, \
            (unsigned long long)nla_get_u64(survey[NL80211_SURVEY_INFO_TIME]), nla_raw_u64(survey[NL80211_SURVEY_INFO_TIME]));
    }

    if (survey[NL80211_SURVEY_INFO_TIME_BUSY]) {
        printf("  NL80211_SURVEY_INFO_TIME_BUSY (%d): %llu us (0x%016llX)\n", NL80211_SURVEY_INFO_TIME_BUSY, \
            (unsigned long long)nla_get_u64(survey[NL80211_SURVEY_INFO_TIME_BUSY]), nla_raw_u64(survey[NL80211_SURVEY_INFO_TIME_BUSY]));
    }

    if (survey[NL80211_SURVEY_INFO_TIME_EXT_BUSY]) {
        printf("  NL80211_SURVEY_INFO_TIME_EXT_BUSY (%d): %llu us (0x%016llX)\n", NL80211_SURVEY_INFO_TIME_EXT_BUSY, \
            (unsigned long long)nla_get_u64(survey[NL80211_SURVEY_INFO_TIME_EXT_BUSY]), nla_raw_u64(survey[NL80211_SURVEY_INFO_TIME_EXT_BUSY]));
    }

    if (survey[NL80211_SURVEY_INFO_TIME_RX]) {
        printf("  NL80211_SURVEY_INFO_TIME_RX (%d): %llu us (0x%016llX)\n", NL80211_SURVEY_INFO_TIME_RX, \
            (unsigned long long)nla_get_u64(survey[NL80211_SURVEY_INFO_TIME_RX]), nla_raw_u64(survey[NL80211_SURVEY_INFO_TIME_RX]));
    }

    if (survey[NL80211_SURVEY_INFO_TIME_TX]) {
        printf("  NL80211_SURVEY_INFO_TIME_TX (%d): %llu us (0x%016llX)\n", NL80211_SURVEY_INFO_TIME_TX, \
            (unsigned long long)nla_get_u64(survey[NL80211_SURVEY_INFO_TIME_TX]), nla_raw_u64(survey[NL80211_SURVEY_INFO_TIME_TX]));
    }

    if (survey[NL80211_SURVEY_INFO_TIME_SCAN]) {
        printf("  NL80211_SURVEY_INFO_TIME_SCAN (%d): %llu us (0x%016llX)\n", NL80211_SURVEY_INFO_TIME_SCAN, \
            (unsigned long long)nla_get_u64(survey[NL80211_SURVEY_INFO_TIME_SCAN]), nla_raw_u64(survey[NL80211_SURVEY_INFO_TIME_SCAN]));
    }

    if (survey[NL80211_SURVEY_INFO_PAD]) {
        printf("  NL80211_SURVEY_INFO_PAD (%d): %llu us (0x%016llX)\n", NL80211_SURVEY_INFO_PAD, \
            (unsigned long long)nla_get_u64(survey[NL80211_SURVEY_INFO_PAD]), nla_raw_u64(survey[NL80211_SURVEY_INFO_PAD]));
    }

#ifdef NL80211_SURVEY_INFO_TIME_BSS_RX
    if (survey[NL80211_SURVEY_INFO_TIME_BSS_RX]) {
        printf("  NL80211_SURVEY_INFO_TIME_BSS_RX (%d): %llu us (0x%016llX)\n", NL80211_SURVEY_INFO_TIME_BSS_RX, \
            (unsigned long long)nla_get_u64(survey[NL80211_SURVEY_INFO_TIME_BSS_RX]), nla_raw_u64(survey[NL80211_SURVEY_INFO_TIME_BSS_RX]));
    }
#endif

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
get_survey(state, ifname)
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

    if (!genlmsg_put(msg, NL_AUTO_PORT, NL_AUTO_SEQ, state->nl80211_id, 0, NLM_F_DUMP, NL80211_CMD_GET_SURVEY, 0)) {
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
