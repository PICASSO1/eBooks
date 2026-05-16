/**
 * File Name: main.c
 * 
 * CopyLeft (C) 2026.
 *
 * Author: Pablo Picasso G.
 *
 * Version: 1.0.0.build051626
 *
 * Date: 2026 / 05 / 16
 *
 * Description: 
 *
 * 01. 這一支程序是用 `TRIGGER_GET_SCAN_v1` 修改而來，這種寫法才是比較偏向於商業化產品的寫法：
 *     ├ nl80211_subscribe_scan_group();    ----> 向 WLAN Driver 註冊一個 Multicast Group ID
 *     │ ├ genl_ctrl_resolve_grp();
 *     │ └ nl_socket_add_membership();
 *     ├ nl80211_trigger_scan();
 *     ├ nl80211_wait_scan_done();          ----> 等待 WLAN Driver 回覆 NL80211_CMD_NEW_SCAN_RESULTS
 *     └ nl80211_get_scan();                ----> WLAN Driver 回覆之後，就可以去讀取結果
 *
 *     特點：沒有 sleep、真正等 Scan Complete Event、可以知道 Aborted、Asynchronous (非同步)、更接近商用產品
 *     缺點：Callback Complexity 提高、Socket / Event Flow 複雜很多、Multicast / Unicast 要分清楚
 * 
 * 02. Multicast / Unicast ID 必須獨立分開；假如都用同一個 Socket ID，同時接收 Unicast Ack 和 Multicast Event 就會報錯：
 *     nl_recvmsgs() failed: Message sequence number mismatch (-16) 
 *     nl80211_trigger_scan() failed: -16
 *     所以在 struct nl80211_state 就要區分 cmd_sock (for unicast cmd)和 event_socket (for multicast event)
 * 
 * 03. 因為有註冊 Multicast ID，所以也必須新增一個 no_seq_check(); handler ，因為 Multicast 的 Sequence & Port ID 都會是０，User process 收到之後就會報錯 (-16)。
 * 
 * 04. 從封包的紀錄檔案中 (TRIGGER_GET_SCAN_v2.PCAP)可以觀察到：
 *     #04: 自己發送的 NL80211_CMD_TRIGGER_SCAN
 *     #05 ~ #09: WLAN Driver 發送的 Multicast Group NL80211_CMD_TRIGGER_SCAN (Sequence ID == 0 && Port ID == 0)
 *     #11: 自己發送的 NL80211_CMD_GET_SCAN
 *     #12: WLAN Driver 回覆的 NL80211_CMD_NEW_SCAN_RESULTS 
 *
(*)?*/

#include "main.h"
#include "trigger_scan.h"
#include "wait_scan.h"
#include "get_scan.h"

