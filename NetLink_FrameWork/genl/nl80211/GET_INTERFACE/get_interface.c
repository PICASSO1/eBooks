#include "get_interface.h"

int
error_handler(nla, err, arg)
struct sockaddr_nl *nla;
struct nlmsgerr *err;
void *arg;
{
    (void)nla;
    struct cb_context *ctx = (struct cb_context *)arg;

    ctx->err = err->error;
    ctx->ack = 1;

    return NL_STOP;
}

int
finish_handler(msg, arg)
struct nl_msg *msg;
void *arg;
{
    (void)msg;
    struct cb_context *ctx = (struct cb_context *)arg;

    ctx->finish = 1;

    return NL_SKIP;
}

int
ack_handler(msg, arg)
struct nl_msg *msg;
void *arg;
{
    (void)msg;
    struct cb_context *ctx = arg;

    ctx->ack = 1;

    return NL_STOP;
}

const char *
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
cmd_to_string(cmd)
enum nl80211_commands cmd;
{
    size_t i = 0, num = sizeof(nl80211_commands_type_map) / sizeof(struct nl80211_commands_type);

    for (i = 0; i < num; i++) {
        if (nl80211_commands_type_map[i].cmd == cmd)
            return nl80211_commands_type_map[i].desc;
    }
}
