/**
 *****************************************************************************************
 * @file gendesc.c
 *
 * @brief Main file for la application, sending DBG commands to LMAC layer
 *
 * Copyright (C) 2024 Siflower
 *
 *****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * DEFINES
 *****************************************************************************************
 */
enum siwifi_hw_type {
    SIWIFI_NX = (1 << 0),
    SIWIFI_HE = (1 << 1),
    SIWIFI_HE_AP = (1 << 2),
    SIWIFI_ANY = SIWIFI_NX | SIWIFI_HE | SIWIFI_HE_AP,
};

struct bitfield_desc {
    enum siwifi_hw_type hw;
    char *name;
    int endbit;
    int startbit;
    char *val_name[16];
};

struct rx_hd
{
    uint32_t upatternrx;
    uint32_t next;
    uint32_t first_pbd_ptr;
    uint32_t swdesc;
    uint32_t datastartptr;
    uint32_t dataendptr;
    uint32_t headerctrlinfo;
    uint32_t frmlen;
    uint32_t tsflo;
    uint32_t tsfhi;
    uint32_t recvec1a;
    uint32_t recvec1b;
    uint32_t recvec1c;
    uint32_t recvec1d;
    uint32_t recvec2a;
    uint32_t recvec2b;
    uint32_t statinfo;
};

struct rx_bd
{
    uint32_t upattern;
    uint32_t next;
    uint32_t datastartptr;
    uint32_t dataendptr;
    uint16_t bufstatinfo;
    uint16_t reserved;
};

struct tx_hd
{
    uint32_t upatterntx;
    uint32_t nextfrmexseq_ptr;
    uint32_t nextmpdudesc_ptr;
    uint32_t first_pbd_ptr;
    uint32_t datastartptr;
    uint32_t dataendptr;
    uint32_t frmlen;
    uint32_t frmlifetime;
    uint32_t phyctrlinfo;
    uint32_t policyentryaddr;
    uint32_t opt20mhzlen;
    uint32_t opt40mhzlen;
    uint32_t opt80mhzlen;
    uint32_t macctrlinfo1;
    uint32_t macctrlinfo2;
    uint32_t statinfo;
    uint32_t mediumtimeused;
};

struct tx_frmx_hd
{
    uint32_t upatterntx;
    uint32_t nextfrmexseq_ptr;
    uint32_t nextfrm_ptr;
    uint32_t nextuserfrmex_ptr;
    uint32_t barfrmex_ptr;
    uint32_t policyentryaddr;
    uint32_t firstmpdudesc_ptr;
    uint32_t frmlen;
    uint32_t frmlifetime;
    uint32_t phyctrlinfo1;
    uint32_t phyctrlinfo2;
    uint32_t macctrlinfo1;
    uint32_t frmxstatinfo;
    uint32_t mediumtimeused;
};

struct tx_mpdu_hd
{
    uint32_t upatterntx;
    uint32_t nextmpdudesc_ptr;
    uint32_t first_pbd_ptr;
    uint32_t datastartptr;
    uint32_t dataendptr;
    uint32_t frmlen;
    uint32_t macctrlinfo2;
    uint32_t mpdustatinfo;
};

struct bitfield_desc phyctrlinfo_desc[] = {
    {SIWIFI_HE | SIWIFI_HE_AP, "Partial ID", 30, 22, {NULL}},
    {SIWIFI_ANY, "Group ID", 21, 16, {NULL}},
    {SIWIFI_NX, "MU-MIMO", 15, 15, {NULL}},
    {SIWIFI_HE_AP, "MU", 15, 15, {NULL}},
    {SIWIFI_HE_AP, "# User", 14, 7, {NULL}},
    {SIWIFI_NX, "RIFS TX", 14,14, {NULL}},
    {SIWIFI_NX, "User Pos", 13, 12, {NULL}},
    {SIWIFI_NX | SIWIFI_HE, "Pkt Traffic Info", 11, 8, {NULL}},
    {SIWIFI_ANY, "Continous TX", 6, 6, {"", "Continous TX"}},
    {SIWIFI_ANY, "Doze Not Allowed", 5, 5, {"Doze Allowed", "Doze Not Allowed"}},
    {SIWIFI_ANY, "Dynamic BW", 4, 4, {"Static BW", "Dynamic BW"}},
    {SIWIFI_ANY, "BW Signaling", 3, 3, {NULL}},
    {SIWIFI_NX | SIWIFI_HE_AP, "Smoothing Ctrl", 2, 2, {"", "Smoothing Prot"}},
    {SIWIFI_NX | SIWIFI_HE_AP, "Smoothing", 1, 1, {"", "Smoothing"}},
    {SIWIFI_NX | SIWIFI_HE_AP, "Sounding", 0, 0, {"", "Sounding"}}
};

struct bitfield_desc phyctrlinfo2_desc[] = {
    {SIWIFI_HE_AP, "Trigger Idx", 31, 24, {NULL}},
    {SIWIFI_HE_AP, "Inactive SubCh", 23, 16, {NULL}},
    {SIWIFI_HE_AP, "sta ID", 10, 0, {NULL}},
};

struct bitfield_desc macctrlinfo1_desc[] = {
    {SIWIFI_ANY, "Dur Prot", 31, 16, {NULL}},

    {SIWIFI_HE_AP, "Write Ack", 15, 15, {NULL}},
    {SIWIFI_HE_AP, "Low Rate Retry", 14, 14, {NULL}},
    {SIWIFI_HE_AP, "BA Buf Size", 13, 12, {NULL}},
    {SIWIFI_HE_AP, "Ack", 11, 10, {"No Ack", "Normal Ack", "Block Ack", "Compressed Block Ack"}},
    {SIWIFI_HE_AP, "Irq TX", 9, 9, {NULL}},
    {SIWIFI_HE_AP, "Pkt Traffic Info", 7, 4, {NULL}},
    {SIWIFI_HE_AP, "A-MPDU", 0, 0, {"", "A-MPDU"}},

    {SIWIFI_NX | SIWIFI_HE, "Write Ack", 14, 14, {NULL}},
    {SIWIFI_NX | SIWIFI_HE, "Low Rate Retry", 13, 13, {NULL}},
    {SIWIFI_NX | SIWIFI_HE, "LSig TXOP Prot", 12, 12, {NULL}},
    {SIWIFI_NX | SIWIFI_HE, "LSig TXOP", 11, 11, {NULL}},
    {SIWIFI_NX | SIWIFI_HE, "Ack", 10, 9, {"No Ack", "Normal ACk", "Block Ack", "Compressed Block Ack"}}
};

struct bitfield_desc macctrlinfo2_desc[] = {
    {SIWIFI_ANY, "No MAC Hdr", 31, 31, {"", "No MAC Hdr Update"}},
    {SIWIFI_ANY, "Dont Encrypt", 30, 30, {"", "No Encrypt"}},
    {SIWIFI_ANY, "Dont Touch FC", 29, 29, {"", "No Frame Ctrl Update"}},
    {SIWIFI_ANY, "Dont Touch Dur", 28, 28, {"", "No Duration Update"}},
    {SIWIFI_ANY, "Dont Touch QOS", 27, 27, {"", "No QOS Update"}},
    {SIWIFI_ANY, "Dont Touch HTC", 26, 26, {"", "No HT Ctrl Update"}},
    {SIWIFI_ANY, "Dont Touch TSF", 25, 25, {"", "No TSF Update"}},
    {SIWIFI_ANY, "Dont Touch DTIM", 24, 24, {"", "No DTIM Update"}},
    {SIWIFI_ANY, "Dont Touch FCS", 23, 23, {"", "No FCS Update"}},

    {SIWIFI_HE_AP, "Dont Fragment", 22, 22, {"", "No Fragment"}},