static const struct nl80211_commands_type nl80211_commands_type_map[] = {
    { NL80211_CMD_UNSPEC                    , "NL80211_CMD_UNSPEC" },
    { NL80211_CMD_GET_WIPHY                 , "NL80211_CMD_GET_WIPHY" },
    { NL80211_CMD_SET_WIPHY                 , "NL80211_CMD_SET_WIPHY" },
    { NL80211_CMD_NEW_WIPHY                 , "NL80211_CMD_NEW_WIPHY" },
    { NL80211_CMD_DEL_WIPHY                 , "NL80211_CMD_DEL_WIPHY" },
    { NL80211_CMD_GET_INTERFACE             , "NL80211_CMD_GET_INTERFACE" },
    { NL80211_CMD_SET_INTERFACE             , "NL80211_CMD_SET_INTERFACE" },
    { NL80211_CMD_NEW_INTERFACE             , "NL80211_CMD_NEW_INTERFACE" },
    { NL80211_CMD_DEL_INTERFACE             , "NL80211_CMD_DEL_INTERFACE" },
    { NL80211_CMD_GET_KEY                   , "NL80211_CMD_GET_KEY" }, 
    { NL80211_CMD_SET_KEY                   , "NL80211_CMD_SET_KEY" },
    { NL80211_CMD_NEW_KEY                   , "NL80211_CMD_NEW_KEY" },
    { NL80211_CMD_DEL_KEY                   , "NL80211_CMD_DEL_KEY" },
    { NL80211_CMD_GET_BEACON                , "NL80211_CMD_GET_BEACON" },
    { NL80211_CMD_SET_BEACON                , "NL80211_CMD_SET_BEACON" },
    { NL80211_CMD_START_AP                  , "NL80211_CMD_START_AP" },
    { NL80211_CMD_NEW_BEACON                , "NL80211_CMD_START_AP / NL80211_CMD_NEW_BEACON" },
    { NL80211_CMD_DEL_BEACON                , "NL80211_CMD_STOP_AP / NL80211_CMD_DEL_BEACON" },
    { NL80211_CMD_GET_STATION               , "NL80211_CMD_GET_STATION" },
    { NL80211_CMD_SET_STATION               , "NL80211_CMD_SET_STATION" },
    { NL80211_CMD_NEW_STATION               , "NL80211_CMD_NEW_STATION" },
    { NL80211_CMD_DEL_STATION               , "NL80211_CMD_DEL_STATION" },
    { NL80211_CMD_GET_MPATH                 , "NL80211_CMD_GET_MPATH" },
    { NL80211_CMD_SET_MPATH                 , "NL80211_CMD_SET_MPATH" },
    { NL80211_CMD_NEW_MPATH                 , "NL80211_CMD_NEW_MPATH" },
    { NL80211_CMD_DEL_MPATH                 , "NL80211_CMD_DEL_MPATH" },
    { NL80211_CMD_SET_BSS                   , "NL80211_CMD_SET_BSS" },
    { NL80211_CMD_SET_REG                   , "NL80211_CMD_SET_REG" }, 
    { NL80211_CMD_REQ_SET_REG               , "NL80211_CMD_REQ_SET_REG" },
    { NL80211_CMD_GET_MESH_CONFIG           , "NL80211_CMD_GET_MESH_CONFIG" },
    { NL80211_CMD_SET_MESH_CONFIG           , "NL80211_CMD_SET_MESH_CONFIG" },    
    { NL80211_CMD_SET_MGMT_EXTRA_IE         , "NL80211_CMD_SET_MGMT_EXTRA_IE" },
    { NL80211_CMD_GET_REG                   , "NL80211_CMD_GET_REG" },
    { NL80211_CMD_GET_SCAN                  , "NL80211_CMD_GET_SCAN" },
    { NL80211_CMD_TRIGGER_SCAN              , "NL80211_CMD_TRIGGER_SCAN" },
    { NL80211_CMD_NEW_SCAN_RESULTS          , "NL80211_CMD_NEW_SCAN_RESULTS" },
    { NL80211_CMD_SCAN_ABORTED              , "NL80211_CMD_SCAN_ABORTED" },
    { NL80211_CMD_REG_CHANGE                , "NL80211_CMD_REG_CHANGE" },
    { NL80211_CMD_AUTHENTICATE              , "NL80211_CMD_AUTHENTICATE" }, 
    { NL80211_CMD_ASSOCIATE                 , "NL80211_CMD_ASSOCIATE" },
    { NL80211_CMD_DEAUTHENTICATE            , "NL80211_CMD_DEAUTHENTICATE" },
    { NL80211_CMD_DISASSOCIATE              , "NL80211_CMD_DISASSOCIATE" },
    { NL80211_CMD_MICHAEL_MIC_FAILURE       , "NL80211_CMD_MICHAEL_MIC_FAILURE" },
    { NL80211_CMD_REG_BEACON_HINT           , "NL80211_CMD_REG_BEACON_HINT" },
    { NL80211_CMD_JOIN_IBSS                 , "NL80211_CMD_JOIN_IBSS" },
    { NL80211_CMD_LEAVE_IBSS                , "NL80211_CMD_LEAVE_IBSS" },
    { NL80211_CMD_TESTMODE                  , "NL80211_CMD_TESTMODE" },
    { NL80211_CMD_CONNECT                   , "NL80211_CMD_CONNECT" },
    { NL80211_CMD_ROAM                      , "NL80211_CMD_ROAM" },
    { NL80211_CMD_DISCONNECT                , "NL80211_CMD_DISCONNECT" },
    { NL80211_CMD_SET_WIPHY_NETNS           , "NL80211_CMD_SET_WIPHY_NETNS" },
    { NL80211_CMD_GET_SURVEY                , "NL80211_CMD_GET_SURVEY" },
    { NL80211_CMD_NEW_SURVEY_RESULTS        , "NL80211_CMD_NEW_SURVEY_RESULTS" },
    { NL80211_CMD_SET_PMKSA                 , "NL80211_CMD_SET_PMKSA" },
    { NL80211_CMD_DEL_PMKSA                 , "NL80211_CMD_DEL_PMKSA" },
    { NL80211_CMD_FLUSH_PMKSA               , "NL80211_CMD_FLUSH_PMKSA" },
    { NL80211_CMD_REMAIN_ON_CHANNEL         , "NL80211_CMD_REMAIN_ON_CHANNEL" },
    { NL80211_CMD_CANCEL_REMAIN_ON_CHANNEL  , "NL80211_CMD_CANCEL_REMAIN_ON_CHANNEL" },
    { NL80211_CMD_SET_TX_BITRATE_MASK       , "NL80211_CMD_SET_TX_BITRATE_MASK" },
    { NL80211_CMD_REGISTER_ACTION           , "NL80211_CMD_REGISTER_FRAME / NL80211_CMD_REGISTER_ACTION" },
    { NL80211_CMD_ACTION                    , "NL80211_CMD_FRAME / NL80211_CMD_ACTION" },
    { NL80211_CMD_ACTION_TX_STATUS          , "NL80211_CMD_FRAME_TX_STATUS / NL80211_CMD_ACTION_TX_STATUS" },
    { NL80211_CMD_SET_POWER_SAVE            , "NL80211_CMD_SET_POWER_SAVE" }, 
    { NL80211_CMD_GET_POWER_SAVE            , "NL80211_CMD_GET_POWER_SAVE" },
    { NL80211_CMD_SET_CQM                   , "NL80211_CMD_SET_CQM" },
    { NL80211_CMD_NOTIFY_CQM                , "NL80211_CMD_NOTIFY_CQM" },
    { NL80211_CMD_SET_CHANNEL               , "NL80211_CMD_SET_CHANNEL" },
    { NL80211_CMD_SET_WDS_PEER              , "NL80211_CMD_SET_WDS_PEER" },
    { NL80211_CMD_FRAME_WAIT_CANCEL         , "NL80211_CMD_FRAME_WAIT_CANCEL" },
    { NL80211_CMD_JOIN_MESH                 , "NL80211_CMD_JOIN_MESH" },
    { NL80211_CMD_LEAVE_MESH                , "NL80211_CMD_LEAVE_MESH" },
    { NL80211_CMD_UNPROT_DEAUTHENTICATE     , "NL80211_CMD_UNPROT_DEAUTHENTICATE" },
    { NL80211_CMD_UNPROT_DISASSOCIATE       , "NL80211_CMD_UNPROT_DISASSOCIATE" },
    { NL80211_CMD_NEW_PEER_CANDIDATE        , "NL80211_CMD_NEW_PEER_CANDIDATE" },
    { NL80211_CMD_GET_WOWLAN                , "NL80211_CMD_GET_WOWLAN" },
    { NL80211_CMD_SET_WOWLAN                , "NL80211_CMD_SET_WOWLAN" },
    { NL80211_CMD_START_SCHED_SCAN          , "NL80211_CMD_START_SCHED_SCAN" },
    { NL80211_CMD_STOP_SCHED_SCAN           , "NL80211_CMD_STOP_SCHED_SCAN" },
    { NL80211_CMD_SCHED_SCAN_RESULTS        , "NL80211_CMD_SCHED_SCAN_RESULTS" },
    { NL80211_CMD_SCHED_SCAN_STOPPED        , "NL80211_CMD_SCHED_SCAN_STOPPED" },
    { NL80211_CMD_SET_REKEY_OFFLOAD         , "NL80211_CMD_SET_REKEY_OFFLOAD" },
    { NL80211_CMD_PMKSA_CANDIDATE           , "NL80211_CMD_PMKSA_CANDIDATE" },
    { NL80211_CMD_TDLS_OPER                 , "NL80211_CMD_TDLS_OPER" },
    { NL80211_CMD_TDLS_MGMT                 , "NL80211_CMD_TDLS_MGMT" },
    { NL80211_CMD_UNEXPECTED_FRAME          , "NL80211_CMD_UNEXPECTED_FRAME" },
    { NL80211_CMD_PROBE_CLIENT              , "NL80211_CMD_PROBE_CLIENT" },
    { NL80211_CMD_REGISTER_BEACONS          , "NL80211_CMD_REGISTER_BEACONS" },
    { NL80211_CMD_UNEXPECTED_4ADDR_FRAME    , "NL80211_CMD_UNEXPECTED_4ADDR_FRAME" },
    { NL80211_CMD_SET_NOACK_MAP             , "NL80211_CMD_SET_NOACK_MAP" },
    { NL80211_CMD_CH_SWITCH_NOTIFY          , "NL80211_CMD_CH_SWITCH_NOTIFY" },
    { NL80211_CMD_START_P2P_DEVICE          , "NL80211_CMD_START_P2P_DEVICE" },
    { NL80211_CMD_STOP_P2P_DEVICE           , "NL80211_CMD_STOP_P2P_DEVICE" },
    { NL80211_CMD_CONN_FAILED               , "NL80211_CMD_CONN_FAILED" },
    { NL80211_CMD_SET_MCAST_RATE            , "NL80211_CMD_SET_MCAST_RATE" },
    { NL80211_CMD_SET_MAC_ACL               , "NL80211_CMD_SET_MAC_ACL" },
    { NL80211_CMD_RADAR_DETECT              , "NL80211_CMD_RADAR_DETECT" },
    { NL80211_CMD_GET_PROTOCOL_FEATURES     , "NL80211_CMD_GET_PROTOCOL_FEATURES" },
    { NL80211_CMD_UPDATE_FT_IES             , "NL80211_CMD_UPDATE_FT_IES" },
    { NL80211_CMD_FT_EVENT                  , "NL80211_CMD_FT_EVENT" },
    { NL80211_CMD_CRIT_PROTOCOL_START       , "NL80211_CMD_CRIT_PROTOCOL_START" },
    { NL80211_CMD_CRIT_PROTOCOL_STOP        , "NL80211_CMD_CRIT_PROTOCOL_STOP" },
    { NL80211_CMD_GET_COALESCE              , "NL80211_CMD_GET_COALESCE" },
    { NL80211_CMD_SET_COALESCE              , "NL80211_CMD_SET_COALESCE" },
    { NL80211_CMD_CHANNEL_SWITCH            , "NL80211_CMD_CHANNEL_SWITCH" },
    { NL80211_CMD_VENDOR                    , "NL80211_CMD_VENDOR" },
    { NL80211_CMD_SET_QOS_MAP               , "NL80211_CMD_SET_QOS_MAP" },
    { NL80211_CMD_ADD_TX_TS                 , "NL80211_CMD_ADD_TX_TS" },
    { NL80211_CMD_DEL_TX_TS                 , "NL80211_CMD_DEL_TX_TS" },
    { NL80211_CMD_GET_MPP                   , "NL80211_CMD_GET_MPP" },
    { NL80211_CMD_JOIN_OCB                  , "NL80211_CMD_JOIN_OCB" },
    { NL80211_CMD_LEAVE_OCB                 , "NL80211_CMD_LEAVE_OCB" },
    { NL80211_CMD_CH_SWITCH_STARTED_NOTIFY  , "NL80211_CMD_CH_SWITCH_STARTED_NOTIFY" },
    { NL80211_CMD_TDLS_CHANNEL_SWITCH       , "NL80211_CMD_TDLS_CHANNEL_SWITCH" },
    { NL80211_CMD_TDLS_CANCEL_CHANNEL_SWITCH, "NL80211_CMD_TDLS_CANCEL_CHANNEL_SWITCH" },
    { NL80211_CMD_WIPHY_REG_CHANGE          , "NL80211_CMD_WIPHY_REG_CHANGE" },
    { NL80211_CMD_ABORT_SCAN                , "NL80211_CMD_ABORT_SCAN" },
    { NL80211_CMD_START_NAN                 , "NL80211_CMD_START_NAN" },
    { NL80211_CMD_STOP_NAN                  , "NL80211_CMD_STOP_NAN" },
    { NL80211_CMD_ADD_NAN_FUNCTION          , "NL80211_CMD_ADD_NAN_FUNCTION" },
    { NL80211_CMD_DEL_NAN_FUNCTION          , "NL80211_CMD_DEL_NAN_FUNCTION" },
    { NL80211_CMD_CHANGE_NAN_CONFIG         , "NL80211_CMD_CHANGE_NAN_CONFIG" },
    { NL80211_CMD_NAN_MATCH                 , "NL80211_CMD_NAN_MATCH" },
    { NL80211_CMD_SET_MULTICAST_TO_UNICAST  , "NL80211_CMD_SET_MULTICAST_TO_UNICAST" },
    { NL80211_CMD_UPDATE_CONNECT_PARAMS     , "NL80211_CMD_UPDATE_CONNECT_PARAMS" },
    { NL80211_CMD_SET_PMK                   , "NL80211_CMD_SET_PMK" },
    { NL80211_CMD_DEL_PMK                   , "NL80211_CMD_DEL_PMK" },
    { NL80211_CMD_PORT_AUTHORIZED           , "NL80211_CMD_PORT_AUTHORIZED" },
    { NL80211_CMD_RELOAD_REGDB              , "NL80211_CMD_RELOAD_REGDB" },
    { NL80211_CMD_EXTERNAL_AUTH             , "NL80211_CMD_EXTERNAL_AUTH" },
    { NL80211_CMD_STA_OPMODE_CHANGED        , "NL80211_CMD_STA_OPMODE_CHANGED" },
    { NL80211_CMD_CONTROL_PORT_FRAME        , "NL80211_CMD_CONTROL_PORT_FRAME" },
    { NL80211_CMD_GET_FTM_RESPONDER_STATS   , "NL80211_CMD_GET_FTM_RESPONDER_STATS" },
    { NL80211_CMD_PEER_MEASUREMENT_START    , "NL80211_CMD_PEER_MEASUREMENT_START" },
    { NL80211_CMD_PEER_MEASUREMENT_RESULT   , "NL80211_CMD_PEER_MEASUREMENT_RESULT" },
    { NL80211_CMD_PEER_MEASUREMENT_COMPLETE , "NL80211_CMD_PEER_MEASUREMENT_COMPLETE" },
    { NL80211_CMD_NOTIFY_RADAR              , "NL80211_CMD_NOTIFY_RADAR" },
    { NL80211_CMD_UPDATE_OWE_INFO           , "NL80211_CMD_UPDATE_OWE_INFO" },
    { NL80211_CMD_PROBE_MESH_LINK           , "NL80211_CMD_PROBE_MESH_LINK" },
};

