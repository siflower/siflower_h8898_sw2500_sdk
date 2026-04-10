#!/bin/sh

check_ifc_count() {
	return `ifconfig |awk '/(HWaddr)/{print $1}' |grep -c wlan`
}

timer=$1
cycle=0
echo "start wifi reset test with count $timer" > /dev/ttyS0
while [ $timer -gt 0 ]; do
	timer=$(($timer - 1))
	cycle=$(($cycle + 1))
	echo "start cycle $cycle" > /dev/ttyS0
	sfwifi reset fmac
	sleep 5
	check_ifc_count
	if [ $? -eq 2  ]; then
		echo ">>>>>>>>reset success<<<<<<<<<" > /dev/ttyS0
	else
		echo "!!!!!!!!!!!! reset fail !!!!!!!!!!!!!" > /dev/ttyS0
		ifconfig > /tmp/wifi_reset.log
		logread >> /tmp/wifi_reset.log
		break
	fi
	sleep 1 
done