    {SIWIFI_NX | SIWIFI_HE, "Under BA", 22, 22, {NULL}},
    {SIWIFI_NX | SIWIFI_HE, "Desc", 21, 19, {"MSDU", "Frag MSDU First", "Frag MSDU", "Frag MSDU Last",
                               "AMPDU Extra", "AMPDU First", "AMPDU Inter", "AMPDU Last"}},
    {SIWIFI_NX | SIWIFI_HE, "# Delim", 18, 9, {NULL}},

    {SIWIFI_ANY, "IRQ TX", 8, 8, {NULL}},
    {SIWIFI_ANY, "Sub Type", 6, 3, {NULL}},
    {SIWIFI_ANY, "Type", 2, 1, {"Mgmt", "Ctrl", "Data", "Rsvd"} },
    {SIWIFI_ANY, "Valid (Sub)Type", 0, 0, {NULL}}
};

struct bitfield_desc statinfo_desc[] ={
    {SIWIFI_ANY, "Done", 31, 31, {NULL}},
    {SIWIFI_ANY, "Done SW", 30, 30, {NULL}},
    {SIWIFI_ANY, "Under BA", 29, 29, {NULL}},

    {SIWIFI_HE_AP, "A-MPDU", 28, 28, {"", "A-MPDU"}},
    {SIWIFI_HE_AP, "BW", 26, 24, {"20MHz", "40MHz", "80MHz", "160MHz",
	     "80MHz (Punctured Primary)", "80 MHz (Punctured Secondary)",
	     "160MHz (Punctured Primary)", "160MHz (Punctured Secondary)"}},

    {SIWIFI_NX | SIWIFI_HE, "Desc", 28, 26, {"MSDU", "Frag MSDUFirst", "Frag MSDU", "Frag MSDU Last",
         "AMPDU Extra", "AMPDU First", "AMPDU Inter", "AMPDU Last"}},
    {SIWIFI_NX | SIWIFI_HE, "BW", 25, 24, {"20MHz", "40MHz", "80MHz", "160MHz"}},

    {SIWIFI_ANY, "TX Success", 23, 23, {NULL}},
    {SIWIFI_HE_AP, "User Dropped", 19, 19, {"", "User Dropped"}},
    {SIWIFI_ANY, "BA Recv", 18, 18, {NULL}},
    {SIWIFI_ANY, "Lifetime Exp", 17, 17, {NULL}},
    {SIWIFI_ANY, "Retry Limit Reach", 16, 16, {NULL}},
    {SIWIFI_ANY, "# MPDU Retries", 15, 8, {NULL}},
    {SIWIFI_ANY, "# RTS Retries", 7, 0, {NULL}}
};

struct bitfield_desc mpdustatinfo_desc[] ={
    {SIWIFI_HE_AP, "Done", 31, 31, {NULL}},
    {SIWIFI_HE_AP, "Done SW", 30, 30, {NULL}},
    {SIWIFI_HE_AP, "Frag", 14, 14, {"","Fragmented"}},
    {SIWIFI_HE_AP, "Fragment Length", 13, 0, {NULL}},
};

/// Definition of a TX payload buffer descriptor.
struct tx_pbd
{
    uint32_t upatterntx;
    uint32_t next;
    uint32_t datastartptr;
    uint32_t dataendptr;
    uint32_t bufctrlinfo;
};

struct bitfield_desc bufctrlinfo_desc[] ={
    {SIWIFI_ANY, "Done HW", 31, 31, {NULL}},
    {SIWIFI_ANY, "Done SW", 30, 30, {NULL}},
    {SIWIFI_ANY, "Irq Enable", 0, 0, {NULL}},
};

struct tx_policy_tbl
{
    uint32_t upatterntx;
    uint32_t phycntrlinfo1;
    uint32_t phycntrlinfo2;
    uint32_t maccntrlinfo1;
    uint32_t maccntrlinfo2;
    uint32_t ratecntrlinfo[4];
    uint32_t powercntrlinfo[4];
};

struct tx_su_policy_tbl_he_ap
{
    uint32_t upatterntx;
    uint32_t phycntrlinfo1;
    uint32_t phycntrlinfo2;
    uint32_t maccntrlinfo1;
    uint32_t maccntrlinfo2;
    uint32_t ratecntrlinfo[4][2];
};

struct bitfield_desc phycntrlinfo1_desc[] = {
    {SIWIFI_HE, "Spatial Reuse", 27, 24, {NULL}},
    {SIWIFI_HE, "Midamble Period", 21, 21, {"Midamble Period=10 Symb", "Midamble Period=20 Symb"}},
    {SIWIFI_HE, "Doppler", 20, 20, {NULL}},

    {SIWIFI_HE_AP, "Spatial Reuse", 23, 20, {NULL}},

    {SIWIFI_ANY, "# Transmit Chain Prot", 19, 17, {NULL}},
    {SIWIFI_ANY, "# Transmit Chain", 16, 14, {NULL}},

    {SIWIFI_HE_AP, "Midamble Period", 10, 10, {"Midamble Period=10 Symb", "Midamble Period=20 Symb"}},
    {SIWIFI_HE_AP, "Doppler", 9, 9, {NULL}},

    {SIWIFI_ANY, "STBC", 8, 7, {NULL}},
    {SIWIFI_ANY, "FEC", 6, 6, {NULL}},
    {SIWIFI_ANY, "Ext Spatial Stream", 5, 4, {NULL}},
    {SIWIFI_ANY, "Beamforming Exch", 3, 3, {"Bfm Exch=Qos NULL/ACK", "Bfm Exch=RTS/CTS"}},
    {SIWIFI_HE, "Smoothing Prot", 1, 2, {"No Smoothing Prot", "SmoothiNg Prot"}},
    {SIWIFI_HE, "Smoothing", 1, 1, {"No Smoothing", "Smoothing"}},
    {SIWIFI_HE, "Sounding", 1, 0, {"Not Sounding", "Sounding"}},
};

struct bitfield_desc phycntrlinfo2_desc[] = {
    {SIWIFI_HE | SIWIFI_HE_AP, "Pkt Ext", 28, 26, {NULL}},
    {SIWIFI_HE | SIWIFI_HE_AP, "BSS Color", 25, 20, {NULL}},
    {SIWIFI_HE | SIWIFI_HE_AP, "Uplink", 18, 18, {NULL}},
    {SIWIFI_HE | SIWIFI_HE_AP, "Beam Change", 17, 17, {"Beam Change=Same", "Beam Change=Diff"}},
    {SIWIFI_ANY, "Beamformed", 16, 16, {NULL}},
    {SIWIFI_ANY, "Spatial Map Matrix", 15, 8, {NULL}},
    {SIWIFI_ANY, "Antenna Set", 7, 0, {NULL}},
};

struct bitfield_desc maccntrlinfo1_desc[] = {
    {SIWIFI_HE_AP, "Min 1st frag size", 24, 23, {"no 1st frag limit", "1st frag 128", "1st frag 256", "1st frag 512"}},
    {SIWIFI_HE_AP, "Min MPDU Start Spacing", 22, 20, {NULL}},
    {SIWIFI_ANY, "Key RAM Idx For RA", 19, 10, {NULL}},
    {SIWIFI_ANY, "Key RAM Idx", 9, 0, {NULL}},
};

struct bitfield_desc maccntrlinfo2_desc[] = {
    {SIWIFI_ANY, "RTS Threshold", 27, 16, {NULL}},
    {SIWIFI_ANY, "Short Retry Limit", 15, 8, {NULL}},
    {SIWIFI_ANY, "Long Retry Limit", 7, 0, {NULL}},
};

struct bitfield_desc ratecntrlinfo1_desc[] = {
    {SIWIFI_ANY, "# RETRY", 31, 29, {NULL}},

