/**
 * File Name: main.c
 * 
 * CopyLeft (C) 2026.
 *
 * Author: Pablo Picasso G.
 *
 * Version: 1.0.0.build060626
 *
 * Date: 2026 / 06 / 06
 *
 * Description: 
 *
 * 01. 這是一個使用 QCA (QSDK) 的 solution 用來發送 Probe Request 封包的範例，僅適用於 Qualcomm 的 chipset；其他 Vendor (Intel, Broadcomm, MediaTek, RealTek, etc..)當然不行！
 * 
 * 02. 當然也不會是我寫的，感謝我的好同事 Jason 分享！
 * 
 * 03. 因為這是 QCA 專屬的 Private Vendor command and data，WireShark 無法解析也很正常。
 * 
 *
(*)?*/

#include "main.h"
#include "send_probe_request.h"

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

    if (argc != 2) {
        fprintf(stderr, "Please input a WLAN Interface name!! \n");

        exit(EXIT_FAILURE);
    }
    bzero(&state, sizeof(struct nl80211_state));

    if (init_nl80211(&state) < 0) {
        fprintf(stderr, "init_nl80211() failed: %d \n", ret);
        
        goto finish;
    }

    ret = send_probe_request(&state, argv[1]);
    if (ret < 0) {
        fprintf(stderr, "send_probe_request() failed: %d \n", ret);
        
        goto finish;
    }

finish:
    deinit_nl80211(&state);

    return 0;
}

int 
init_nl80211(state)
struct nl80211_state *state;
{
    int ret = -1;

    state->sock = nl_socket_alloc();
    if (state->sock == (struct nl_sock *)NULL) {
        fprintf(stderr, "nl_socket_alloc() failed. \n");

        return -ENOMEM;
    }

    ret = genl_connect(state->sock);
    if (ret < 0) {
        fprintf(stderr, "genl_connect() failed: %s (%d). \n", nl_geterror(ret), ret);

        return ret;
    }

    state->nl80211_id = genl_ctrl_resolve(state->sock, "nl80211");
    if (state->nl80211_id < 0) {
        fprintf(stderr, "genl_ctrl_resolve(\"nl80211\") failed: %s (%d)\n", nl_geterror(state->nl80211_id), state->nl80211_id);

        return state->nl80211_id;
    }

    return 0;
}

void
deinit_nl80211(state)
struct nl80211_state *state;
{
    if (state && state->sock) {
        nl_socket_free(state->sock);
        state->sock = NULL;
    }
	return;
}
