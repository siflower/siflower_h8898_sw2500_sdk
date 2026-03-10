#!/bin/sh

# =============================================
# DEBUG SECTION - Detailed explanation of each step
# =============================================

# This script configures a cellular modem connection with the following steps:
# 1. Sets APN (Access Point Name) either from argument or defaults to CTNET
# 2. Configures modem with AT commands
# 3. Sets up network interfaces
# 4. Restarts networking services
# 5. Waits for interface to come up
# 6. Starts DHCP client

# --------------------------
# APN Configuration Section
# --------------------------

# Get APN from first argument, use CTNET as default if not provided
# Syntax: ${1:-CTNET} means "use $1 if exists, otherwise use CTNET"
# Debug tip: To verify APN is set correctly, add: echo "Using APN: $APN"

# --------------------------
# AT Command Section
# --------------------------

# AT+QICSGP=1,1,"APN" - Sets the APN for context 1
# AT+CGDCONT=1,"IP","APN" - Defines PDP context
# AT+QNETDEVCTL=1,1,1 - Activates network connection
# Debug tip: To verify AT commands, you can redirect to a log file first

# --------------------------
# Network Configuration Section
# --------------------------

# The following block adds network interface configuration to /etc/config/network:
# - wwan interface using pcie0 with DHCP
# - dummy0 interface with DHCP
# Debug tip: Check /etc/config/network after running to verify changes

# --------------------------
# Interface Monitoring Section
# --------------------------

# The script waits for:
# 1. pcie0 interface directory to appear in /sys/class/net/
# 2. pcie0 interface to reach "UP" state
# Debug tip: Add 'echo' statements to monitor wait progress

# --------------------------
# DHCP Client Section
# --------------------------

# Starts udhcpc on pcie0 interface to get IP address
# Debug tip: Check 'logread' after running to see DHCP process

# =============================================
# WORKING SCRIPT - Minimal version without debug
# =============================================

# Set APN (use first argument or default to CTNET)
usage() {
cat << EOF
build cryption case
Usage: ./dial_up_networking.sh <运营商>

 1 -> 中国电信 (CTNET)
 2 -> 中国移动 (CMNET)
 3 -> 中国联通 (CUWAP)

 Example:
 ./dial_up_networking.sh 1
EOF
}
add_config() {
    echo "Appending network config..."

    cat >> /etc/config/network <<EOF
config interface 'wwan'
    option proto 'dhcp'
    option device 'pcie0'

config interface 'dummy0'
    option proto 'dhcp'
    option device 'sipa_dummy0'
EOF
}
case "$1" in
    1)
        APN="CTNET"
        CNAME="中国电信"
        ;;
    2)
        APN="CMNET"
        CNAME="中国移动"
        ;;
    3)
        APN="CUWAP"
        CNAME="中国联通"
        ;;
    *)
        usage
        exit 1
        ;;
esac

echo "Selected APN: $APN"
echo "运营商: $CNAME"

# APN="${1:-CTNET}"

# Configure modem with AT commands
echo -e "AT+QICSGP=1,1,\"$APN\"\r" > /dev/stty_nr31
echo -e "AT+CGDCONT=1,\"IP\",\"$APN\"\r" > /dev/stty_nr31
echo -e "AT+QNETDEVCTL=1,1,1\r" > /dev/stty_nr31


# Check if 'wwan' and 'dummy0' already exist in config
WWAN_EXIST=$(grep -c "config interface 'wwan'" /etc/config/network)

if [ "$WWAN_EXIST" -eq 0 ]; then
    add_config
else
    echo " 5G module's interface already configured."
fi


# Restart network service
/etc/init.d/network restart

# Wait for interface to be available
while [ ! -d /sys/class/net/pcie0 ]; do
    sleep 1
done

# Wait for interface to come up
while ! ip link show pcie0 | grep -q "state UP"; do
    sleep 1
done

# Get IP address via DHCP
udhcpc -i pcie0