    {SIWIFI_HE_AP, "TX Power", 27, 20, {NULL}},
    {SIWIFI_HE_AP, "HE-LTF", 19, 18, {"1x HE-LTF", "2x HE-LTF", "4x HE-LTF", "-"}},
    {SIWIFI_HE_AP, "GI", 17, 16, {"Long GI/0.8us", "Short GI/1.6us", "3.2us"}},
    {SIWIFI_HE_AP, "Mod", 15, 12, {"Non HT", "OFDM Dup", "HT", "HT-GF", "VHT", "HE-SU", "HE-MU", "HE-EXT",
         "HE-TB", "-", "-", "-", "-", "-", "-", "-"}},
    {SIWIFI_HE_AP, "Preamble", 11, 11, {"Short Preamble", "Long Preamble"}},
    {SIWIFI_HE_AP, "BW", 10, 8, {"20MHz", "40MHz", "80MHz", "160MHz",
	     "80MHz (Punctured Primary)", "80 MHz (Punctured Secondary)",
	     "160MHz (Punctured Primary)", "160MHz (Punctured Secondary)"}},
    {SIWIFI_HE_AP, "DCM", 7, 7, {"", "DCM"}},

    {SIWIFI_NX | SIWIFI_HE, "Mod (Prot)", 28, 26, {"Non HT", "OFDM Dup", "HT", "HT-GF", "VHT", "HE-SU", "-", "HE-EXT"}},
    {SIWIFI_NX | SIWIFI_HE, "BW (Prot)", 25, 24, {"20MHz", "40MHz", "80MHz", "160MHz"}},
    {SIWIFI_NX | SIWIFI_HE, "MCS Index (Prot)", 23, 17, {NULL}},
    {SIWIFI_NX | SIWIFI_HE, "Nav Prot", 16, 14, {"None", "Self CTS", "RTS/CTS", "RTS/CTS/QAP", "STBC", "-", "-", "-"}},
    {SIWIFI_NX | SIWIFI_HE, "Mod", 13, 11, {"Non HT", "OFDM Dup", "HT", "HT-GF", "VHT", "HE-SU", "-", "HE-EXT"}},
    {SIWIFI_NX, "Preamble", 10, 10, {"Short Preamble", "Long Preamble"}},
    {SIWIFI_NX, "Short GI", 9, 9, {"Long GI", "Short GI"}},
    {SIWIFI_HE, "GI/Preamle", 10, 9, {"Short Preamle/Long GI/0.8us", "Long Preamle/Short GI/1.6us", "3.2us"}},
    {SIWIFI_NX | SIWIFI_HE, "BW", 8, 7, {"20MHz", "40MHz", "80MHz", "160MHz"}},

    {SIWIFI_ANY, "MCS Index", 6, 0, {NULL}},
};

struct bitfield_desc powercntrlinfo_desc[] = {
    {SIWIFI_HE, "DCM", 18, 18, {NULL}},
    {SIWIFI_HE, "HE-LTF", 17, 16, {"1x HE-LTF", "2x HE-LTF", "4x HE-LTF", "-"}},
    {SIWIFI_NX | SIWIFI_HE, "Power Level (Prot)", 15, 8, {NULL}},
    {SIWIFI_NX | SIWIFI_HE, "Power Level", 7, 0, {NULL}},
};

struct bitfield_desc ratecntrlinfo2_desc[] = {
    {SIWIFI_HE_AP, "Nav Prot", 31, 29, {"No Prot", "Self CTS", "RTS/CTS", "RTS/CTS/QAP", "STBC", "-", "-", "-"}},
    {SIWIFI_HE_AP, "TX Power", 27, 20, {NULL}},
    {SIWIFI_HE_AP, "Mod (Prot)", 15, 12, {"Non HT", "OFDM Dup", "HT", "HT-GF", "VHT", "HE-SU", "HE-MU", "HE-EXT",
         "HE-TB", "-", "-", "-", "-", "-", "-", "-"}},
    {SIWIFI_HE_AP, "Preamble", 11, 11, {"Short Preamble", "Long Preamble"}},
    {SIWIFI_HE_AP, "BW (PROT)", 10, 8, {"20MHz", "40MHz", "80MHz", "160MHz",
	     "80MHz (Punctured Primary)", "80 MHz (Punctured Secondary)",
	     "160MHz (Punctured Primary)", "160MHz (Punctured Secondary)"}},
};

struct tx_compressed_policy_tbl {
    uint32_t upatterntx;
    uint32_t sec_user_control;
};

struct bitfield_desc sec_user_control_desc[] = {
    {SIWIFI_NX, "Key Ram Idx", 25, 16, {NULL}},
    {SIWIFI_NX, "Spatial Map Matrix", 15, 8, {NULL}},
    {SIWIFI_NX, "FEC", 7, 7, {NULL}},
    {SIWIFI_NX, "MCS Index", 6, 0, {NULL}},
};

struct tx_pu_policy_tbl_he_ap {
    uint32_t upatterntx;
    uint32_t per_user_phyinfo;
    uint32_t per_user_macinfo;
};

struct bitfield_desc per_user_phyinfo_desc[] = {
    {SIWIFI_HE_AP, "Spatial Map Matrix", 31, 24, {NULL}},
    {SIWIFI_HE_AP, "User Pos", 17, 16, {NULL}},
    {SIWIFI_HE_AP, "Starting STS", 15, 13, {NULL}},
    {SIWIFI_HE_AP, "Beamformed", 12, 12, {NULL}},
    {SIWIFI_HE_AP, "FEC", 11, 11, {NULL}},
    {SIWIFI_HE_AP, "Pkt Extension", 10, 8, {NULL}},
    {SIWIFI_HE_AP, "DCM", 7, 7, {NULL}},
    {SIWIFI_HE_AP, "MCS Index", 6, 0, {NULL}},
};

struct bitfield_desc per_user_macinfo_desc[] = {
    {SIWIFI_HE_AP, "User RU Allocation", 31, 24, {NULL}},
    {SIWIFI_HE_AP, "Min Frag size", 14, 13, {NULL}},
    {SIWIFI_HE_AP, "Min MPDU Start Spacing", 12, 10, {NULL}},
    {SIWIFI_HE_AP, "Key RAM Idx", 9, 0, {NULL}},
};

struct tx_mu_policy_tbl_he_ap {
    uint32_t upatterntx;
    uint32_t phycntrlinfo1;
    uint32_t phycntrlinfo2;
    uint32_t userphycntrlinfo;
    uint32_t usermaccntrlinfo;
    uint32_t muratecntrlinfo;
    uint32_t muprotratecntrlinfo;
    uint32_t muphycntrlinfo1;
    uint32_t muphycntrlinfo2;
    uint32_t muphycntrlinfo3;
};

struct tx_policy_tbl_he_ap {
    union {
        struct tx_su_policy_tbl_he_ap su;
        struct tx_mu_policy_tbl_he_ap mu;
        struct tx_pu_policy_tbl_he_ap pu;
    };
};

struct bitfield_desc mu_phycntrlinfo1_desc[] = {
    {SIWIFI_HE_AP, "Spatial Reuse", 23, 20, {NULL}},
    {SIWIFI_HE_AP, "# Transmit Chain Prot", 19, 17, {NULL}},
    {SIWIFI_HE_AP, "# Transmit Chain", 16, 14, {NULL}},
    {SIWIFI_HE_AP, "Midamble Period", 10, 10, {"Midamble Period=10 Symb", "Midamble Period=20 Symb"}},
    {SIWIFI_HE_AP, "Doppler", 9, 9, {NULL}},
    {SIWIFI_HE_AP, "STBC", 8, 7, {NULL}},
    {SIWIFI_HE_AP, "Beamforming Exch", 3, 3, {"Bfm Exch=Qos NULL/ACK", "Bfm Exch=RTS/CTS"}},
};

struct bitfield_desc mu_phycntrlinfo2_desc[] = {
    {SIWIFI_HE_AP, "Bss Color", 25, 20, {NULL}},
    {SIWIFI_HE_AP, "Beam Change", 17, 17, {"Beam Change=Same", "Beam Change=Diff"}},
    {SIWIFI_HE_AP, "Antenna Set", 7, 0, {NULL}},
};

