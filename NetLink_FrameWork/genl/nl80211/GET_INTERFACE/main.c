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

#include "main.h"
#include "get_interface.h"

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
        fprintf(stderr, "init_nl80211() failed: %d \n", ret);
        
        goto finish;
    }

    ret = dump_interfaces(&state);
    if (ret < 0) {
        fprintf(stderr, "nl80211_trigger_scan() failed: %d \n", ret);
        
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

        return ret;
    }

    /* genl_ctrl_resolve(); 詢問 Generic Netlink： "nl80211" 這個子集合的 Family ID 是多少？ */
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

char *
iftype_to_string(iftype)
enum nl80211_iftype iftype;
{
    switch (iftype) {
        case NL80211_IFTYPE_UNSPECIFIED: return "unspecified";
        case NL80211_IFTYPE_ADHOC:       return "adhoc";
        case NL80211_IFTYPE_STATION:     return "station";
        case NL80211_IFTYPE_AP:          return "ap";
        case NL80211_IFTYPE_AP_VLAN:     return "ap_vlan";
        case NL80211_IFTYPE_WDS:         return "wds";
        case NL80211_IFTYPE_MONITOR:     return "monitor";
        case NL80211_IFTYPE_MESH_POINT:  return "mesh";
        case NL80211_IFTYPE_P2P_CLIENT:  return "p2p_client";
        case NL80211_IFTYPE_P2P_GO:      return "p2p_go";
        case NL80211_IFTYPE_P2P_DEVICE:  return "p2p_device";
        case NL80211_IFTYPE_OCB:         return "ocb";
        case NL80211_IFTYPE_NAN:         return "nan";
        default:                         return "unknown";
    }
}

void
print_mac(mac, len)
const unsigned char *mac;
int len;
{
    if (!mac || len < 6) {
        printf("(invalid)");
        return;
    }

    printf("%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

void
print_ssid(data, len)
const unsigned char *data;
int len;
{
    int i = -1;

    printf("\"");
    for (i = 0; i < len; i++) {
        unsigned char c = data[i];

        if (c >= 32 && c <= 126)
            printf("%c", c);
        else
            printf("\\x%02x", c);
    }
    printf("\"");
}

const char *
channel_type_to_string(type)
unsigned int type;
{
    switch (type) {
        case NL80211_CHAN_NO_HT:     return "NL80211_CHAN_NO_HT";
        case NL80211_CHAN_HT20:      return "NL80211_CHAN_HT20";
        case NL80211_CHAN_HT40MINUS: return "NL80211_CHAN_HT40MINUS";
        case NL80211_CHAN_HT40PLUS:  return "NL80211_CHAN_HT40PLUS";
        default:                     return "UNKNOWN";
    }
}

const char *
channel_width_to_string(width)
unsigned int width;
{
    switch (width) {
        case NL80211_CHAN_WIDTH_20_NOHT: return "NL80211_CHAN_WIDTH_20_NOHT";
        case NL80211_CHAN_WIDTH_20:      return "NL80211_CHAN_WIDTH_20";
        case NL80211_CHAN_WIDTH_40:      return "NL80211_CHAN_WIDTH_40";
        case NL80211_CHAN_WIDTH_80:      return "NL80211_CHAN_WIDTH_80";
        case NL80211_CHAN_WIDTH_80P80:   return "NL80211_CHAN_WIDTH_80P80";
        case NL80211_CHAN_WIDTH_160:     return "NL80211_CHAN_WIDTH_160";
        case NL80211_CHAN_WIDTH_5:       return "NL80211_CHAN_WIDTH_5";
        case NL80211_CHAN_WIDTH_10:      return "NL80211_CHAN_WIDTH_10";
        default:                         return "UNKNOWN";
    }
}

const char *
cmd_to_string(cmd)
enum nl80211_commands cmd;
{
    size_t i = 0, num = sizeof(nl80211_commands_type_map) / sizeof(struct nl80211_commands_type);

    for (i = 0; i < num; i++) {
        if (nl80211_commands_type_map[i].cmd == cmd)
            return nl80211_commands_type_map[i].desc;
    }
    return "UNKNOWN";
}
