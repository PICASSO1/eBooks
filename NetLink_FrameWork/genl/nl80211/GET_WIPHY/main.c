/**
 * File Name: main.c
 * 
 * CopyLeft (C) 2026.
 *
 * Author: Pablo Picasso G.
 *
 * Version: 1.0.0.build052426
 *
 * Date: 2026 / 05 / 24
 *
 * Description: 
 *
 * 01. `NL80211_CMD_GET_WIPHY` 是用來擷取每乙顆實體 Radio (phy0, phy1, phy2, etc..)的相關資訊；而 `NL80211_CMD_GET_INTERFACE` 則是以 WLAN Interface (wlan0, wlan1, wlan2, etc..)的相關資訊。
 * 
 * 02. 在 get_wiphy.c 的 get_wiphy(); 當中有一段註解掉的程式碼，NL80211_CMD_GET_WIPHY 和 NL80211_CMD_GET_INTERFACE 一樣，都可以指令特定的介面 (Interface or Phy)；倘若不指定的話，就是本機上所有的介面。
 * 
 * 03. nl80211 Protocol 寫來寫去，重點就是在發送 `NL80211_CMD_GET_XXXX` 和它的 callback handler(); 函式，這一個範例已經所有的巢狀屬性都解譯開來了。
 *
(*)?*/

#include "main.h"
#include "get_wiphy.h"

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

    ret = get_wiphy(&state);
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
        case NL80211_IFTYPE_UNSPECIFIED: return "NL80211_IFTYPE_UNSPECIFIED";
        case NL80211_IFTYPE_ADHOC:       return "NL80211_IFTYPE_ADHOC";
        case NL80211_IFTYPE_STATION:     return "NL80211_IFTYPE_STATION";
        case NL80211_IFTYPE_AP:          return "NL80211_IFTYPE_AP";
        case NL80211_IFTYPE_AP_VLAN:     return "NL80211_IFTYPE_AP_VLAN";
        case NL80211_IFTYPE_WDS:         return "NL80211_IFTYPE_WDS";
        case NL80211_IFTYPE_MONITOR:     return "NL80211_IFTYPE_MONITOR";
        case NL80211_IFTYPE_MESH_POINT:  return "NL80211_IFTYPE_MESH_POINT";
        case NL80211_IFTYPE_P2P_CLIENT:  return "NL80211_IFTYPE_P2P_CLIENT";
        case NL80211_IFTYPE_P2P_GO:      return "NL80211_IFTYPE_P2P_GO";
        case NL80211_IFTYPE_P2P_DEVICE:  return "NL80211_IFTYPE_P2P_DEVICE";
        case NL80211_IFTYPE_OCB:         return "NL80211_IFTYPE_OCB";
        case NL80211_IFTYPE_NAN:         return "NL80211_IFTYPE_NAN";
        default:                         return "UNKNOWN";
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

void
print_hex(data, len)
const void *data;
int len;
{
    const unsigned char *p = data;
    int i = -1;

    for (i = 0; i < len; i++)
        printf("0x%02X", p[i]);

    return;
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
