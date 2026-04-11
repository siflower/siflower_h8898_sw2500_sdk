#!/bin/sh

init_interface(){
        rm /etc/config/wireless
        touch /etc/config/wireless
		uci commit wireless
        hostssid="wifi-test-$1"

        uci -q batch <<-EOF
                set wireless.radio0=wifi-device
                set wireless.radio0.type='mac80211'
                set wireless.radio0.country='CN'
                set wireless.radio0.txpower_lvl='2'
                set wireless.radio0.channel='1'
                set wireless.radio0.band='2.4G'
                set wireless.radio0.hwmode='11g'
                set wireless.radio0.noscan='0'
                set wireless.radio0.netisolate='0'
                set wireless.radio0.max_all_num_sta='64'
                set wireless.radio0.path='platform/11000000.wifi-lb'
                set wireless.radio0.htmode='HT20'
                set wireless.radio0.disabled='0'
EOF
        uci -q batch <<-EOF
                set wireless.radio1=wifi-device
                set wireless.radio1.type='mac80211'
                set wireless.radio1.country='CN'
                set wireless.radio1.txpower_lvl='2'
                set wireless.radio1.channel='161'
                set wireless.radio1.band='5G'
                set wireless.radio1.hwmode='11a'
                set wireless.radio1.noscan='0'
                set wireless.radio1.netisolate='0'
                set wireless.radio1.max_all_num_sta='64'
                set wireless.radio1.path='platform/17800000.wifi-hb'
                set wireless.radio1.htmode='VHT80'
                set wireless.radio1.disabled='0'
EOF
        for i in $(seq 0 7)
        do
                uci add wireless wifi-iface > /dev/null                 #......ap......
                r=$(($i%2))                                             #......radio num......
				[ i -lt 6 ] && {
                uci -q batch <<-EOF
                        set wireless.@wifi-iface[$i].ifname=wlan${r}-ap${i}
                        set wireless.@wifi-iface[$i].device=radio${r}
                        set wireless.@wifi-iface[$i].network=lan
                        set wireless.@wifi-iface[$i].mode=ap
                        set wireless.@wifi-iface[$i].ssid="${hostssid}${i}"
                        set wireless.@wifi-iface[$i].encryption="psk2+ccmp"
                        set wireless.@wifi-iface[$i].key=12345678
EOF
				} || {
					local_network="wwan"
						if [ $r -eq 1 ]; then
							local_network="wwwan"
						fi
						uci -q batch <<-EOF
                        set wireless.@wifi-iface[$i].ifname=sfi${r}
                        set wireless.@wifi-iface[$i].device=radio${r}
                        set wireless.@wifi-iface[$i].network=$local_network
                        set wireless.@wifi-iface[$i].mode=sta
                        set wireless.@wifi-iface[$i].ssid="${hostssid}${i}"
                        set wireless.@wifi-iface[$i].bssid="10:16:88:12:33:44"
                        set wireless.@wifi-iface[$i].encryption="none"
						set wireless.@wifi-iface[$i].key=12345678
						EOF
				}
        done
        uci -q commit wireless
}


