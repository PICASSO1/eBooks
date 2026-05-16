#include "main.h"
#include "trigger_scan.h"

int
trigger_error_handler(nla, err, arg)
struct sockaddr_nl *nla;
struct nlmsgerr *err;
void *arg;
{
    (void)nla;
    struct trigger_cb_context *ctx = (struct trigger_cb_context *)arg;

    ctx->err = err->error;
    ctx->done = 1;

    return NL_STOP;
}

int
trigger_finish_handler(msg, arg)
struct nl_msg *msg;
void *arg;
{
    (void)msg;
    struct trigger_cb_context *ctx = (struct trigger_cb_context *)arg;

    ctx->done = 1;

    return NL_SKIP;
}

int
trigger_ack_handler(msg, arg)
struct nl_msg *msg;
void *arg;
{
    (void)msg;
    struct trigger_cb_context *ctx = arg;

    ctx->done = 1;

    return NL_STOP;
}

int 
nl80211_trigger_scan(state, ifname, scan_type)
struct nl80211_state *state;
const char *ifname;
const enum SCAN_TYPE scan_type;
{
    int ret = -1;
    struct nl_msg *msg = NULL;
    struct nl_cb *cb = NULL;
    struct trigger_cb_context ctx;
    unsigned int ifindex = 0;
    struct nlattr *ssids = NULL;

    bzero(&ctx, sizeof(struct trigger_cb_context));

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

    /* NL80211_CMD_TRIGGER_SCAN 並非 dump request；所以第６個參數設定０即可！ */
    if (!genlmsg_put(msg, NL_AUTO_PORT, NL_AUTO_SEQ, state->nl80211_id, 0, 0, NL80211_CMD_TRIGGER_SCAN, 0)) {
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

    /* NL80211_ATTR_SCAN_SSIDS 是一個 Nested Attribute (巢狀屬性)；
     * 在 Active Scanning 的情況之下，發送一個 Broadcast 的 Probe Request， 等待其他 AP 回覆 Probe Response。
     * 最後的 Raw Data: 08 00 2D 80 04 00 01 00
     *     Attribute: NL80211_ATTR_SCAN_SSIDS
     *         Len: 8                                                 (08 00)
     *         Type: 0x802d, Nested, NL80211_ATTR_SCAN_SSIDS (45)
     *             1... .... .... .... = Nested: True
     *             .0.. .... .... .... = Network byte order: False
     *             Attribute Type: NL80211_ATTR_SCAN_SSIDS (45)       (2D 80)
     *         Attribute Value: 0x00010004 (65540)                    (04 00 01 00)
     */
    if (scan_type == IW_SCAN_TYPE_ACTIVE) {
        ssids = nla_nest_start(msg, NL80211_ATTR_SCAN_SSIDS);
        if (ssids == NULL) {
            fprintf(stderr, "nla_nest_start(NL80211_ATTR_SCAN_SSIDS) failed! \n");
            ret = -ENOBUFS;

            goto finish;
        }

        ret = nla_put(msg, 1, 0, "");
        if (ret < 0) {
            fprintf(stderr, "nla_put() failed: %s (%d) \n", nl_geterror(ret), ret);

            goto finish;
        }

        ret = nla_nest_end(msg, ssids);
        if (ret != 0) {
            fprintf(stderr, "nla_nest_end() failed: %s (%d) \n", nl_geterror(ret), ret);

            goto finish;
        }
    }
    /* Passive Scannin 就是不發送任何 Probe Request 封包，而是靜靜地等待別人的 Beacon 封包。 */

/*  {
        struct nlattr *freqs = nla_nest_start(msg, NL80211_ATTR_SCAN_FREQUENCIES);

        if (freqs == NULL) {
            fprintf(stderr, "nla_nest_start() failed: %s (%d) \n", nl_geterror(ret), ret);

            goto finish;
        }

        ret = nla_put_u32(msg, 1, 2412);
        if (ret < 0) {
            fprintf(stderr, "nla_put_u32() failed: %s (%d) \n", nl_geterror(ret), ret);

            goto finish;
        }

        nla_nest_end(msg, freqs);
    }
*/

    /* NL80211_ATTR_MEASUREMENT_DURATION 指定在同一個 Channel 上必須 scan 多少 TU (Time Unit)？ 1 TU ≈ 1024 µs */
    ret = nla_put_u16 (msg, NL80211_ATTR_MEASUREMENT_DURATION, 100);
    if (ret < 0) {
        fprintf(stderr, "nla_put() failed: %s (%d) \n", nl_geterror(ret), ret);

        goto finish;
    }

    /* 強制 WLAN Driver 必須照這個時間掃描，不能夠自己調整！ */
    ret = nla_put_flag (msg, NL80211_ATTR_MEASUREMENT_DURATION_MANDATORY);
    if (ret < 0) {
        fprintf(stderr, "nla_put() failed: %s (%d) \n", nl_geterror(ret), ret);

        goto finish;
    }

    /* NL80211_ATTR_MEASUREMENT_DURATION & NL80211_ATTR_MEASUREMENT_DURATION_MANDATORY 與 Active & Passive 無關；且是非必要參數。
     * 主要用途是：精準測量 / 同步掃描 / 協議控制，e.g. IEEE 802.11k / RRM (Radio Resource Measurement)；但並不是所有的 WLAN Driver 都有支援。
     */

    ret = nl_cb_err(cb, NL_CB_CUSTOM, trigger_error_handler, &ctx);
    if (ret < 0) {
        fprintf(stderr, "nl_cb_err(trigger_error_handler) failed: %s (%d) \n", nl_geterror(ret), ret);

        goto finish;
    }

/*  因為 NL80211_CMD_TRIGGER_SCAN 只是送一個 Command ID 給 WLAN Driver 而已，不需要準備回傳的 Handler ；
 *  而且前面已經 nl_cb_alloc(NL_CB_DEFAULT); 所以也不需要設定 NL_CB_DEFAULT
 *  ret = nl_cb_set(cb, NL_CB_VALID,     NL_CB_DEFAULT, (nl_recvmsg_msg_cb_t)0, (void *)&ctx);
 *  if (ret < 0) {
 *      fprintf(stderr, "nl_cb_set(NL_CB_DEFAULT) failed: %s (%d) \n", nl_geterror(ret), ret);
 *
 *      goto finish;
 *  }
 */
    ret = nl_cb_set(cb, NL_CB_FINISH,    NL_CB_CUSTOM, trigger_finish_handler, (void *)&ctx);
    if (ret < 0) {
        fprintf(stderr, "nl_cb_set(trigger_finish_handler) failed: %s (%d) \n", nl_geterror(ret), ret);

        goto finish;
    }

    ret = nl_cb_set(cb, NL_CB_ACK,       NL_CB_CUSTOM,  trigger_ack_handler, (void *)&ctx);
    if (ret < 0) {
        fprintf(stderr, "nl_cb_set(trigger_ack_handler) failed: %s (%d) \n", nl_geterror(ret), ret);

        goto finish;
    }

    ret = nl_send_auto(state->cmd_sock, msg);
    if (ret < 0) {
        fprintf(stderr, "nl_cb_set(trigger_ack_handler) failed: %s (%d) \n", nl_geterror(ret), ret);

        goto finish;
    }

    /* 假如還沒有回傳到最後一筆資料，就持續地接收封包；而接收回來的封包會在 NL_CB_VALID 所註冊的 callback 函式處理 */
    while (ctx.done != 1) {
        ret = nl_recvmsgs(state->cmd_sock, cb);
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