struct bitfield_desc mu_userphycntrlinfo_desc[] = {
    {SIWIFI_HE_AP, "Spatial Map Matrix", 31, 24, {NULL}},
    {SIWIFI_HE_AP, "User Pos", 17, 16, {NULL}},
    {SIWIFI_HE_AP, "Starting STS", 15, 13, {NULL}},
    {SIWIFI_HE_AP, "Beamformed", 12, 12, {NULL}},
    {SIWIFI_HE_AP, "FEC", 11, 11, {NULL}},
    {SIWIFI_HE_AP, "Pkt Extension", 10, 8, {NULL}},
    {SIWIFI_HE_AP, "DCM", 7, 7, {NULL}},
    {SIWIFI_HE_AP, "MCS Index", 6, 0, {NULL}},
};

struct bitfield_desc mu_usermaccntrlinfo_desc[] = {
    {SIWIFI_HE_AP, "User RU Allocation", 31, 24, {NULL}},
    {SIWIFI_HE_AP, "Min Frag Size", 14, 13, {NULL}},
    {SIWIFI_HE_AP, "Min MPDU Start Spacing", 12, 10, {NULL}},
    {SIWIFI_HE_AP, "Key RAM Idx", 9, 0, {NULL}},
};

struct bitfield_desc mu_muratecntrlinfo_desc[] = {
    {SIWIFI_HE_AP, "TX Power", 27, 20, {NULL}},
    {SIWIFI_HE_AP, "HE-LTF", 19, 18, {"1x HE-LTF", "2x HE-LTF", "4x HE-LTF", "-"}},
    {SIWIFI_HE_AP, "GI", 17, 16, {"LONG GI/0.8us", "SHORT GI/1.6us", "3.2us"}},
    {SIWIFI_HE_AP, "MOD", 15, 12, {"Non HT", "OFDM Dup", "HT_MIXED", "HT_GF", "VHT", "HE-SU", "HE-MU", "HE-EXT",
         "HE-TB", "-", "-", "-", "-", "-", "-", "-"}},
    {SIWIFI_HE_AP, "BW", 10, 8, {"20MHz", "40MHz", "80MHz", "160MHz",
	     "80MHz (Punctured Primary)", "80 MHz (Punctured Secondary)",
	     "160MHz (Punctured Primary)", "160MHz (Punctured Secondary)"}},
};

struct bitfield_desc mu_muprotratecntrlinfo_desc[] = {
    {SIWIFI_HE_AP, "NAV Prot", 31, 29, {"No Prot", "Self CTS", "RTS/CTS", "RTS/CTS/QAP", "STBC", "-", "-", "-"}},
    {SIWIFI_HE_AP, "TX Power", 27, 20, {NULL}},
    {SIWIFI_HE_AP, "Mod (Prot)", 15, 12, {"Non HT", "OFDM Dup", "HT Mixed", "HT-GF", "VHT", "HE-SU", "HE-MU", "HE-EXT",
         "HE-TB", "-", "-", "-", "-", "-", "-", "-"}},
    {SIWIFI_HE_AP, "Preamble", 11, 11, {"Short Preamble", "Long Preamble"}},
    {SIWIFI_HE_AP, "BW (Prot)", 10, 8, {"20MHz", "40MHz", "80MHz", "160MHz",
	     "80MHz (Punctured Primary)", "80 MHz (Punctured Secondary)",
	     "160MHz (Punctured Primary)", "160MHz (Punctured Secondary)"}},
    {SIWIFI_HE_AP, "MCS Index", 6, 0, {NULL}},
};

struct bitfield_desc mu_muphycntrlinfo1_desc[] = {
    {SIWIFI_HE_AP, "# HE-LTF Symbols", 26, 24, {NULL}},
    {SIWIFI_HE_AP, "Center 26T RU", 23, 22, {NULL}},
    {SIWIFI_HE_AP, "SigB Compr", 20, 20, {NULL}},
    {SIWIFI_HE_AP, "SigB DCM", 19, 19, {NULL}},
    {SIWIFI_HE_AP, "SigB MCS", 18, 16, {NULL}},
};

struct bitfield_desc mu_muphycntrlinfo2_desc[] = {
    {SIWIFI_HE_AP, "RU Alloc SigB3", 31, 24, {NULL}},
    {SIWIFI_HE_AP, "RU Alloc SigB2", 23, 16, {NULL}},
    {SIWIFI_HE_AP, "RU Alloc SigB1", 15, 8, {NULL}},
    {SIWIFI_HE_AP, "RU Alloc SigB0", 7, 0, {NULL}},
};

struct bitfield_desc mu_muphycntrlinfo3_desc[] = {
    {SIWIFI_HE_AP, "RU Alloc SigB7", 31, 24, {NULL}},
    {SIWIFI_HE_AP, "RU Alloc SigB6", 23, 16, {NULL}},
    {SIWIFI_HE_AP, "RU Alloc SigB5", 15, 8, {NULL}},
    {SIWIFI_HE_AP, "RU Alloc SigB4", 7, 0, {NULL}},
};

/*
 * FUNCTION DEFINITIONS
 *****************************************************************************************
 */
char bitfield_buf[256];

enum siwifi_hw_type siwifi_hw;

#define BITFIELD_VAL(start, end, val) (((val) >> (start)) & ((1 << (end - start + 1)) - 1))

static char *parse_bitfields(uint32_t val, const struct bitfield_desc *bt, int nb_elt)
{
    int res, index = 0, first = 1;
    char *buf = bitfield_buf;
    int buf_size = sizeof(bitfield_buf);

    while (index < nb_elt) {
        int field_val = BITFIELD_VAL(bt[index].startbit, bt[index].endbit, val);

        if (!(bt[index].hw & siwifi_hw))
            goto next;

        if (!first) {
            res = snprintf(buf, buf_size, "|");
            buf += res;
            buf_size -= res;
        } else {
            first = 0;
        }

        if (bt[index].val_name[0]) {
            res = snprintf(buf, buf_size, "%s", bt[index].val_name[field_val]);
        } else {
            res = snprintf(buf, buf_size, "%s=%d", bt[index].name, field_val);
        }

        buf += res;
        buf_size -= res;

      next:
        index++;
    }

    return bitfield_buf;
}

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#define __print_bitfields(val, desc) parse_bitfields(val, desc, ARRAY_SIZE(desc))

static ssize_t readline(char **lineptr, FILE *fp)
{
    *lineptr = NULL;
    char *line;
    size_t len = 0;
    ssize_t read;
    int i;

    read = getline(lineptr, &len, fp);

    if (read <= 0)
        return read;

    line = *lineptr;

    // Remove the end of line character
    if (line[read - 1] == '\n')
        line[read - 1] = '\0';

    // Keep only the characters up to a * or a [
    for (i = 0; i < strlen(line); i++)
    {
        if ((line[i] == '*') || (line[i] == '['))
        {
            line[i] = '\0';
            break;
        }
    }

    return strlen(line);
}

static void get_hw(char *path)
{
    FILE *fp;
    char filename[256];
    char *line;
    ssize_t line_size;

    /* Default value, for HE hw we should always have siwifiplat file */
    siwifi_hw = SIWIFI_HE;

    snprintf(filename, sizeof(filename) - 1, "%s/siwifiplat", path);
    fp = fopen(filename, "r");
    if (fp == NULL)
        return;

    line_size = readline(&line, fp);
    while (line_size >= 0)
    {
        if (strncmp(line, "HE_AP", 5) == 0)
            siwifi_hw = SIWIFI_HE_AP;
        else if (strncmp(line, "HE", 2) == 0)
            siwifi_hw = SIWIFI_HE;
        else if (strncmp(line, "NX", 2) == 0)
            siwifi_hw = SIWIFI_NX;

        line_size = readline(&line, fp);
    }

    fclose(fp);
}

