#!/bin/sh
function get_phy()
{
    if [ ! -d /sys/kernel/debug/ieee80211 ]
    then
        return
    fi
    if [ "$gmn" =  "5" ]
    then
        iwinfo | grep  'Channel:'|grep -n 'Channel'|grep '(5.'| cut -d : -f1 > wifi.txt
        iwinfo | grep 'PHY' | grep -n 'PHY' |grep "$(cat wifi.txt):" | awk '{print $7}' > phy.txt
    elif [ "$gmn" = "24" ]
    then
        iwinfo | grep  'Channel:'|grep -n 'Channel'|grep '(2.'|cut -d : -f1 > wifi.txt
        iwinfo | grep 'PHY' | grep -n 'PHY' |grep "$(cat wifi.txt):" | awk '{print $7}' > phy.txt
    fi
    echo `cat phy.txt`
}
function get_station()
{
    ls /sys/kernel/debug/ieee80211/$(get_phy)/siwifi/stations/ > sta.txt
    echo `cat sta.txt`
}
function rate_cck()
{
    case $rate in
    1)echo 0;;
    2)echo 1;;
    5.5)echo 2;;
    11)echo 3;;
    *)
    echo ""
    return ;;
    esac
}
function rate_ofdm()
{
    case $rate in
    6)echo 0;;
    9)echo 1;;
    12)echo 2;;
    18)echo 3;;
    24)echo 4;;
    36)echo 5;;
    48)echo 6;;
    54)echo 7;;
    *)
    echo ""
    return ;;
    esac
}
function bw_list()
{
    case $bw in
    20)echo 0;;
    40)echo 1;;
    80)echo 2;;
    160)echo 3;;
    *)
    echo ""
    return ;;
    esac
}




nss=1 mcs=0 sgi=0 bw=20 ru=26 driver= mode=HT rate= idx= verbose=0 preamble=0 rc=rwnx
gmn=5

help="rc_idx [-v (verbose)] [-s(softmac)|-f(fullmac)] [-t <HT|VHT|HE|CCK|OFDM>] [-n <nss>] [-c <24|5>] [-m <mcs>] [-r <rate mbps>] [-g <gi/preamble>] [-b <20|40|80>]

rc_idx -t CCK -r <1|2|5.5|11> -p <0 (short preamble)|1 (long preamble)> -c <24(2.4G-phyname)|5(5G-phyname) >
rc_idx -t OFDM -r <6|9|12|18|24|36|48|54> -c <24|5>
rc_idx -t HT -m [0-15] -g <0(long gi)|1(short gi)> -b <20|40> -c <24|5>
rc_idx -t VHT -n <1|2> -m [0-9] -g <0(long gi)|1(short gi)> -b <20|40|80> -c <24|5>
rc_idx -t HE -n <1|2> -m [0-11] -g <0(0.8us gi)|1(1.6us gi)|2(3.2us gi)> -b <20|40|80> -c <24|5>

"


while getopts "sfn:m:r:g:p:b:u:t:hv:c:" o; do
        case "$o" in
                s) driver=rwnx_drv ;;
                f) driver=rwnx_fdrv ;;
                n) nss=$OPTARG ;;
                m) mcs=$OPTARG ;;
                r) rate=$OPTARG ;;
                g) sgi=$OPTARG ;;
                p) preamble=$OPTARG ;;
                b) bw=$OPTARG ;;
                u) ru=$OPTARG ;;
                t) mode=$OPTARG ;;
                v) verbose=1 ;;
                c) gmn=$OPTARG ;;
                h|*) echo "$help" > /dev/console
                return;;
        esac
done



case $mode in
        CCK|OFDM)
                if [ $bw != "20" ] && [ $verbose -eq 1 ]
                then
                    echo "Assume 20Mhz bandwidth for $mode (ignore -b $bw)"
                fi
                case $mode in
                CCK)
                    if [ "$preamble" != "0" -a "$preamble" != "1" ]
                    then
                        echo "Invalid value for -p option, should be 0 or 1 not $preamble"
                        return 1
                    fi

                    if [ ! $rate ] || [ ! $(rate_cck) ]
                    then
                        echo "Invalid or missing rate for $mode [$rate]"
                        echo  "possible rate values: 1 2 5.5 11"
                        return 1
                    fi
                    rate=$(rate_cck)

                    idx=$(((rate * 2) + preamble))
                    echo $idx
                    echo $idx > /sys/kernel/debug/ieee80211/$(get_phy)/siwifi/stations/$(get_station)/rc/fixed_rate_idx
                    ;;
                *)
                    if [ ! $rate ] || [ ! $(rate_ofdm) ]
                    then
                        echo "Invalid or missing rate for $mode [$rate]"
                        echo  "possible rate values: 6 9 12 18 24 36 48 54"
                        return 1
                    fi

                    rate=$(rate_ofdm)
                    idx=$((8+(rate - 4)))
                    echo $idx
                    echo $idx > /sys/kernel/debug/ieee80211/$(get_phy)/siwifi/stations/$(get_station)/rc/fixed_rate_idx
                esac
                ;;
        HT|VHT|HE)
                if [ $rate ] && [ $verbose -eq 1 ]
                then
                    echo "-r option for $mode is useless, ignore it"
                fi

                if [ $ru ] && [ $verbose -eq 1 ]
                then
                    echo "-u option for $mode is useless, ignore it"
                fi
                if [ ! $(bw_list) ] || ([ $mode = "HT" ] && [ $(bw_list) -gt 1 ])
                then
                    echo "Invalid bandwidth [$bw]"
                    return 1
                fi
                bw=$(bw_list)


                if [ "$sgi" != "0" -a "$sgi" != "1" ]
                then
                    if ! [[ $mode = "HE" ]] || [ "$sgi" != 2 ]
                    then
                        echo "Invalid value for -g option, should be 0 or 1 (or 2 for HE) not $sgi"
                        return 1
                    fi
                fi

                if [ -n "$nss" -a "$nss" != "1" -a "$nss" != "2" -a "$nss" != "3" ]
                then
                    echo "Invalid value for -n option, should be 1,2 or 3 not $nss"
                    return 1
                fi

                if [[ $mode = "HT" ]]
                then
                    if [ $nss ] && [ $nss -ne $((($mcs / 8) + 1)) ]
                    then
                        echo "Mismatch between nss and mcs. In HT mode use mcs to specify nss"
                        return 1
                    fi
                    nss=$((mcs / 8))
                    mcs=$((mcs % 8))

                else
                    mcs_max=9
                    if [[ $mode = "HE" ]]
                    then
                        mcs_max=11
                    fi
                    if [ $mcs -gt $mcs_max ]
                    then
                        echo "In $mode mcs should be in [0-$mcs_max]"
                        return 1
                    fi
                    nss=${nss-1}
                    nss=$((nss - 1))
                fi

                if [[ $mode = "HE" ]]
                then
                    idx=$((784 + (nss * 144) + (mcs * 12) + bw * 3 + sgi ))
                elif [[ $mode = "VHT" ]]
                then
                    idx=$((144 + (nss * 80) + (mcs * 8) + bw * 2 + sgi ))
                else
                    idx=$((16 + (nss * 32) + (mcs * 4) + bw * 2 + sgi ))
                fi
                echo $idx
                echo $idx > /sys/kernel/debug/ieee80211/$(get_phy)/siwifi/stations/$(get_station)/rc/fixed_rate_idx
                ;;
        *)
                echo "Invalid mode [$mode]"
                return 1
esac


rm phy.txt
rm wifi.txt
rm sta.txt