static const struct nl80211_bss_scan_width_type nl80211_bss_scan_width_type_map[] = {
    { NL80211_BSS_CHAN_WIDTH_20, "NL80211_BSS_CHAN_WIDTH_20" },
    { NL80211_BSS_CHAN_WIDTH_10, "NL80211_BSS_CHAN_WIDTH_10" },
    { NL80211_BSS_CHAN_WIDTH_5 , "NL80211_BSS_CHAN_WIDTH_5"  },
};

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
    enum SCAN_TYPE scan_type = IW_SCAN_TYPE_ACTIVE;

    if (argc != 2) {
        fprintf(stderr, "Please input a WLAN Interface name!! \n");

        exit(EXIT_FAILURE);
    }
    bzero(&state, sizeof(struct nl80211_state));

    ret = init_nl80211(&state);
    if (ret < 0) {
        fprintf(stderr, "init_nl80211() failed: %d \n", ret);
        
        goto finish;
    }

    ret = nl80211_subscribe_scan_group(&state);
    if (ret < 0) {
        fprintf(stderr, "nl80211_subscribe_scan_group() failed: %d\n", ret);

        goto finish;
    }

    ret = nl80211_trigger_scan(&state, argv[1], scan_type);
    if (ret < 0) {
        fprintf(stderr, "nl80211_trigger_scan() failed: %d \n", ret);
        
        goto finish;
    }

    ret = nl80211_wait_scan_done(&state);
    if (ret < 0) {
        fprintf(stderr, "nl80211_wait_scan_done() failed: %d\n", ret);

        goto finish;
    }

    ret = nl80211_get_scan(&state, argv[1]);
    if (ret < 0) {
        fprintf(stderr, "nl80211_get_scan() failed: %d \n", ret);
        
        goto finish;
    }

