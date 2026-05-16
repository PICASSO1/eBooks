#include "main.h"
#include "wait_scan.h"

int
nl80211_subscribe_scan_group(state)
struct nl80211_state *state;
{
    int mcid = 0;
    int ret = 0;

    mcid = genl_ctrl_resolve_grp(state->event_sock, "nl80211", "scan");
    if (mcid < 0) {
        fprintf(stderr, "genl_ctrl_resolve_grp(scan) failed: %s (%d)\n", nl_geterror(mcid), mcid);
        ret = mcid;

        goto finish;
    }

    ret = nl_socket_add_membership(state->event_sock, mcid);
    if (ret < 0) {
        fprintf(stderr, "nl_socket_add_membership(scan) failed: %s (%d)\n", nl_geterror(ret), ret);

        return ret;
    }

finish:
    return ret;
}

int
scan_event_handler(msg, arg)
struct nl_msg *msg;
void *arg;
{
    struct scan_event_ctx *ctx = (struct scan_event_ctx *)arg;
    struct nlmsghdr *nlh = NULL;
    struct genlmsghdr *gnlh = NULL;
    int ret;

    nlh = nlmsg_hdr(msg);
    if (nlh == NULL) {
        return NL_SKIP;
    }

    gnlh = genlmsg_hdr(nlh);
    if (gnlh == NULL) {
        return NL_SKIP;
    }

    switch (gnlh->cmd) {
        case NL80211_CMD_NEW_SCAN_RESULTS:
            printf("%s: %s \n", __FUNCTION__, cmd_to_string(gnlh->cmd));
            ctx->done = 1;
            ret = NL_STOP;
            break;
        case NL80211_CMD_SCAN_ABORTED:
            printf("%s: %s \n", __FUNCTION__, cmd_to_string(gnlh->cmd));
            ctx->aborted = 1;
            ctx->done = 1;
            ret = NL_STOP;
            break;
        default:
            printf("%s: Ignore CMD: %s \n", __FUNCTION__, cmd_to_string(gnlh->cmd));
            ret = NL_SKIP;
            break;
    }

    return ret;
}

int
no_seq_check(msg, arg)
struct nl_msg *msg; 
void *arg;
{
    (void)msg;
    (void)arg;

    return NL_OK;
}

int
nl80211_wait_scan_done(state)
struct nl80211_state *state;
{
    int ret = -1;
    struct nl_cb *cb = NULL;
    struct scan_event_ctx ctx;

    memset(&ctx, 0, sizeof(struct scan_event_ctx));

    cb = nl_cb_alloc(NL_CB_DEFAULT);
    if (cb == NULL) {
        ret = -ENOMEM;

        goto finish;
    }

    /* 這裡要註冊一個 scan_event_handler(); 假如有收到 WLAN Driver 的 Multicast 封包就能夠即時處理。 */
    ret = nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, scan_event_handler, &ctx);
    if (ret < 0) {
        fprintf(stderr, "nl_cb_set(scan_event_handler) failed: %s (%d) \n", nl_geterror(ret), ret);

        goto finish;
    }

    /* 除此之外，也要註冊一個 no_seq_check(); handler ，因為 Multicast Group ID 會是０，所以要忽略檢查 (NL_OK)，要不然會報錯！ */
    ret = nl_cb_set(cb, NL_CB_SEQ_CHECK, NL_CB_CUSTOM, no_seq_check, &ctx);
    if (ret < 0) {
        fprintf(stderr, "nl_cb_set(scan_event_handler) failed: %s (%d) \n", nl_geterror(ret), ret);

        goto finish;
    }

    while (ctx.done == 0) {
        ret = nl_recvmsgs(state->event_sock, cb);
        if (ret < 0) {
            fprintf(stderr, "nl_recvmsgs(wait scan) failed: %s (%d)\n", nl_geterror(ret), ret);

            goto finish;
        }
    }

    if (ctx.aborted == 1) {
        ret = -ECANCELED;
    }

finish:
    if (cb != NULL) {
        nl_cb_put(cb);
        cb = NULL;
    }

    return ret;
}
