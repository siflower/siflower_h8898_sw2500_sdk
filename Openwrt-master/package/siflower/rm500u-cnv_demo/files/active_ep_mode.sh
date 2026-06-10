#!/bin/sh

USB_DEV="/dev/ttyUSB2"

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

# 打开读写 fd
wait_for_modem_ready "$USB_DEV" || { echo "等待设备通信准备失败"; exit 1; }

mode=$(get_pcie_mode)
case "$mode" in
    0)
        echo "当前处于pcie的ep模式"
        ;;
    1)
        echo "当前处于pcie的rc模式,将切换为ep模式"
        if common_resp 'AT+QCFG="pcie/mode",0';then
            echo "切换pcie的ep模式成功，重置设备"
            if common_resp 'AT+CFUN=1,1';then
                echo "重置设备成功"
            else
                echo "重置设备失败"
            fi
        else
            echo "切换pcie的ep模式失败"
        fi
        ;;
    *)
        echo "获取模式失败: $mode"
        exec 3>&-
        exec 3<&-
        exit 1;
        ;;
esac

# 关闭 fd
exec 3>&-
exec 3<&-