static int get_rxdesc_pointers(char *path, char **rhd, char **rbd, uint32_t *hwrhd, uint32_t *hwrbd)
{
    FILE *fp;
    char filename[256];
    ssize_t read;

    printf("Get RX descriptor pointers\n");

    snprintf(filename, sizeof(filename) - 1, "%s/rxdesc", path);
    fp = fopen(filename, "r");
    if (fp == NULL)
        return -1;

    // Read the RX descriptor pointers
    read = readline(rhd, fp);
    if (read != 8)
        return -1;

    read = readline(rbd, fp);
    if (read != 8)
        return -1;

    fclose(fp);

    snprintf(filename, sizeof(filename) - 1, "%s/macrxptr", path);
    fp = fopen(filename, "r");
    if (fp == NULL)
        return 0;

    // Read the RX descriptor pointers
    do
    {
        if (fread(hwrhd, sizeof(*hwrhd), 1, fp) != 1)
        {
            *hwrhd = 0xFFFFFFFF;
            break;
        }

        if (fread(hwrbd, sizeof(*hwrbd), 1, fp) != 1)
        {
            *hwrbd = 0xFFFFFFFF;
            break;
        }
    } while(0);

    fclose(fp);

    return 0;
}

static int get_txdesc_pointers(char *path, char *thd[])
{
    FILE *fp;
    char filename[256];
    ssize_t read;
    int i;

    printf("Get TX descriptor pointers\n");

    snprintf(filename, sizeof(filename) - 1, "%s/txdesc", path);
    fp = fopen(filename, "r");
    if (fp == NULL)
        return -1;

    // Read the TX descriptor pointers
    for (i = 0; i < 5; i++)
    {
        read = readline(&thd[i], fp);
        if (read != 8)
            break;
    }

    fclose(fp);

    return i;
}

static int gen_rhd(char *path, char *first, uint32_t hwrhd)
{
    FILE *in;
    FILE *out;
    char filename[256];
    int i = 1;
    struct rx_hd rhd;
    uint32_t ptr;
    bool hwrhd_found = false;

    if (sscanf(first, "%08x", &ptr) != 1)
        ptr = 0;

    printf("Generate RHD file\n");

    snprintf(filename, sizeof(filename) - 1, "%s/rhd", path);
    in = fopen(filename, "r");

    if (in == NULL)
        return -1;

    snprintf(filename, sizeof(filename) - 1, "%s/rhd.txt", path);
    out = fopen(filename, "w");

    if (out == NULL)
        return -1;

    fprintf(out, "RX Header Descriptor list\n\n");

    while (1)
    {
        if (fread(&rhd, sizeof(rhd), 1, in) != 1)
            break;
        fprintf(out, "RHD#%d @ 0x%08x", i, ptr);

        if (hwrhd != 0xFFFFFFFF)
        {
            if (i == 1)
                fprintf(out, "    <--- Spare RHD");
            else if (i == 2)
                fprintf(out, "    <--- First RHD for SW");

            if (ptr == hwrhd)
            {
                hwrhd_found = true;
                if (i < 3)
                    fprintf(out, " - Current RHD for HW");
                else
                    fprintf(out, "    <--- Current RHD for HW");
            }
        }

        fprintf(out, "\n~~~~~~~~~~~~~~~~~~~\n");
        if (rhd.upatternrx != 0xBAADF00D)
            fprintf(out, "UNIQUE PATTERN ERROR!!!!\n");

        fprintf(out, "  Unique pattern                 = 0x%08x\n", rhd.upatternrx);
        fprintf(out, "  Next Header Descriptor Pointer = 0x%08x\n", rhd.next);
        fprintf(out, "  First Payload Buffer Pointer   = 0x%08x\n", rhd.first_pbd_ptr);
        fprintf(out, "  Software Descriptor Pointer    = 0x%08x\n", rhd.swdesc);
        fprintf(out, "  Data Start Pointer             = 0x%08x\n", rhd.datastartptr);
        fprintf(out, "  Data End Pointer               = 0x%08x\n", rhd.dataendptr);
        fprintf(out, "  Header Control Info            = 0x%08x\n", rhd.headerctrlinfo);
        fprintf(out, "  Frame Length                   = 0x%08x\n", rhd.frmlen);
        fprintf(out, "  TSF Low                        = 0x%08x\n", rhd.tsflo);
        fprintf(out, "  TSF High                       = 0x%08x\n", rhd.tsfhi);
        fprintf(out, "  Rx Vector 1a                   = 0x%08x\n", rhd.recvec1a);
        fprintf(out, "  Rx Vector 1b                   = 0x%08x\n", rhd.recvec1b);
        fprintf(out, "  Rx Vector 1c                   = 0x%08x\n", rhd.recvec1c);
        fprintf(out, "  Rx Vector 1d                   = 0x%08x\n", rhd.recvec1d);
        fprintf(out, "  Rx Vector 2a                   = 0x%08x\n", rhd.recvec1d);
        fprintf(out, "  Rx Vector 2b                   = 0x%08x\n", rhd.recvec2a);
        fprintf(out, "  Status Information             = 0x%08x\n\n\n", rhd.statinfo);

        ptr = rhd.next;

        i++;
    }

    fprintf(out, "\nEnd of list\n");
    if ((hwrhd != 0xFFFFFFFF) && !hwrhd_found)
        fprintf(out, "\n!!!! MAC HW is pointing to unknown RHD@0x%08X !!!!\n", hwrhd);

    fclose(in);
    fclose(out);

    return 0;
}

static int gen_rbd(char *path, char *first, uint32_t hwrbd)
{
    FILE *in;
    FILE *out;
    char filename[256];
    int i = 1;
    struct rx_bd rbd;
    uint32_t ptr;
    bool hwrbd_found = false;

    if (sscanf(first, "%08x", &ptr) != 1)
        ptr = 0;

    printf("Generate RBD file\n");

    snprintf(filename, sizeof(filename) - 1, "%s/rbd", path);
    in = fopen(filename, "r");

    if (in == NULL)
        return -1;

    snprintf(filename, sizeof(filename) - 1, "%s/rbd.txt", path);
    out = fopen(filename, "w");

    if (out == NULL)
        return -1;

    fprintf(out, "RX Buffer Descriptor list\n\n");

    while (1)
    {
        if (fread(&rbd, sizeof(rbd), 1, in) != 1)
            break;
        fprintf(out, "RBD#%d @ 0x%08x", i, ptr);

        if (hwrbd != 0xFFFFFFFF)
        {
            if (i == 1)
                fprintf(out, "    <--- Spare RBD");
            else if (i == 2)
                fprintf(out, "    <--- First RBD for SW");

            if (ptr == hwrbd)
            {
                hwrbd_found = true;
                if (i < 3)
                    fprintf(out, " - Current RBD for HW");
                else
                    fprintf(out, "    <--- Current RBD for HW");
            }
        }

        fprintf(out, "\n~~~~~~~~~~~~~~~~~~~\n");
        if (rbd.upattern != 0xC0DEDBAD)
            fprintf(out, "UNIQUE PATTERN ERROR!!!!\n");

        fprintf(out, "  Unique pattern                 = 0x%08x\n", rbd.upattern);
        fprintf(out, "  Next Buffer Descriptor Pointer = 0x%08x\n", rbd.next);
        fprintf(out, "  Data Start Pointer             = 0x%08x\n", rbd.datastartptr);
        fprintf(out, "  Data End Pointer               = 0x%08x\n", rbd.dataendptr);
        fprintf(out, "  Buffer Status Info             = 0x%08x\n\n\n", rbd.bufstatinfo);

        ptr = rbd.next;

        i++;
    }

    fprintf(out, "\nEnd of list\n");
    if ((hwrbd != 0xFFFFFFFF) && !hwrbd_found)
        fprintf(out, "\n!!!! MAC HW is pointing to unknown RBD@0x%08X !!!!\n", hwrbd);

    fclose(in);
    fclose(out);

    return 0;
}