finish:
    deinit_nl80211(&state);

    return ret;
}

int 
init_nl80211(state)
struct nl80211_state *state;
{
    int ret = -1;

    state->cmd_sock = nl_socket_alloc();
    if (state->cmd_sock == NULL) {
        fprintf(stderr, "nl_socket_alloc() failed. \n");

        return -ENOMEM;
    }

    ret = genl_connect(state->cmd_sock);
    if (ret < 0) {
        fprintf(stderr, "genl_connect() failed: %s (%d). \n", nl_geterror(ret), ret);

        return ret;
    }

    state->nl80211_id = genl_ctrl_resolve(state->cmd_sock, "nl80211");
    if (state->nl80211_id < 0) {
        fprintf(stderr, "genl_ctrl_resolve(\"nl80211\") failed: %s (%d)\n", nl_geterror(state->nl80211_id), state->nl80211_id);

        return state->nl80211_id;
    }

    state->event_sock = nl_socket_alloc();
    if (state->event_sock == NULL) {
        fprintf(stderr, "nl_socket_alloc() failed. \n");

        return -ENOMEM;
    }

    ret = genl_connect(state->event_sock);
    if (ret < 0) {
        fprintf(stderr, "genl_connect() failed: %s (%d). \n", nl_geterror(ret), ret);

        return ret;
    }

