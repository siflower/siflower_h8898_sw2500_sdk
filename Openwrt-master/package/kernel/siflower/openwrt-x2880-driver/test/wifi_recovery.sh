#!/bin/sh

#set recovery env
for _dev in /sys/class/ieee80211/*; do
	[ -e "$_dev" ] || continue
	dev="${_dev##*/}"
done

# if set assert, platform restart continuously less 3 seconds cause error, so sleep longer
timer=100000
while [ $timer -gt 0 ]; do
	timer=$(($timer - 1))
	for _phy in /sys/class/ieee80211/*; do
		[ -e "$_phy" ] || continue
		phy="${_phy##*/}"
		# trigger assert_err, restart driver, if need restart driver, open assert and close recovery
		# echo assert > /sys/kernel/debug/ieee80211/$phy/siwifi/rw/console
		# trigger assert_rec, wifi recovery
		echo recovery > /sys/kernel/debug/ieee80211/$phy/siwifi/rw/console
		sleep 2
		while [ 1 ]; do
			state=`cat /sys/kernel/debug/ieee80211/$phy/siwifi/run_state`
			if [ "$(($state))" -eq "1" ]; then
				echo "+++++++++++++++++++++++++++++++++++++++++++++++timer=$timer"
				break
			else
				sleep 1
			fi
		done
	done
	sleep 2
done
