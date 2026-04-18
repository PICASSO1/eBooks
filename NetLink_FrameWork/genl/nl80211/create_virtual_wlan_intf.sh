#!/bin/bash

MODPROBE="/usr/sbin/modprobe"
LSMOD="/usr/sbin/lsmod"
RADIOS=3

GREP="/usr/bin/grep"

IW="/usr/sbin/iw"
IWCONFIG="/usr/sbin/iwconfig"

ECHO="/usr/bin/echo"
SLEEP="/usr/bin/sleep"

SYSTEMCTL="/usr/bin/systemctl"

HOSTAPD="/usr/sbin/hostapd"
WPA_SUPPLICANT="/usr/sbin/wpa_supplicant"

IP="/usr/bin/ip"

${MODPROBE} nlmon && ${SLEEP} 1;
${IP} link add nlmon0 type nlmon && ${SLEEP} 1;
${IP} link set nlmon0 up && ${SLEEP} 1;

${MODPROBE} mac80211_hwsim radios=${RADIOS}
${SLEEP} 1;

${LSMOD} | ${GREP} "mac80211_hwsim"
${SLEEP} 1;

${IW} dev && ${ECHO} "" && ${SLEEP} 1; 
${IWCONFIG} && ${ECHO} "" && ${SLEEP} 1; 

${SYSTEMCTL} stop NetworkManager;
${SLEEP} 1;

${IW} reg set US && ${SLEEP} 1;

${WPA_SUPPLICANT} -i wlan0 -c /media/genl/nl80211/wpa_supplicant.conf & 

${HOSTAPD} /media/genl/nl80211/hostapd_wlan1.conf &
${HOSTAPD} /media/genl/nl80211/hostapd_wlan2.conf &
# ${HOSTAPD} /media/genl/nl80211/hostapd_wlan2.conf &

${IW} dev && ${ECHO} "" && ${SLEEP} 1; 
${IWCONFIG} && ${ECHO} "" && ${SLEEP} 1; 

exit 0