static int mu_mimo_ampdu_header(uint32_t maccntrlinfo2, uint32_t phyctrlinfo)
{
    int group;
    int mumimo;
    int which_desc;

    if (siwifi_hw != SIWIFI_NX)
        return 0;

    which_desc = BITFIELD_VAL(19, 21, maccntrlinfo2);
    mumimo = BITFIELD_VAL(15, 15, phyctrlinfo);

    if ((which_desc != 4) || (mumimo ==0))
        return 0;

    group = BITFIELD_VAL(16, 21, phyctrlinfo);
    if ((group > 0) && (group < 63))
        return 1;

    return 2;
}

static int parse_thd(FILE *in, FILE *out, int i)
{
    struct tx_hd thd;
    struct tx_pbd tbd;
    struct tx_policy_tbl pol;
    uint32_t tbd_ptr;
    int mumimo_header;

    if (fread(&thd, sizeof(thd), 1, in) != 1)
        return 1;

    mumimo_header = mu_mimo_ampdu_header(thd.macctrlinfo2, thd.phyctrlinfo);
    fprintf(out, "TX Header Descriptor n=%d\n", i);
    fprintf(out, "~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    if (thd.upatterntx != 0xCAFEBABE)
        fprintf(out, "UNIQUE PATTERN ERROR!!!!\n");

    fprintf(out, "  Unique pattern               = 0x%08x\n", thd.upatterntx);
    fprintf(out, "  Next Atomic Pointer          = 0x%08x\n", thd.nextfrmexseq_ptr);
    fprintf(out, "  Next MPDU Descriptor Pointer = 0x%08x\n", thd.nextmpdudesc_ptr);
    if (mumimo_header == 1)
    {
        fprintf(out, "  User 1, A-MPDU Descriptor    = 0x%08x\n", thd.first_pbd_ptr);
        fprintf(out, "  User 2, A-MPDU Descriptor    = 0x%08x\n", thd.datastartptr);
        fprintf(out, "  User 3, A-MPDU Descriptor    = 0x%08x\n", thd.dataendptr);
        thd.first_pbd_ptr = 0;
    }
    else
    {
        fprintf(out, "  First Payload Buffer Pointer = 0x%08x\n", thd.first_pbd_ptr);
        fprintf(out, "  Data Start Pointer           = 0x%08x\n", thd.datastartptr);
        fprintf(out, "  Data End Pointer             = 0x%08x\n", thd.dataendptr);
    }
    fprintf(out, "  Frame Length                 = 0x%08x\n", thd.frmlen);
    fprintf(out, "  Frame Lifetime               = 0x%08x\n", thd.frmlifetime);
    fprintf(out, "  Phy Control Info             = 0x%08x (%s)\n", thd.phyctrlinfo,
            __print_bitfields(thd.phyctrlinfo, phyctrlinfo_desc));
    fprintf(out, "  Policy Entry Address         = 0x%08x\n", thd.policyentryaddr);
    fprintf(out, "  Optional 20MHz Length        = 0x%08x\n", thd.opt20mhzlen);
    fprintf(out, "  Optional 40MHz Length        = 0x%08x\n", thd.opt40mhzlen);
    fprintf(out, "  Optional 80MHz Length        = 0x%08x\n", thd.opt80mhzlen);
    fprintf(out, "  MAC Control Information 1    = 0x%08x (%s)\n", thd.macctrlinfo1,
            __print_bitfields(thd.macctrlinfo1, macctrlinfo1_desc));
    fprintf(out, "  MAC Control Information 2    = 0x%08x (%s)\n", thd.macctrlinfo2,
            __print_bitfields(thd.macctrlinfo2, macctrlinfo2_desc));
    fprintf(out, "  Status Information           = 0x%08x (%s)\n", thd.statinfo,
            __print_bitfields(thd.statinfo, statinfo_desc));
    fprintf(out, "  Medium Time Used             = 0x%08x\n\n", thd.mediumtimeused);

    // Check if we have to display the policy table
    if (!((thd.macctrlinfo2 & 0x00200000) &&
          (thd.macctrlinfo2 & 0x00180000)))
    {
        if (fread(&pol, sizeof(pol), 1, in) != 1)
            return 1;

        fprintf(out, "  Policy table\n");
        fprintf(out, "  ~~~~~~~~~~~~\n");
        if (pol.upatterntx != 0xBADCAB1E)
            fprintf(out, "UNIQUE PATTERN ERROR!!!!\n");

        fprintf(out, "    Unique pattern                      = 0x%08x\n", pol.upatterntx);
        if (mumimo_header == 2)
        {
            fprintf(out, "    Secondary User control Info         = 0x%08x (%s)\n", pol.phycntrlinfo1,
                    __print_bitfields(pol.phycntrlinfo1, sec_user_control_desc));
        }
        else
        {
            fprintf(out, "    P-Table PHY Control Info 1          = 0x%08x (%s)\n", pol.phycntrlinfo1,
                    __print_bitfields(pol.phycntrlinfo1, phycntrlinfo1_desc));
            fprintf(out, "    P-Table PHY Control Info 2          = 0x%08x (%s)\n", pol.phycntrlinfo2,
                    __print_bitfields(pol.phycntrlinfo2, phycntrlinfo2_desc));
            fprintf(out, "    P-Table MAC Control Info 1          = 0x%08x (%s)\n", pol.maccntrlinfo1,
                    __print_bitfields(pol.maccntrlinfo1, maccntrlinfo1_desc));
            fprintf(out, "    P-Table MAC Control Info 2          = 0x%08x (%s)\n", pol.maccntrlinfo2,
                    __print_bitfields(pol.maccntrlinfo2, maccntrlinfo2_desc));
            fprintf(out, "    P-Table Rate Control Information 1  = 0x%08x (%s)\n", pol.ratecntrlinfo[0],
                    __print_bitfields(pol.ratecntrlinfo[0], ratecntrlinfo1_desc));
            fprintf(out, "    P-Table Rate Control Information 2  = 0x%08x (%s)\n", pol.ratecntrlinfo[1],
                    __print_bitfields(pol.ratecntrlinfo[1], ratecntrlinfo1_desc));
            fprintf(out, "    P-Table Rate Control Information 3  = 0x%08x (%s)\n", pol.ratecntrlinfo[2],
                    __print_bitfields(pol.ratecntrlinfo[2], ratecntrlinfo1_desc));
            fprintf(out, "    P-Table Rate Control Information 4  = 0x%08x (%s)\n", pol.ratecntrlinfo[3],
                    __print_bitfields(pol.ratecntrlinfo[3], ratecntrlinfo1_desc));
            fprintf(out, "    P-Table Power Control Information 1 = 0x%08x (%s)\n", pol.powercntrlinfo[0],
                    __print_bitfields(pol.powercntrlinfo[0], powercntrlinfo_desc));
            fprintf(out, "    P-Table Power Control Information 2 = 0x%08x (%s)\n", pol.powercntrlinfo[1],
                    __print_bitfields(pol.powercntrlinfo[1], powercntrlinfo_desc));
            fprintf(out, "    P-Table Power Control Information 3 = 0x%08x (%s)\n", pol.powercntrlinfo[2],
                    __print_bitfields(pol.powercntrlinfo[2], powercntrlinfo_desc));
            fprintf(out, "    P-Table Power Control Information 4 = 0x%08x (%s)\n\n", pol.powercntrlinfo[3],
                    __print_bitfields(pol.powercntrlinfo[3], powercntrlinfo_desc));
        }
    }

    // Display the list of TBD attached to this THD
    tbd_ptr = thd.first_pbd_ptr;
    while (tbd_ptr != 0)
    {
        if (fread(&tbd, sizeof(tbd), 1, in) != 1)
            return 1;

        fprintf(out, "  TX Buffer Descriptor\n");
        fprintf(out, "  ~~~~~~~~~~~~~~~~~~~~\n");
        if (tbd.upatterntx == 0xCAFEBABE) {
            /* fw didn't upload buffer desc and directly jumped to next tx desc */
            fprintf(out, "  Not uploaded as not in SHARED RAM\n");
            fseek(in, -sizeof(tbd), SEEK_CUR);
            return 0;
        } else if (tbd.upatterntx != 0xCAFEFADE)
            fprintf(out, "UNIQUE PATTERN ERROR!!!!\n");

        fprintf(out, "    Unique pattern               = 0x%08x\n", tbd.upatterntx);
        fprintf(out, "    Next Buf Descriptor Pointer  = 0x%08x\n", tbd.next);
        fprintf(out, "    Data Start Pointer           = 0x%08x\n", tbd.datastartptr);
        fprintf(out, "    Data End Pointer             = 0x%08x\n", tbd.dataendptr);
        fprintf(out, "    Buffer Control Information   = 0x%08x (%s)\n\n", tbd.bufctrlinfo,
                __print_bitfields(tbd.bufctrlinfo, bufctrlinfo_desc));

        tbd_ptr = tbd.next;
    }
    fprintf(out, "\n");

    return 0;
}

static int parse_thd_he_ap(FILE *in, FILE *out, int fthd_idx)
{
    struct tx_frmx_hd fthd;
    struct tx_policy_tbl_he_ap pol;
    uint32_t next_mthd;
    uint32_t next_fthd;
    size_t pol_size;
    bool first = true;
    bool mu_tx = false;
    int user_idx = 0;

    if (fread(&fthd, sizeof(fthd), 1, in) != 1)
        return 1;

    fprintf(out, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    fprintf(out, "                                   PPDU #%d\n", fthd_idx);
    fprintf(out, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    do {
        int mpdu_idx = 0;

        fprintf(out, "\nUser Frame Descriptor %d.%d\n", fthd_idx, user_idx);
        fprintf(out, "~~~~~~~~~~~~~~~~~~~~~~~~~\n");
        if (fthd.upatterntx != 0xCAFEBABE)
            fprintf(out, "UNIQUE PATTERN ERROR!!!!\n");

        fprintf(out, "  Unique pattern               = 0x%08x\n", fthd.upatterntx);
        fprintf(out, "  Next Atomic Pointer          = 0x%08x\n", fthd.nextfrmexseq_ptr);
        fprintf(out, "  Next Frame Desc Pointer      = 0x%08x\n", fthd.nextfrm_ptr);
        fprintf(out, "  Next User Frame Desc Pointer = 0x%08x\n", fthd.nextuserfrmex_ptr);
        fprintf(out, "  BAR Frame Desc Pointer       = 0x%08x\n", fthd.barfrmex_ptr);
        fprintf(out, "  Policy Entry Address         = 0x%08x\n", fthd.policyentryaddr);
        fprintf(out, "  Firt MDPU Desc pointer       = 0x%08x\n", fthd.firstmpdudesc_ptr);
        fprintf(out, "  Frame Length                 = 0x%08x\n", fthd.frmlen);
        fprintf(out, "  Frame Lifetime               = 0x%08x\n", fthd.frmlifetime);
        fprintf(out, "  Phy Control Info 1           = 0x%08x (%s)\n", fthd.phyctrlinfo1,
                __print_bitfields(fthd.phyctrlinfo1, phyctrlinfo_desc));
        fprintf(out, "  Phy Control Info 2           = 0x%08x (%s)\n", fthd.phyctrlinfo2,
                __print_bitfields(fthd.phyctrlinfo2, phyctrlinfo2_desc));
        fprintf(out, "  MAC Control Info 1           = 0x%08x (%s)\n", fthd.macctrlinfo1,
                __print_bitfields(fthd.macctrlinfo1, macctrlinfo1_desc));
        fprintf(out, "  Status Information           = 0x%08x (%s)\n", fthd.frmxstatinfo,
                __print_bitfields(fthd.frmxstatinfo, statinfo_desc));
        fprintf(out, "  Medium Time Used             = 0x%08x\n", fthd.mediumtimeused);

        next_mthd = fthd.firstmpdudesc_ptr;
        next_fthd = fthd.nextuserfrmex_ptr;

        if (first) {
            if (next_fthd) {
                pol_size = sizeof(struct tx_mu_policy_tbl_he_ap);
                mu_tx = true;
            } else
                pol_size = sizeof(struct tx_su_policy_tbl_he_ap);

            first = false;
        } else {
            pol_size = sizeof(struct tx_pu_policy_tbl_he_ap);
        }

        if (fread(&pol, pol_size, 1, in) != 1)
            return 1;

        fprintf(out, "\n  Policy table \n");
        fprintf(out, "  ~~~~~~~~~~~~\n");

        if (mu_tx) {
            if (pol_size == sizeof(struct tx_mu_policy_tbl_he_ap)) {
                struct tx_mu_policy_tbl_he_ap *mu = &pol.mu;

                if (mu->upatterntx != 0xFA7CAB1E)
                    fprintf(out, "UNIQUE PATTERN ERROR!!!!\n");

                fprintf(out, "    Unique pattern                   = 0x%08x\n", mu->upatterntx);
                fprintf(out, "    PHY Control Info 1               = 0x%08x (%s)\n", mu->phycntrlinfo1,
                        __print_bitfields(mu->phycntrlinfo1, mu_phycntrlinfo1_desc));
                fprintf(out, "    PHY Control Info 2               = 0x%08x (%s)\n", mu->phycntrlinfo2,
                        __print_bitfields(mu->phycntrlinfo2, mu_phycntrlinfo2_desc));
                fprintf(out, "    User PHY Control Info            = 0x%08x (%s)\n", mu->userphycntrlinfo,
                        __print_bitfields(mu->userphycntrlinfo, mu_userphycntrlinfo_desc));
                fprintf(out, "    User MAC Control Info            = 0x%08x (%s)\n", mu->usermaccntrlinfo,
                        __print_bitfields(mu->usermaccntrlinfo, mu_usermaccntrlinfo_desc));
                fprintf(out, "    MU Rate Control Info             = 0x%08x (%s)\n", mu->muratecntrlinfo,
                        __print_bitfields(mu->muratecntrlinfo, mu_muratecntrlinfo_desc));
                fprintf(out, "    MU Prot Rate Control Info        = 0x%08x (%s)\n", mu->muprotratecntrlinfo,
                        __print_bitfields(mu->muprotratecntrlinfo, mu_muprotratecntrlinfo_desc));
                fprintf(out, "    MU PHY Control Info 1            = 0x%08x (%s)\n", mu->muphycntrlinfo1,
                        __print_bitfields(mu->muphycntrlinfo1, mu_muphycntrlinfo1_desc));
                fprintf(out, "    MU PHY Control Info 2            = 0x%08x (%s)\n", mu->muphycntrlinfo2,
                        __print_bitfields(mu->muphycntrlinfo2, mu_muphycntrlinfo2_desc));
                fprintf(out, "    MU PHY Control Info 3            = 0x%08x (%s)\n", mu->muphycntrlinfo3,
                        __print_bitfields(mu->muphycntrlinfo3, mu_muphycntrlinfo3_desc));
            } else {
                struct tx_pu_policy_tbl_he_ap *pu = &pol.pu;

                if (pu->upatterntx != 0x5ADCAB1E)
                    fprintf(out, "UNIQUE PATTERN ERROR!!!!\n");

                fprintf(out, "    Unique pattern                   = 0x%08x\n", pu->upatterntx);
                fprintf(out, "    Per-User PHY Control Info        = 0x%08x (%s)\n", pu->per_user_phyinfo,
                        __print_bitfields(pu->per_user_phyinfo, per_user_phyinfo_desc));
                fprintf(out, "    Per-User MAC Control Info        = 0x%08x (%s)\n", pu->per_user_macinfo,
                        __print_bitfields(pu->per_user_macinfo, per_user_macinfo_desc));
            }
        } else {
            struct tx_su_policy_tbl_he_ap *su = &pol.su;

            if (su->upatterntx != 0xBADCAB1E)
                fprintf(out, "UNIQUE PATTERN ERROR!!!!\n");

            fprintf(out, "    Unique pattern                   = 0x%08x\n", su->upatterntx);
            fprintf(out, "    PHY Control Info 1               = 0x%08x (%s)\n", su->phycntrlinfo1,
                    __print_bitfields(su->phycntrlinfo1, phycntrlinfo1_desc));
            fprintf(out, "    PHY Control Info 2               = 0x%08x (%s)\n", su->phycntrlinfo2,
                    __print_bitfields(su->phycntrlinfo2, phycntrlinfo2_desc));
            fprintf(out, "    MAC Control Info 1               = 0x%08x (%s)\n", su->maccntrlinfo1,
                    __print_bitfields(su->maccntrlinfo1, maccntrlinfo1_desc));
            fprintf(out, "    MAC Control Info 2               = 0x%08x (%s)\n", su->maccntrlinfo2,
                    __print_bitfields(su->maccntrlinfo2, maccntrlinfo2_desc));

            for (int j = 0; j < 4 ; j++) {
                fprintf(out, "    Rate[%d] Control Information      = 0x%08x (%s)\n", j, su->ratecntrlinfo[j][0],
                        __print_bitfields(su->ratecntrlinfo[j][0], ratecntrlinfo1_desc));
                fprintf(out, "    Rate[%d] Prot Control Information = 0x%08x (%s)\n", j, su->ratecntrlinfo[j][1],
                        __print_bitfields(su->ratecntrlinfo[j][1], ratecntrlinfo2_desc));
            }
        }

        while (next_mthd) {
            struct tx_mpdu_hd mthd;
            uint32_t next_tbd;
            int tbd_idx = 0;

            if (fread(&mthd, sizeof(mthd), 1, in) != 1)
                return 1;

            mpdu_idx++;
            next_mthd = mthd.nextmpdudesc_ptr;
            fprintf(out, "\n  MPDU Descriptor %d.%d.%d\n", fthd_idx, user_idx, mpdu_idx);
            fprintf(out, "  ~~~~~~~~~~~~~~~~~~~~~\n");
            if (mthd.upatterntx != 0xCAFEB0B0)
                fprintf(out, "UNIQUE PATTERN ERROR!!!!\n");

            fprintf(out, "    Unique pattern          = 0x%08x\n", mthd.upatterntx);
            fprintf(out, "    Next MPDU Desc pointer  = 0x%08x\n", mthd.nextmpdudesc_ptr);
            fprintf(out, "    First Buf Desc pointer  = 0x%08x\n", mthd.first_pbd_ptr);
            fprintf(out, "    Data Start pointer      = 0x%08x\n", mthd.datastartptr);
            fprintf(out, "    Data End pointer        = 0x%08x\n", mthd.dataendptr);
            fprintf(out, "    MDPU Length             = 0x%08x\n", mthd.frmlen);
            fprintf(out, "    MAC Control Info 2      = 0x%08x (%s)\n", mthd.macctrlinfo2,
                    __print_bitfields(mthd.macctrlinfo2, macctrlinfo2_desc));
            fprintf(out, "    Status Info             = 0x%08x (%s)\n", mthd.mpdustatinfo,
                    __print_bitfields(mthd.mpdustatinfo, mpdustatinfo_desc));

            // Display the list of TBD attached to this THD
            next_tbd = mthd.first_pbd_ptr;
            while (next_tbd)
            {
                struct tx_pbd tbd;

                if (fread(&tbd, sizeof(tbd), 1, in) != 1)
                    return 1;

                tbd_idx++;
                fprintf(out, "\n    TX Buffer Descriptor %d.%d.%d.%d\n", fthd_idx, user_idx, mpdu_idx, tbd_idx);
                fprintf(out, "    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
                if (
                    (next_mthd && tbd.upatterntx == 0xCAFEB0B0)
                    || ((!next_mthd) && tbd.upatterntx == 0xCAFEBABE)
                ) {
                    /* fw didn't upload buffer desc and directly jumped to next tx desc */
                    fprintf(out, "    Not uploaded as not in SHARED RAM\n");
                    fseek(in, -sizeof(tbd), SEEK_CUR);
                    break;
                } else if (tbd.upatterntx != 0xCAFEFADE)
                    fprintf(out, "UNIQUE PATTERN ERROR!!!!\n");

                fprintf(out, "        Unique pattern               = 0x%08x\n", tbd.upatterntx);
                fprintf(out, "        Next Buf Descriptor Pointer  = 0x%08x\n", tbd.next);
                fprintf(out, "        Data Start Pointer           = 0x%08x\n", tbd.datastartptr);
                fprintf(out, "        Data End Pointer             = 0x%08x\n", tbd.dataendptr);
                fprintf(out, "        Buffer Control Information   = 0x%08x (%s)\n\n", tbd.bufctrlinfo,
                        __print_bitfields(tbd.bufctrlinfo, bufctrlinfo_desc));

                next_tbd = tbd.next;
            }
        }
        user_idx++;
        if (next_fthd) {
            if (fread(&fthd, sizeof(fthd), 1, in) != 1)
                return 1;
        }
    } while (next_fthd);

    fprintf(out, "\n");

    return 0;
}


static int gen_thd(char *path, char *first, int idx)
{
    FILE *in;
    FILE *out;
    char filename[256];
    int i = 1;

    printf("Generate THD file %d\n", idx);

    snprintf(filename, sizeof(filename) - 1, "%s/thd%d", path, idx);
    in = fopen(filename, "r");

    if (in == NULL)
        return -1;

    snprintf(filename, sizeof(filename) - 1, "%s/thd%d.txt", path, idx);
    out = fopen(filename, "w");

    if (out == NULL)
        return -1;

    fprintf(out, "TX Header Head Pointer = 0x%s\n\n", first);

    while (1)
    {
        int res;

        if (siwifi_hw & (SIWIFI_NX | SIWIFI_HE))
            res = parse_thd(in, out, i);
        else
            res = parse_thd_he_ap(in, out, i);

        if (res)
            break;

        i++;
    }

    fprintf(out, "\nEnd of list\n");

    fclose(in);
    fclose(out);

    return 0;
}

/*
 *****************************************************************************************
 * @brief Main entry point of the application.
 *
 * @param argc   usual parameter counter
 * @param argv   usual parameter values
 *****************************************************************************************
 */
int main(int argc, char **argv)
{
    char *first_rhd, *first_rbd;
    uint32_t cur_rhd = 0xFFFFFFFF, cur_rbd = 0xFFFFFFFF;
    char *thd[5];
    int txcnt;
    int i;

    if (argc < 2) {
        fprintf(stderr, "usage: %s <dir>\n<dir> must contains the follwing files.\n"
                "For RX:\n"
                " - rxdesc : address of RX header and buffer descriptor\n"
                " - rbd    : binary dump of RX buffer descriptor\n"
                " - rhd    : binary dump of RX header descriptor\n"
                "For TX:\n"
                " - txdesc    : address of TX descriptor for each queue\n"
                " - thd{0..4} : binary dump of TX descriptor for the queue\n"
                ,
                argv[0]);
        return -1;
    }

    /* skip progname */
    argv++;

    get_hw(argv[0]);

    if (get_rxdesc_pointers(argv[0], &first_rhd, &first_rbd, &cur_rhd, &cur_rbd)) {
        fprintf(stderr, "Failed to read 'rxdesc'\n");
        return -1;
    }

    txcnt = get_txdesc_pointers(argv[0], thd);
    if (txcnt < 0) {
        fprintf(stderr, "Failed to read 'txdesc'\n");
        return -1;
    }

    if (gen_rhd(argv[0], first_rhd, cur_rhd)) {
        fprintf(stderr, "Failed to read 'rhd'\n");
        return -1;
    }

    if (gen_rbd(argv[0], first_rbd, cur_rbd)) {
        fprintf(stderr, "Failed to read 'rbd'\n");
        return -1;
    }

    for (i = 0; i < txcnt; i++)
    {
        if (gen_thd(argv[0], thd[i], i)) {
            fprintf(stderr, "Failed to read 'thd%d'\n", i);
            return -1;
        }
    }

    free(first_rhd);
    free(first_rbd);

    return 0;
}
