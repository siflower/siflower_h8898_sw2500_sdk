#!/bin/sh
#$1 cycle times
count=$1

while [ $count -gt 0 ]
do
    echo a8:5a:f3:ff:00:a4 7 10000 > /sys/kernel/debug/ieee80211/phy0/siwifi/sendraw
    let count-=1
done
