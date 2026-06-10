#!/bin/sh

PCIE_DEV="/dev/stty_nr31"

send_at() {
    local CMD="$1"
    timeout 0.1 cat <&3 > /dev/null 2>/dev/null
    printf '%s\r' "$CMD" >&3
    timeout 0.5 cat <&3 2>/dev/null
}

common_resp() {
    local CMD="$1"
    local RESP
    RESP=$(send_at "$CMD")
    if echo "$RESP" | grep -q "OK"; then
        return 0
    else
        return 1
    fi
}

wait_for_modem_ready() {
    local dev="$1"
    local timeout_sec="${2:-30}"
    local i=0
    local resp

    echo "等待 $dev 就绪..."

    while [ "$i" -lt "$timeout_sec" ]; do
        if [ -e "$dev" ]; then
            if [ "$dev" == "$USB_DEV" ]; then
                stty -F "$dev" raw -echo
            fi
            exec 3<>"$dev" 2>/dev/null
            if [ $? -eq 0 ]; then
                common_resp 'AT' && return 0
                exec 3>&-
                exec 3<&-
            fi
        fi

        sleep 2
        i=$((i + 1))
    done

    return 1
}

get_pcie_mode() {
    local RESP
    RESP=$(send_at 'AT+QCFG="pcie/mode"')
    echo "$RESP" | awk -F',' '/\+QCFG: "pcie\/mode"/ {gsub(/\r/,"",$2); print $2}'
}

get_sim_state() {
    local RESP1 RESP2 simstat cpin
    RESP1=$(send_at 'AT+QSIMSTAT?')
    RESP2=$(send_at 'AT+CPIN?')
    simstat=$(echo "$RESP1" | sed -n 's/.*+QSIMSTAT: [0-9]\+,\([0-9]\+\).*/\1/p' | tr -d '[:space:]')
    cpin=$(echo "$RESP2" | sed -n 's/.*+CPIN: \([A-Z]\+\).*/\1/p' | tr -d '[:space:]')
    [ "$simstat" = "1" ] && [ "$cpin" = "READY" ]
}

get_init_stat() {
    local RESP initstat
    RESP=$(send_at 'AT+QINISTAT')
    initstat=$(echo "$RESP" | sed -n 's/.*+QINISTAT: \([0-9]\+\).*/\1/p' | tr -d '[:space:]')
    [ "$initstat" = "7" ]
}

wait_cgact_success() {
    local i RESP
    for i in $(seq 1 5); do
        common_resp 'AT+CGACT=1,1' && return 0
        sleep 3
    done
    return 1
}

add_config() {
    cat >> /etc/config/network <<EOF
config interface 'wwan'
    option proto 'dhcp'
    option device 'pcie0'

config interface 'dummy0'
    option proto 'dhcp'
    option device 'sipa_dummy0'
EOF
}

# 打开读写 fd
wait_for_modem_ready "$PCIE_DEV" || { echo "等待设备通信准备失败"; exit 1; }

get_sim_state || { echo "SIM卡检测失败"; exec 3>&-; exec 3<&-; exit 1; }
get_init_stat || { echo "模块初始化检测失败"; exec 3>&-; exec 3<&-; exit 1; }
common_resp 'AT+QCFG="autoapn",1' || { echo "启用自动 APN 选择失败"; exec 3>&-; exec 3<&-; exit 1; }
wait_cgact_success || { echo "激活 PDP 上下文失败,请检查天线是否连接"; exec 3>&-; exec 3<&-; exit 1; }
common_resp 'AT+QNETDEVCTL=1,1,1' || { echo "连接指定 PDP 建立失败"; exec 3>&-; exec 3<&-; exit 1; }

EXIST=$(grep -c "option device 'pcie0'" /etc/config/network)
if [ "$EXIST" -eq 0 ]; then
    add_config
    /etc/init.d/network restart
else
    ip link set pcie0 up
fi

# 关闭 fd
exec 3>&-
exec 3<&-
