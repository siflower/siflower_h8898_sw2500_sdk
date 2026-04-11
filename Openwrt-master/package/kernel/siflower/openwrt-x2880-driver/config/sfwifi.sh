#!/bin/sh

. /lib/functions.sh

INSMOD="/sbin/insmod"
RMMOD="/sbin/rmmod"

insmod_umac(){
    all_modparams=
    extra_user_args=
    modparams="
    ht_on=${ht_on-1}
    vht_on=${vht_on-1}
    he_on=${he_on-1}
    he_dl_on=${he_dl_on-0}
    he_ul_on=${he_ul_on-0}
    ldpc_on=${ldpc_on-1}
    stbc_on=${stbc_on-1}
    phycfg=${phycfg-0}
    uapsd_timeout=${uapsd_timeout-0}
    ap_uapsd_on=${ap_uapsd_on-0}
    sgi=${sgi-1}
    sgi80=${sgi80-1}
    use_2040=${use_2040-1}
    nss=${nss-2}
    amsdu_rx_max=${amsdu_rx_max-1}
    bfmee=${bfmee-0}
    bfmer=${bfmer-0}
    mesh=${mesh-0}
    murx=${murx-0}
    mutx=${mutx-0}
    mutx_on=${mutx_on-1}
    use_160=${use_160-1}
    custregd=${custregd-0}
    lp_clk_ppm=${lp_clk_ppm-1000}
    addr_maskall=${addr_maskall-0}
    not_send_null=${not_send_null-0}
    ps_on=${ps_on-1}
    tx_lft=${tx_lft-100}
    tdls=${tdls-1}
    txpower_lvl=${txpower_lvl-1}
    uf=${uf-0}
    ampdu_max_cnt=${ampdu_max_cnt-128}
    rts_cts_change=${rts_cts_change-2}
    "
    fmac_modparams="
    ant_div=${ant_div-1}
    tx_queue_num=${tx_queue_num-5}
    # tx_queue_num is 8 if open NEW_AC macro
    # tx_queue_num=${tx_queue_num-8}
    "
    iqc_modparams="
    noiqc=1
    "
        if [ "Xlb" = "X$2" ];then
                all_modparams="band_mode=${band_mode-0}"$modparams$fmac_modparams
        elif [ "Xhb" = "X$2" ];then
                all_modparams="band_mode=${band_mode-1}"$modparams$fmac_modparams
        elif [ "Xall" = "X$2" ];then
                all_modparams="band_mode=${band_mode-2}"$modparams$fmac_modparams
        else
                all_modparams="band_mode=${band_mode-2}"$modparams$fmac_modparams
        fi

        if [ "Xnoiqc" = "X$3" ];then
                all_modparams=$all_modparams$iqc_modparams
        fi

    extra_user_args="force_mod_name=siwifi_fmac"
    cmd="/sbin/insmod $1 $extra_user_args $all_modparams"
    eval $cmd
}

insmod_cfg80211(){
    modprobe cfg80211
}

insmod_fmac() {
        if [ "X$1" = "Xlb" ];then
                insmod_umac siwifi_fmac lb $2
        elif [ "X$1" = "Xhb" ];then
                insmod_umac siwifi_fmac hb $2
        elif [ "X$1" = "Xall" ];then
                insmod_umac siwifi_fmac all $2
        else
                insmod_umac siwifi_fmac all $2
        fi
}

unload_fmac(){
    cmd="/sbin/rmmod siwifi_fmac"
    eval $cmd
}