    return 0;
}

void
deinit_nl80211(state)
struct nl80211_state *state;
{
    if (state != NULL) {
        if (state->cmd_sock != NULL) {
            nl_socket_free(state->cmd_sock);
            state->cmd_sock = NULL;
        }

        if (state->event_sock != NULL) {
            nl_socket_free(state->event_sock);
            state->event_sock = NULL;
        }
    }
	return;
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
        char c = data[i];

        if (c >= 32 && c <= 126)
            printf("%c", c);
        else
            printf("\\x%02x", c);
    }
    printf("\"");
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
    int i = 0, num = sizeof(nl80211_commands_type_map) / sizeof(struct nl80211_commands_type);

    for (i = 0; i < num; i++) {
        if (nl80211_commands_type_map[i].cmd == cmd)
            return nl80211_commands_type_map[i].desc;
    }
    return "UNKNOWN";
}

const char *
bss_width_to_string(cmd)
enum nl80211_bss_scan_width cmd;
{
    int i = 0, num = sizeof(nl80211_bss_scan_width_type_map) / sizeof(struct nl80211_bss_scan_width_type);

    for (i = 0; i < num; i++) {
        if (nl80211_bss_scan_width_type_map[i].cmd == cmd)
            return nl80211_bss_scan_width_type_map[i].desc;
    }
    return "UNKNOWN";
}
