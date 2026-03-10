#!/bin/sh
# Configure modem with AT commands
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

echo -e "AT+QCFG=\"autoapn\",1\r" > /dev/stty_nr31
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

# Debug version of dia_up_networking.sh to configure Quectel RM500U-CN 5G module
# Automates AT commands, network configuration, and connectivity test on OpenWrt
# Includes debug steps from Quectel AT Commands Manual for detailed troubleshooting
# Date: July 28, 2025

# Debug Steps (based on provided debug content, excluding irrelevant outputs like +CSQ: 30,99):
# Step 1: Access the AT Command Port
#   - Use /dev/stty_nr31 for AT command communication (replaced with 'chat' for automation)
# Step 2: Verify Module and SIM Status
#   2.1 Check Module Status (pages 16-18):
#       - AT: Test basic communication with the module
#       - AT+GMI: Query manufacturer information
#       - AT+GMR: Query firmware revision
#   2.2 Check SIM Card Status (pages 60–76):
#       - AT+QSIMSTAT?: Query SIM insertion status
#       - AT+CPIN?: Verify SIM is ready and unlocked
#   2.3 Query Initialization Status (page 72):
#       - AT+QINISTAT: Check module initialization (7 indicates ready)
# Step 3: Configure Network Settings
#   3.1 Set Network Mode (page 100):
#       - AT+QNWPREFCFG="mode_pref": Query current network mode
#       - AT+QNWPREFCFG="mode_pref","NR5G:LTE": Set to 5G (NSA/SA) and LTE
#       - Supported modes:
#         | Command                              | Meaning                                   |
#         | ------------------------------------ | ----------------------------------------- |
#         | AT+QNWPREFCFG="mode_pref",AUTO      | Automatically select 5G + LTE + WCDMA     |
#         | AT+QNWPREFCFG="mode_pref",LTE       | Lock module to LTE only                   |
#         | AT+QNWPREFCFG="mode_pref",NR5G-SA   | Use 5G Standalone only                    |
#         | AT+QNWPREFCFG="mode_pref",NR5G-NSA  | Use 5G Non-Standalone only                |
#         | AT+QNWPREFCFG="mode_pref",NR5G:LTE  | Use both 5G (NSA/SA) and LTE              |
#         | AT+QNWPREFCFG="mode_pref",WCDMA     | Use only 3G (UMTS/WCDMA)                  |
#   3.2 Set APN (page 206):(if done, don't need do 3.3 Enable Automatic APN Selection)
#       - AT+QICSGP=1,1,"CTNET": Configure APN for PDP context ID 1 (China Telecom)
#   3.3 Enable Automatic APN Selection (page 52):
#       - AT+QCFG="autoapn",1: Enable automatic APN configuration(if done, don't need do 3.2 Set APN)
#   3.4 Check Network Registration (page 82):
#       - AT+CREG?: Check circuit-switched registration status
#   3.5 Query Network Information (page 90):
#       - AT+QNWINFO: Query current network type and parameters
# Step 4: Configure PCIe/USB Mode
#   4.1 Check the current USB mode:
#       - AT+QCFG="usbmode": Query USB mode (e.g., MBIM, ECM)
#   4.2 If not in MBIM mode, set it:
#       - AT+QCFG="usbmode","mbim": Set to MBIM mode for PCIe data
#   4.3 Save settings:
#       - AT&W: Save configuration to non-volatile memory
# Step 5: Activate Data Connection
#   5.1 Define PDP Context (page 176):(if do 3.3 Enable Automatic APN Selection, don't do 5.1 Define PDP Context)
#       - AT+CGDCONT=1,"IP","CTNET": Define PDP context for IP
#   5.2 Activate PDP Context (page 191):
#       - AT+CGACT=1,1: Activate PDP context for CID 1
#   5.3 Make a call through specified PDP:
#       - AT+QNETDEVCTL=1,1,1: Activate network interface for CID 1
#   5.4 Check Network Status (page 204):
#       - AT+QNETDEVSTATUS?: Query network interface status
# Step 6: Set Up Networking on the Host
#   - echo normal > /sys/class/net/pcie0/mode: Set interface to normal mode
#   - udhcpc -i pcie0: Obtain IP address via DHCP
# Step 7: Configure OpenWrt Network
#   - Append 'wwan' and 'dummy0' interfaces to /etc/config/network
#   - Restart network service to apply changes
# Step 8: Verify Connectivity
#   - ping www.baidu.com: Test network connectivity
