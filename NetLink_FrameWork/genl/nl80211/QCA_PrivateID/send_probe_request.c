#include "main.h"
#include "send_probe_request.h"

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
send_probe_request(state, ifname)
struct nl80211_state *state;
const char *ifname;
{
    int ret = -1;
    struct nl_msg *msg = NULL;
    struct nl_cb *cb = NULL;
    struct cb_context ctx;
    struct nlattr *data = NULL;
    unsigned int ifindex = 0;
    int is_trigger = 1;

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

    /* 因為發送 Probe Request 封包只是單一事件，通知 QCA 的 WLAN Driver 而已，不需要解譯 callback 回傳資料；所以第６個參數填０即可。 */
    if (!genlmsg_put(msg, NL_AUTO_PORT, NL_AUTO_SEQ, state->nl80211_id, 0, 0, NL80211_CMD_VENDOR, 0)) {
        fprintf(stderr, "genlmsg_put() failed. \n");
        ret = -ENOBUFS;

        goto finish;
    }

    /* 發送 Probe Request 一定要指定一個 Station 介面；AP 不會發送 Probe Request，除非有改過。 */
    ifindex = if_nametoindex (ifname);
    if (ifindex == 0) {
        fprintf(stderr, "if_nametoindex() failed!! \n");
        ret = -ENODEV;

        goto finish;
    }

    ret = nla_put_u32(msg, NL80211_ATTR_IFINDEX, ifindex);
    if (ret < 0) {
        fprintf(stderr, "nla_put_u32(NL80211_ATTR_IFINDEX) failed: %s (%d) \n", nl_geterror(ret), ret);

        goto finish;
    }

    /* 這邊要告訴 nl80211 我們要設定的是一組 Vendor ID (NL80211_ATTR_VENDOR_ID)；且值為 QCA_NL80211_VENDOR_ID。 */
    ret = nla_put_u32(msg, NL80211_ATTR_VENDOR_ID,QCA_NL80211_VENDOR_ID);
    if (ret < 0) {
        fprintf(stderr, "nla_put_u32(NL80211_ATTR_VENDOR_ID) failed: %s (%d) \n", nl_geterror(ret), ret);

        goto finish;
    }

    /* 這裡則是告訴 nl80211 我們還要設定一個子屬性的 Command (NL80211_ATTR_VENDOR_SUBCMD)叫做 QCA_NL80211_VENDOR_SUBCMD_SET_WIFI_CONFIGURATION */
    ret = nla_put_u32(msg, NL80211_ATTR_VENDOR_SUBCMD, QCA_NL80211_VENDOR_SUBCMD_SET_WIFI_CONFIGURATION);
    if (ret < 0) {
        fprintf(stderr, "nla_put_u32(NL80211_ATTR_VENDOR_SUBCMD) failed: %s (%d) \n", nl_geterror(ret), ret);

        goto finish;
    }

    /* 開始準備巢狀資料 NL80211_ATTR_VENDOR_DATA，放置的是 QCA 專屬的私有屬性。 */
    data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
    if (data == NULL) {
        fprintf(stderr, "nla_nest_start(NL80211_ATTR_VENDOR_DATA) failed! \n");
        ret = -ENOBUFS;

        goto finish;
    }

    ret = nla_put_u32(msg, QCA_WLAN_VENDOR_ATTR_CONFIG_GENERIC_COMMAND, QCA_NL80211_VENDOR_SUBCMD_WIFI_PARAMS);
    if (ret < 0) {
        fprintf(stderr, "nla_put_u32(QCA_WLAN_VENDOR_ATTR_CONFIG_GENERIC_COMMAND) failed: %s (%d) \n", nl_geterror(ret), ret);

        goto finish;
    }

    ret = nla_put_u32(msg, QCA_WLAN_VENDOR_ATTR_CONFIG_GENERIC_VALUE, IEEE80211_PARAM_SEND_PROBE_REQ);
    if (ret < 0) {
        fprintf(stderr, "nla_put_u32(QCA_WLAN_VENDOR_ATTR_CONFIG_GENERIC_VALUE) failed: %s (%d) \n", nl_geterror(ret), ret);

        goto finish;
    }

    ret = nla_put(msg, QCA_WLAN_VENDOR_ATTR_CONFIG_GENERIC_DATA, sizeof(int), &is_trigger);
    if (ret < 0) {
        fprintf(stderr, "nla_put() failed: %s (%d) \n", nl_geterror(ret), ret);

        goto finish;
    }

    ret = nla_nest_end(msg, data);
    if (ret != 0) {
        fprintf(stderr, "nla_nest_end() failed: %s (%d) \n", nl_geterror(ret), ret);

        goto finish;
    }

    ret = nl_cb_err(cb, NL_CB_CUSTOM, error_handler, &ctx);
    if (ret < 0) {
        fprintf(stderr, "nl_cb_err(error_handler) failed: %s (%d) \n", nl_geterror(ret), ret);

        goto finish;
    }

    /* 因為發送 Probe Request 封包只是單一事件，所以也不需要準備 valid_handler(); 函式！ */

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
