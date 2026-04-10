/*
 * =====================================================================================
 *
 *       Filename:  main.c
 *
 *    Description:  call the wireless extension' cmd to make clear how the drivers(mac80211) support
 *                  wext
 *
 *        Version:  1.0
 *        Created:  02/22/2016 03:56:43 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  franklin , franklin.wang@siflower.com.cn
 *        Company:  Siflower Communication Tenology Co.,Ltd
 *
 * =====================================================================================
 */

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <sys/socket.h>
#include <linux/wireless.h>
#include <sys/time.h>
#include <stdint.h>
#include <dirent.h>
#include <libubi.h>
#include <errno.h>
#include <stdbool.h>
//#include <getopt.h>
//#define IFNAME "wlan0"

#ifdef NAND_SUPPORT
#define UBI_FACTORY "/dev/ubi0_1"
#define CAL_MAX_LEN 16384

static libubi_t libubi;
static char data_buf[CAL_MAX_LEN];
#endif

char ifname[16] = {"wlan0"};

char beacon[192] = {
	0x80, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x16, 0x88, 0x08, 0x07, 0xf9,
	0x00, 0x16, 0x88, 0x08, 0x07, 0xf9, 0x20, 0x5e, 0x5f, 0x20, 0x31, 0x3b, 0x00, 0x00, 0x00, 0x00,
	0x64, 0x00, 0x01, 0x00, 0x00, 0x06, 0x6f, 0x70, 0x65, 0x6e, 0x35, 0x67, 0x01, 0x08, 0x8c, 0x12,
	0x98, 0x24, 0xb0, 0x48, 0x60, 0x6c, 0x03, 0x01, 0x9d, 0x07, 0x06, 0x4e, 0x6f, 0x20, 0x24, 0x0d,
	0x14, 0x05, 0x04, 0x00, 0x01, 0x00, 0x00, 0x2d, 0x1a, 0xec, 0x01, 0x17, 0xff, 0xff, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x3d, 0x16, 0x9d, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xbf, 0x0c, 0xa0, 0x01, 0xc0,
	0x31, 0xfa, 0xff, 0x0c, 0x03, 0xfa, 0xff, 0x0c, 0x03, 0xc0, 0x05, 0x00, 0x00, 0x00, 0xf5, 0xff,
	0x7f, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0xdd, 0x18, 0x00, 0x50, 0xf2, 0x02,
	0x01, 0x01, 0x00, 0x00, 0x03, 0xa4, 0x00, 0x00, 0x27, 0xa4, 0x00, 0x00, 0x42, 0x43, 0x5e, 0x00,
	0x62, 0x32, 0x2f, 0x00, 0xdd, 0x07, 0x00, 0x0c, 0x43, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

#define IS_24G 1
#define ETH_P_SFCFG 0x1688
#define SFCFG_MAGIC_NO      0x18181688
#define FAC_MODE 200
#define NOR_MODE 201
#define SLE_MODE 202
#define TMP_MODE 203
#define ANT1 1
#define ANT2 2
#define AX80CALI 0

/*calibration size*/
#define CALI_TABLE_2G_SIZE 676
#define CALI_TABLE_5G_A_N_AC_SIZE 1325
#define CALI_TABLE_5G_AX_160_SIZE 768
#define CALI_TABLE_24G_CH_SIZE 52
#define CALI_TABLE_5G_CH_SIZE 101
#define CALI_TABLE_5G_ANAC_CH_SIZE 53
#define CALI_TABLE_ONE_ANT 3201
#define CALI_TABLE_WHOLE 6402
#define CALI_TABLE_2G_LINE 13
#define CALI_TABLE_5G_LINE 25
#define CALI_TABLE_MAX_GAIN 127
#define CALI_TABLE_MIN_GAIN 0

/*  command id */
#define SFCFG_CMD_ATE_START				 0x0000
#define SFCFG_CMD_ATE_STOP				 0x0001
#define SFCFG_CMD_ATE_TX_START			 0x0002
#define SFCFG_CMD_ATE_TX_STOP			 0x0003
#define SFCFG_CMD_ATE_RX_START			 0x0002
#define SFCFG_CMD_ATE_RX_STOP			 0x0003
#define SFCFG_CMD_ATE_TX_FRAME_START	 0x0006
#define SFCFG_CMD_ATE_RX_FRAME_START	 0x0007
#define SFCFG_CMD_ATE_MACBYPASS_TX_START 0x0004
#define SFCFG_CMD_ATE_MACBYPASS_TX_STOP  0x0005
#define SFCFG_CMD_ATE_TX_TEST_TONE_START 0x000a
#define SFCFG_CMD_ATE_TX_TEST_TONE_STOP  0x000b
#define SFCFG_CMD_ATE_MACBYPASS_RX_START 0x000c
#define SFCFG_CMD_ATE_MACBYPASS_RX_STOP  0x000d
#define SFCFG_CMD_ATE_MACBYPASS_TX_RX_START 0x0006
#define SFCFG_CMD_ATE_MACBYPASS_TX_RX_STOP  0x0007

//define 0x20  command_type set tx vector
//define command_id by command_tpye and command_idx
//command_idx  is index in txvector in rwnx_ioctl.c
#define SFCFG_CMD_ATE_TXVECTOR_PARAM 0x0020
#define SFCFG_CMD_ATE_TXVECTOR_PARAM_NSS 0x0021

#define SFCFG_CMD_ATE_SET_BANDWIDTH		    0x0100
#define SFCFG_CMD_ATE_SET_CHANNEL		    0x0101
#define SFCFG_CMD_ATE_SET_PHY_MODE		    0x0102
#define SFCFG_CMD_ATE_SET_RATE			    0x0103
#define SFCFG_CMD_ATE_SET_TX_POWER		    0x0104
#define SFCFG_CMD_ATE_SET_PREAMBLE		    0x0105
#define SFCFG_CMD_ATE_SET_GI			    0x0106
#define SFCFG_CMD_ATE_GET_INFO			    0x0107
#define SFCFG_CMD_ATE_SAVE_TO_FLASH		    0x0108
#define SFCFG_CMD_ATE_SET_CENTER_FREQ1	    0x0109
#define SFCFG_CMD_ATE_READ_FROM_FLASH       0X010b
#define SFCFG_CMD_ATE_SAVE_DATA_TO_FLASH    0X010c
#define SFCFG_CMD_ATE_GET_DATA_FROM_FLASH   0x010d
#define SFCFG_CMD_ATE_SET_NSS 			    0x0111
#define SFCFG_CMD_ATE_SET_TX_FILTER         0x0112

#define SFCFG_CMD_ATE_GET_POWER 0x0110
#define SFCFG_CMD_ATE_WHOLE_FRAME			0x1001
#define SFCFG_CMD_ATE_ANT_NUM				0x1000
#define SFCFG_CMD_ATE_TX_FRAME_BW			0x1001
#define SFCFG_CMD_ATE_TX_COUNT				0x1001
#define SFCFG_CMD_ATE_PAYLOAD_LENGTH		0x1002
#define SFCFG_CMD_ATE_TX_FC					0x1003
#define SFCFG_CMD_ATE_TX_DUR				0x1004
#define SFCFG_CMD_ATE_TX_BSSID				0x1005
#define SFCFG_CMD_ATE_TX_DA					0x1006
#define SFCFG_CMD_ATE_TX_SA					0x1007
#define SFCFG_CMD_ATE_TX_SEQC				0x1008
#define SFCFG_CMD_ATE_PAYLOAD				0x1009
#define SFCFG_CMD_ATE_MACBYPASS_INTERVAL    0x100b
#define SFCFG_CMD_ATE_XO_VALUE  0X100c

#define SFCFG_CMD_ATE_READ_REG       0x10000
#define SFCFG_CMD_ATE_RF_TEMP_GET    0X10000
#define SFCFG_CMD_ATE_WRITE_REG      0x10001
#define SFCFG_CMD_ATE_BULK_READ_REG  0x10002
#define SFCFG_CMD_ATE_BULK_WRITE_REG 0x10003

#define SFCFG_PRIV_IOCTL_ATE	  (SIOCIWFIRSTPRIV + 0x08)

#define REFPOWER_OFFSET 186
#define CALIPOWER_OFFSET 188

#define true  1
#define false 0
#define PRINT_ATE_LOG 0
typedef unsigned char   uint8_t;
typedef int    int32_t;
typedef unsigned int   uint32_t;
typedef unsigned short  uint16_t;
typedef void* caddr_t;

enum register_mode {
	MAC_REG,
	PHY_REG,
	RF_REG,
};

static uint8_t  power_table[2048] = { 0 };


bool g_tx_frame_flag = false;
bool g_rx_frame_flag = false;
bool g_stop_tx_frame_flag = false;
bool g_stop_rx_frame_flag = false;
bool g_clock_flag = false;
bool g_start_tx_rx_by_macbypass_flag = false;
bool g_stop_tx_rx_by_macbypass_flag = false;
bool g_start_tx_frame_by_macbypass_flag = false;
bool g_stop_tx_frame_by_macbypass_flag = false;
bool g_start_rx_by_macbypass_flag = false;
bool g_stop_rx_by_macbypass_flag = false;
bool g_tx_test_tone_flag = false;
bool g_stop_tx_test_tone_flag = false;
bool g_agg_tx_frame_flag = false;

unsigned int g_tx_frame_num  = 1;
unsigned int g_tx_chan_freq  = 2412;
unsigned int g_tx_band_width = 0;
unsigned int g_tx_mode       = 0;
unsigned int g_tx_rate_idx   = 0;
unsigned int g_tx_use_sgi    = 0;
unsigned int g_tx_power    = 0;

static int32_t ioctl_socket = -1;
#if 0
static  uint8_t *sa = "16:88:aa:bb:cc:ee";
static  uint8_t *da = "16:88:aa:bb:cc:ee";
static  uint8_t *bssid = "16:88:aa:bb:cc:ee";
#endif


struct tx_frame_param {
	int frame_num;
};

enum ieee80211_band {
	IEEE80211_BAND_2GHZ =0,
	IEEE80211_BAND_5GHZ,
	IEEE80211_BAND_60GHZ,

	/* keep last */
	IEEE80211_NUM_BANDS
};


enum nl80211_chan_width {
	NL80211_CHAN_WIDTH_20_NOHT,
	NL80211_CHAN_WIDTH_20,
	NL80211_CHAN_WIDTH_40,
	NL80211_CHAN_WIDTH_80,
	NL80211_CHAN_WIDTH_80P80,
	NL80211_CHAN_WIDTH_160,
};


enum format_mode {
	NON_HT_MODE = 0,
	NON_HT_DUP_MODE,
	HT_MM_MODE,
	HT_GF_MODE,
	VHT_MODE,
};


struct rwnx_ate_channel_params{
	enum ieee80211_band chan_band;
	unsigned int chan_freq;
	enum nl80211_chan_width chan_width;
};


struct rwnx_ate_rate_params{
	enum format_mode mode;
	unsigned int rate_idx;
	bool use_short_preamble; //only for 2.4G CCK mode rate_idx 1~3
	bool use_short_gi;       //for HT VHT mode but not HT_GF_MODE
};

struct rwnx_ate_packet_params{
	unsigned int frame_num;
	unsigned int frame_len;
	unsigned int tx_power;
};

struct rwnx_ate_address_params{
	unsigned char bssid[6];
	unsigned char da[6];
	unsigned char sa[6];
	unsigned char addr4[6];

};

struct sfcfghdr {
	uint32_t magic_no;
	uint32_t comand_type;
	uint32_t comand_id;
	uint16_t length;
	uint16_t sequence;
	uint32_t status;
	char  data[25600];
}__attribute__((packed));



//back to user space  /sync to user space parameter
struct rwnx_ioctl_ate_dump_info {
	int bandwidth;
	int frame_bandwidth;
	int band;
	int freq;
	int freq1;
	int rate;
	int mode;
	int gi;
	int pre;
	int power;
	int len;
	int rssi_1;
	int rssi_2;
	uint32_t xof1;
	uint32_t xof2;
	uint32_t count;
	uint32_t tx_cont;
	uint32_t tx_successful;
	uint32_t tx_retry;
	uint32_t rec_rx_count;
	uint32_t fcs_err;
	uint32_t per;
	uint32_t phy_err;
	uint32_t mib_20m;
	uint32_t mib_40m;
	uint32_t mib_80m;
	uint32_t mib_160m;
	uint8_t  bssid[12];
	uint8_t  da[12];
	uint8_t  sa[12];
	uint32_t rf_temp;
}__attribute__((packed));

enum factory_info_type {
	TYPE_MAC = 0,
	TYPE_STR,
	TYPE_HEX,
	TYPE_INT,
	TYPE_RSSI_OFFSET
};

typedef struct {
	const char *name;
	const enum factory_info_type type;
	unsigned int offset;
	unsigned int length;
} factory_info_entry;

#define FACTORY_INFO_COUNT 22

const factory_info_entry factory_info_table[FACTORY_INFO_COUNT] = {
	{"mac-address",       TYPE_MAC,			0,   6},
	{"sn-number",         TYPE_STR,			6,   16},
	{"sn-flag",           TYPE_HEX,			22,  1},
	{"pcba-boot",         TYPE_STR,			23,  4},
	{"hardware-ver-flag", TYPE_HEX,			27,  2},
	{"hardware-ver",      TYPE_STR,			29,  32},
	{"country-id",        TYPE_STR,			61,  2},
	{"model-ver-flag",    TYPE_HEX,			63,  2},
	{"model-ver",         TYPE_STR,			65,  32},
	{"hw-feature",        TYPE_HEX,			97,  4},
	{"vender-flag",       TYPE_HEX,			101, 2},
	{"vender",            TYPE_STR,			103, 16},
	{"product-key-flag",  TYPE_HEX,			119, 2},
	{"product-key",       TYPE_STR,			121, 32},
	{"login-info-flag",   TYPE_HEX,			153, 2},
	{"login-info",        TYPE_HEX,			155, 4},
	{"rom-type-flag",     TYPE_HEX,			159, 2},
	{"rom-type",          TYPE_STR,			161, 4},
	{"cooling-temp",      TYPE_INT,			174, 2},
	{"gmac-delay",        TYPE_HEX,			178, 4},
	{"rssi-offset",       TYPE_RSSI_OFFSET,	182, 4},
	{"refpower",          TYPE_HEX,			186, 2},
	{"calipower",         TYPE_HEX,			188, 2},
};

static const factory_info_entry *get_factory_info_entry(const char *field_name)
{
	int i;

	for (i = 0; i < FACTORY_INFO_COUNT; i++) {
		if (strcmp(field_name, factory_info_table[i].name) == 0) {
			return &factory_info_table[i];
		}
	}
	return NULL;
}

/* Convert string(char) into decimal int*/
static int siwifi_char_to_int(char *tmp_data)
{
    int tmp_int = 0;

    while (*tmp_data != 0) {
        if (*tmp_data >= '0' && *tmp_data <= '9') {
            tmp_int = tmp_int * 10 + (*tmp_data - '0');
            tmp_data++;
        } else {
            return -1;
        }
    }
    return tmp_int;
}

/* create the socket fd to wext ioctl*/
static int32_t get_ioctl_socket(void)
{
	/*  Prepare socket */
	if (ioctl_socket == -1)
	{
		ioctl_socket = socket(AF_INET, SOCK_DGRAM, 0);
		fcntl(ioctl_socket, F_SETFD, fcntl(ioctl_socket, F_GETFD) | FD_CLOEXEC);
	}

	return ioctl_socket;
}

/* close the ioctl socket fd*/
void close_ioctl_socket(void)
{
	if (ioctl_socket > -1)
		close(ioctl_socket);
	ioctl_socket = -1;
}


/* do the ioctl*/
int32_t do_ioctl(int32_t cmd, struct iwreq *wrq)
{
	int32_t s = get_ioctl_socket();
	if(!wrq){
		printf("%s error:wrq is NULL\n",__func__);
		return -1;
	}
	strncpy(wrq->ifr_name, ifname, IFNAMSIZ);
	if(s < 0)
		return s;
	return ioctl(s, cmd, wrq);
}

#ifdef NAND_SUPPORT
int flash_init(void)
{
	struct ubi_vol_info vol_info;
	int ret;

	libubi = libubi_open();

	if (!libubi) {
		if (errno == 0)
			printf("UBI is not present in the system\n");
		perror("cannot open libubi");
		return -1;
	}

	ret = ubi_probe_node(libubi, UBI_FACTORY);
	if (ret != 2) {
		printf("error while probing \"%s\"", UBI_FACTORY);
		ret = -ENODEV;
		goto out_libubi;
	}

	ret = ubi_get_vol_info(libubi, UBI_FACTORY, &vol_info);
	if (ret) {
		printf("cannot get information about UBI volume \"%s\"",
		       UBI_FACTORY);
		goto out_libubi;
	}

	if (strcmp(vol_info.name, "factory") != 0) {
		fprintf(stderr, "%s (%s) isn't the factory volume.\n",
			UBI_FACTORY, vol_info.name);
		ret = -ENODEV;
	}
	return 0;
out_libubi:
	libubi_close(libubi);
	return ret;
}

void flash_deinit(void)
{
	if (libubi)
		libubi_close(libubi);
}

int ubi_read_all(void)
{
	int fd, ret = 0;
	fd = open(UBI_FACTORY, O_RDONLY);
	if (fd < 0) {
		perror("failed to open factory volume to read.");
		return fd;
	}
	memset(data_buf, 0, CAL_MAX_LEN);
	ret = read(fd, data_buf, CAL_MAX_LEN);
	close(fd);
	if (ret < 0)
		return ret;
	return 0;
}

int flash_read(char *buf, unsigned int offset, unsigned int len)
{
	int ret;
	if (offset + len > CAL_MAX_LEN) {
		fprintf(stderr, "reading %u from %u is out of bound.\n", len,
			offset);
		return -ERANGE;
	}
	ret = ubi_read_all();
	if (ret)
		return ret;
	memcpy(buf, data_buf + offset, len);
	return 0;
}

int flash_write(const char *buf, unsigned int offset, unsigned int len)
{
	int ret;
	int fd;
	int wr_len = CAL_MAX_LEN;
	char *wr_buf = data_buf;
	if (offset + len > CAL_MAX_LEN) {
		fprintf(stderr, "writing %u to %u is out of bound.\n", len,
			offset);
		return -ERANGE;
	}

	ret = ubi_read_all();
	if (ret)
		return ret;

	memcpy(data_buf + offset, buf, len);

	fd = open(UBI_FACTORY, O_RDWR);
	if (fd == -1) {
		perror("failed to open volume");
		return errno;
	}
	ret = ubi_update_start(libubi, fd, CAL_MAX_LEN);
	if (ret) {
		fprintf(stderr, "cannot start volume update: %d", ret);
		goto out;
	}

	while (wr_len) {
		ret = write(fd, wr_buf, len);
		if (ret < 0) {
			if (errno == EINTR) {
				fprintf(stderr, "do not interrupt me!");
				continue;
			}
			perror("error writing to the volume.");
			ret = errno;
			goto out;
		}

		if (ret == 0) {
			perror("error writing to the volume.");
			ret = -EINVAL;
			goto out;
		}

		wr_len -= ret;
		wr_buf += ret;
	}

out:
	close(fd);
	return ret;
}
#endif

/*
  descript : get power offset in factory

 * */
uint32_t get_power_offset(uint32_t channel,uint32_t mode,uint32_t bw,uint32_t rate){

    uint32_t offset  = 0;
    if (channel < 3000) {
        offset = (channel - 2412) / 5 * 52;
        switch (mode) {
        case 0:
            offset += rate;
            break;
        case 2:
            if (bw == 0 || bw ==1)
                offset += 12 + rate;
            else
                offset += 20 + rate;
            break;
        case 5:
            if (bw == 1)
		offset += 28 + rate;
            else if (bw ==2)
                offset += 40 + rate;
        }
    } else {
        switch (channel) {
        case 5180:
	    if (mode == 0 || mode == 2 || mode == 4 || mode == 5)
                offset = CALI_TABLE_2G_SIZE + CALI_TABLE_5G_CH_SIZE * 0;
            break;
        case 5200:
	    if (mode == 0 || mode == 2 || mode == 4 || mode == 5)
		offset = CALI_TABLE_2G_SIZE + CALI_TABLE_5G_CH_SIZE * 1;
            break;
        case 5220:
            if (mode == 0 || mode == 2 || mode == 4 || mode == 5)
                offset = CALI_TABLE_2G_SIZE + CALI_TABLE_5G_CH_SIZE * 2;
            break;
        case 5240:
            if (mode == 0 || mode == 2 || mode == 4 || mode == 5)
		offset = CALI_TABLE_2G_SIZE + CALI_TABLE_5G_CH_SIZE * 3;
            break;
        case 5260:
            if (mode == 0 || mode == 2 || mode == 4 || mode == 5)
                offset = CALI_TABLE_2G_SIZE + CALI_TABLE_5G_CH_SIZE * 4;
            break;
        case 5280:
            if (mode == 0 || mode == 2 || mode == 4 || mode == 5)
                offset = CALI_TABLE_2G_SIZE + CALI_TABLE_5G_CH_SIZE * 5;
            break;
        case 5300:
            if (mode == 0 || mode == 2 || mode == 4 || mode == 5)
                offset = CALI_TABLE_2G_SIZE + CALI_TABLE_5G_CH_SIZE * 6;
            break;
        case 5320:
            if (mode == 0 || mode == 2 || mode == 4 || mode == 5)
                offset = CALI_TABLE_2G_SIZE + CALI_TABLE_5G_CH_SIZE * 7;
            break;
        case 5500:
            if (mode == 0 || mode == 2 || mode == 4 || mode == 5)
                offset = CALI_TABLE_2G_SIZE + CALI_TABLE_5G_CH_SIZE * 8;
            break;
        case 5520:
            if (mode == 0 || mode == 2 || mode == 4 || mode == 5)
		offset = CALI_TABLE_2G_SIZE + CALI_TABLE_5G_CH_SIZE * 9;
            break;
        case 5540:
            if (mode == 0 || mode == 2 || mode == 4 || mode == 5)
                offset = CALI_TABLE_2G_SIZE + CALI_TABLE_5G_CH_SIZE * 10;
            break;
        case 5560:
            if (mode == 0 || mode == 2 || mode == 4 || mode == 5)
                offset = CALI_TABLE_2G_SIZE + CALI_TABLE_5G_CH_SIZE * 11;
            break;
        case 5580:
            if (mode == 0 || mode == 2 || mode == 4 || mode == 5)
                offset = CALI_TABLE_2G_SIZE + CALI_TABLE_5G_CH_SIZE * 12;
            break;
        case 5600:
            if (mode == 0 || mode == 2 || mode == 4 || mode == 5)
                offset = CALI_TABLE_2G_SIZE + CALI_TABLE_5G_CH_SIZE * 13;
            break;
        case 5620:
            if (mode == 0 || mode == 2 || mode == 4 || mode == 5)
                offset = CALI_TABLE_2G_SIZE + CALI_TABLE_5G_CH_SIZE * 14;
            break;
        case 5640:
            if (mode == 0 || mode == 2 || mode == 4 || mode == 5)
                offset = CALI_TABLE_2G_SIZE + CALI_TABLE_5G_CH_SIZE * 15;
            break;
        case 5660:
            if (mode == 0 || mode == 2 || mode == 4 || mode == 5)
                offset = CALI_TABLE_2G_SIZE + CALI_TABLE_5G_CH_SIZE * 16;
            break;
        case 5680:
            if (mode == 0 || mode == 2 || mode == 4 || mode == 5)
                offset = CALI_TABLE_2G_SIZE + CALI_TABLE_5G_CH_SIZE * 17;
            break;
        case 5700:
            if (mode == 0 || mode == 2 || mode == 4 || mode == 5)
                offset = CALI_TABLE_2G_SIZE + CALI_TABLE_5G_CH_SIZE * 18;
            break;
        case 5720:
            if (mode == 0 || mode == 2 || mode == 4 || mode == 5)
                offset = CALI_TABLE_2G_SIZE + CALI_TABLE_5G_CH_SIZE * 19;
            break;
        case 5745:
            if (mode == 0 || mode == 2 || mode == 4 || mode == 5)
                offset = CALI_TABLE_2G_SIZE + CALI_TABLE_5G_CH_SIZE * 20;
            break;
        case 5765:
            if (mode == 0 || mode == 2 || mode == 4 || mode == 5)
                offset = CALI_TABLE_2G_SIZE + CALI_TABLE_5G_CH_SIZE * 21;
            break;
        case 5785:
            if (mode == 0 || mode == 2 || mode == 4 || mode == 5)
                offset = CALI_TABLE_2G_SIZE + CALI_TABLE_5G_CH_SIZE * 22;
            break;
        case 5805:
            if (mode == 0 || mode == 2 || mode == 4 || mode == 5)
                offset = CALI_TABLE_2G_SIZE + CALI_TABLE_5G_CH_SIZE * 23;
            break;
        case 5825:
            if (mode == 0 || mode == 2 || mode == 4 || mode == 5)
                offset = CALI_TABLE_2G_SIZE + CALI_TABLE_5G_CH_SIZE * 24;
            break;
        }
        switch (mode) {
        case 0:
            offset += rate;
            break;
        case 2:
            if (bw == 0 || bw ==1)
                offset += 8 + rate;
            else
                offset += 16 + rate;
            break;
        case 4:
            if (bw == 0 || bw ==1)
                offset += 24 + rate;
            else if (bw == 2)
                offset += 33 + rate;
            else
                offset += 43 + rate;
            break;
        case 5:
            if (bw == 1)
                offset += rate + CALI_TABLE_5G_ANAC_CH_SIZE;
	    else if (bw == 2)
                offset += 12 + rate + CALI_TABLE_5G_ANAC_CH_SIZE;
	    else if (bw == 3)
                offset += 24 + rate + CALI_TABLE_5G_ANAC_CH_SIZE;
            else if (bw == 4)
                offset += 36 + rate + CALI_TABLE_5G_ANAC_CH_SIZE;
        }
    }
	return  offset;
}

int32_t read_gain_table_from_mtd(unsigned char *buffer)
{
	struct iwreq wrq;
	struct sfcfghdr tmp;
	//uint32_t offset = 0;
	int i ;
	memset(&wrq, 0, sizeof(struct iwreq));
	memset(&tmp, 0, sizeof(struct sfcfghdr));

	//read gian table from mtd
	tmp.magic_no = SFCFG_MAGIC_NO;
	tmp.comand_type = SFCFG_CMD_ATE_READ_FROM_FLASH;
	tmp.comand_id   = SFCFG_CMD_ATE_READ_FROM_FLASH;
	tmp.length      = 6406;
	tmp.sequence    = 1;

	wrq.u.data.pointer = (caddr_t)&tmp;
	wrq.u.data.length = sizeof(struct sfcfghdr) - sizeof(((struct sfcfghdr *)0)->data) + tmp.length;

#ifdef NAND_SUPPORT
	int ret;

	ret = flash_init();
	if (ret)
		return ret;
	ret = flash_read(tmp.data, 2048, tmp.length);
	if (ret)
		return ret;
#else
	if (do_ioctl(SFCFG_PRIV_IOCTL_ATE,&wrq) < 0) {
		printf("SFCFG_PRIV_IOCTL_ATE ioctl SFCFG_CMD_ATE_READ_FROM_FLASH failed\n");
		return -1;
	}
#endif

	memcpy(buffer,&tmp.data[0],6406);
	printf("read tx power is : \n");
	for(i = 0;i < 6406;i++)
	{
		if( i >= 0 && i <= 3 )
		{
			printf("%4d",buffer[i]);
			if(i == 3)
				printf("\n");
		}
		else if (i > 3 && i < 680)
		{
			if(i == 4)
				printf("-----2.4G ant1 calibration power-----\r\n");
			printf("%4d",buffer[i]);
			if(i%52 == 3)
				printf("\n");
		}
		else if(i >= 680 && i < 3205)
		{
			if(i == 680)
				printf("-----5.8G ant1 calibration power-----\r\n");
			printf("%4d",buffer[i]);
			if((i-679)%101 == 0)
				printf("\n");
		}
		else if(i >= 3205 && i < 3881)
		{
			if(i == 3205)
				printf("-----2.4G ant2 calibration power-----\r\n");
			printf("%4d",buffer[i]);
			if((i-3204)%52 == 0)
				printf("\n");
		}
		else
		{
			if(i == 3881)
				printf("-----5.8G ant2 calibration power-----\r\n");
			printf("%4d",buffer[i]);
			if((i-3880)%101 == 0)
				printf("\n");
		}
	}

	return 0;
}

int32_t save_version_to_factory(char *version)
{
	struct iwreq wrq;
	struct sfcfghdr tmp;
	memset(&wrq, 0, sizeof(struct iwreq));
	memset(&tmp, 0, sizeof(struct sfcfghdr));

	// cmd ate band
	tmp.magic_no = SFCFG_MAGIC_NO;
	tmp.comand_type = SFCFG_CMD_ATE_SAVE_DATA_TO_FLASH;
	tmp.comand_id   = SFCFG_CMD_ATE_SAVE_DATA_TO_FLASH;
	tmp.length      = 2;
	tmp.sequence    = 1;


	sprintf(tmp.data,"%4d",2048);
	memcpy(&tmp.data[4],version,2);
	wrq.u.data.pointer = (caddr_t)&tmp;
	wrq.u.data.length = sizeof(struct sfcfghdr) - sizeof(((struct sfcfghdr *)0)->data) + 10;

  	printf("tmp.data------%s\n",tmp.data);

#ifdef NAND_SUPPORT
	int ret;

	ret = flash_init();
	if (ret)
		return ret;

	flash_write(version, 2048, 2);
	flash_deinit();
#else
	if(do_ioctl(SFCFG_PRIV_IOCTL_ATE, &wrq) < 0){
		printf("SFCFG_PRIV_IOCTL_ATE ioctl SFCFG_CMD_ATE_SAVE_DATA_TO_FLASH failed\n");
		return -1;
	}
#endif

	return 0;
}


int32_t save_power_table_to_factory(uint32_t length,uint32_t offset){
	struct iwreq wrq;
	struct sfcfghdr tmp;
	uint32_t offset_tmp = offset;
	FILE *p1,*p2;
	int i;
	char power_text[6420]={ 0 };
	memset(&wrq, 0, sizeof(struct iwreq));
	memset(&tmp, 0, sizeof(struct sfcfghdr));

	// cmd ate band
	tmp.magic_no = SFCFG_MAGIC_NO;
	tmp.comand_type = SFCFG_CMD_ATE_SAVE_DATA_TO_FLASH;
	tmp.comand_id   = SFCFG_CMD_ATE_SAVE_DATA_TO_FLASH;
	tmp.length      = 6420;
	tmp.sequence    = 1;

	p1 = fopen("/etc/atetools/power_save_ant1.txt","r");
	for(i = 0; i < CALI_TABLE_ONE_ANT; i++)
	{
		fscanf(p1,"%hhd",&power_text[i]);
	}

	p2 = fopen("/etc/atetools/power_save_ant2.txt","r");
	for(i = CALI_TABLE_ONE_ANT; i < CALI_TABLE_WHOLE; i++)
	{
		fscanf(p2,"%hhd",&power_text[i]);
	}

	fclose(p1);
	fclose(p2);

	sprintf(tmp.data,"%4d",offset_tmp + 2048 + 4);//factory form 2048 begin and 4 bytes form 2048 to 2052 is save xo;
	memcpy(&tmp.data[4],power_text,length);

	printf("power_text[start] :%d",power_text[0]);
	printf("power_text[end] :%d",power_text[CALI_TABLE_WHOLE]);
	wrq.u.data.pointer = (caddr_t)&tmp;
	wrq.u.data.length = sizeof(struct sfcfghdr) - sizeof(((struct sfcfghdr *)0)->data) + tmp.length + 4;

	printf("length :%d\n",length);

#ifdef NAND_SUPPORT
	int ret;

	ret = flash_init();
	if (ret)
		return ret;

	flash_write(power_text, offset_tmp + 2048 + 4, length);
	flash_deinit();
#else
	if (do_ioctl(SFCFG_PRIV_IOCTL_ATE, &wrq) < 0) {
		printf("SFCFG_PRIV_IOCTL_ATE ioctl SFCFG_CMD_ATE_SAVE_DATA_TO_FLASH failed\n");
		return -1;
	}
#endif

	return 0;
}

struct calibrate_info
{
	uint32_t channel;
	uint32_t mode;
	uint32_t bw;
	uint32_t rate;
};

struct calibrate_info  get_ate_calibrate_info()
{
	struct iwreq wrq;
	struct sfcfghdr tmp;
	struct rwnx_ioctl_ate_dump_info *usdrv;
	struct calibrate_info n;

	memset(&wrq, 0, sizeof(struct iwreq));
	memset(&tmp, 0, sizeof(struct sfcfghdr));
	// cmd ate band
	tmp.magic_no = SFCFG_MAGIC_NO;
	tmp.comand_type = SFCFG_CMD_ATE_GET_INFO;
	tmp.comand_id   = SFCFG_CMD_ATE_GET_INFO;
	tmp.length      = 10;
	tmp.sequence    = 1;

	wrq.u.data.pointer = (caddr_t)&tmp;
	wrq.u.data.length = sizeof(struct sfcfghdr) - sizeof(((struct sfcfghdr *)0)->data) + tmp.length;

	if(do_ioctl(SFCFG_PRIV_IOCTL_ATE,&wrq) < 0){
		printf("SFCFG_CMD_ATE_GET_INFO ioctl failed\n");
		//return -1;
	}
    	usdrv = (struct rwnx_ioctl_ate_dump_info *)malloc(sizeof(struct rwnx_ioctl_ate_dump_info));
   	if(!usdrv){
        printf("oom!\n");
        //return -2;
    }

	memset(usdrv,0,sizeof(struct rwnx_ioctl_ate_dump_info));
	memcpy(usdrv, wrq.u.data.pointer, sizeof(struct rwnx_ioctl_ate_dump_info));

	printf("bandwidth = %d\n",usdrv->bandwidth);
	printf("freq = %d\n",usdrv->freq);
	printf("rate = %d\n",usdrv->rate);
	printf("mode = %d\n",usdrv->mode);
	n.channel = usdrv->freq;
	n.mode = usdrv->mode;
	n.bw = usdrv->bandwidth;
	n.rate = usdrv->rate;

	free(usdrv);
	return n;
}

int32_t get_ate_info()
{
	struct iwreq wrq;
	struct sfcfghdr tmp;
	struct rwnx_ioctl_ate_dump_info *usdrv;

	memset(&wrq, 0, sizeof(struct iwreq));
	memset(&tmp, 0, sizeof(struct sfcfghdr));
	// cmd ate band
	tmp.magic_no = SFCFG_MAGIC_NO;
	tmp.comand_type = SFCFG_CMD_ATE_GET_INFO;
	tmp.comand_id   = SFCFG_CMD_ATE_GET_INFO;
	tmp.length      = 10;
	tmp.sequence    = 1;

	wrq.u.data.pointer = (caddr_t)&tmp;
	wrq.u.data.length = sizeof(struct sfcfghdr) - sizeof(((struct sfcfghdr *)0)->data) + tmp.length;

	if(do_ioctl(SFCFG_PRIV_IOCTL_ATE,&wrq) < 0){
		printf("SFCFG_CMD_ATE_GET_INFO ioctl failed\n");
		return -1;
	}
    usdrv = (struct rwnx_ioctl_ate_dump_info *)malloc(sizeof(struct rwnx_ioctl_ate_dump_info));
    if(!usdrv){
        printf("oom!\n");
        return -2;
    }

	memset(usdrv,0,sizeof(struct rwnx_ioctl_ate_dump_info));
	memcpy(usdrv, wrq.u.data.pointer, sizeof(struct rwnx_ioctl_ate_dump_info));

	printf("bandwidth = %d\n",usdrv->bandwidth);
	printf("frame_bandwidth = %d\n",usdrv->frame_bandwidth);
	printf("band = %d\n",usdrv->band);
	printf("freq = %d\n",usdrv->freq);
	printf("freq = %d\n",usdrv->freq1);
	printf("rate = %d\n",usdrv->rate);
	printf("mode = %d\n",usdrv->mode);
	printf("GI = %d\n",usdrv->gi);
	printf("preamble = %d\n",usdrv->pre);
	printf("power = %d\n",usdrv->power);
	printf("length = %d\n",usdrv->len);
	printf("rssi_1 = %d\n",usdrv->rssi_1);
	printf("rssi_2 = %d\n",usdrv->rssi_2);
	printf("xof1 = %d\n",usdrv->xof1);
	printf("xof2 = %d\n",usdrv->xof2);
	printf("total count = %d\n",usdrv->count);
	printf("tx_cont = %d\n",usdrv->tx_cont);
	printf("tx_successful = %d\n",usdrv->tx_successful);
	printf("tx_retry = %d\n",usdrv->tx_retry);
	printf("rec_rx_count = %d\n",usdrv->rec_rx_count);
	printf("fcs_err = %d\n",usdrv->fcs_err);
	printf("per = %d\n",usdrv->per);
	printf("phy_err = %d\n",usdrv->phy_err);
	// printf("reg_val = %d\n",usdrv->reg_val);
	printf("bssid = 0x%02x : 0x%02x : 0x%02x : 0x%02x : 0x%02x : 0x%02x\n",usdrv->bssid[0],usdrv->bssid[1],usdrv->bssid[2],usdrv->bssid[3],usdrv->bssid[4],usdrv->bssid[5]);
	printf("da = 0x%02x : 0x%02x : 0x%02x : 0x%02x : 0x%02x : 0x%02x\n",usdrv->da[0],usdrv->da[1],usdrv->da[2],usdrv->da[3],usdrv->da[4],usdrv->da[5]);
	printf("sa = 0x%02x : 0x%02x : 0x%02x : 0x%02x : 0x%02x : 0x%02x\n",usdrv->sa[0],usdrv->sa[1],usdrv->sa[2],usdrv->sa[3],usdrv->sa[4],usdrv->sa[5]);
    free(usdrv);
	return 0;
}

int32_t read_iw_reg(char *tmpdata)
{
	struct iwreq wrq;
	struct sfcfghdr tmp;
	memset(&wrq, 0, sizeof(struct iwreq));
	memset(&tmp, 0, sizeof(struct sfcfghdr));
	// cmd ate read reg
	tmp.magic_no = SFCFG_MAGIC_NO;
	tmp.comand_type = SFCFG_CMD_ATE_READ_REG;
	tmp.comand_id   = SFCFG_CMD_ATE_READ_REG;
	tmp.length      = 20;
	tmp.sequence    = 1;

	memcpy(tmp.data,tmpdata,20);
	printf("tmp.data= %s---------\n",tmp.data);

	wrq.u.data.pointer = (caddr_t)&tmp;
	wrq.u.data.length = sizeof(struct sfcfghdr) - sizeof(((struct sfcfghdr *)0)->data) + tmp.length;

	if(do_ioctl(SFCFG_PRIV_IOCTL_ATE, &wrq) < 0){
		printf("SFCFG_PRIV_IOCTL_ATE ioctl SFCFG_CMD_ATE_READ_REG failed\n");
		return -1;
	}
#if 0
	//get the results
	memset(&wrq, 0, sizeof(struct iwreq));
	memset(&tmp, 0, sizeof(struct sfcfghdr));

	tmp.magic_no = SFCFG_MAGIC_NO;
	tmp.comand_type = SFCFG_CMD_ATE_GET_INFO;
	tmp.comand_id   = SFCFG_CMD_ATE_GET_INFO;
	tmp.length      = 10;
	tmp.sequence    = 1;
	wrq.u.data.pointer = (caddr_t)&tmp;
	wrq.u.data.length = sizeof(struct sfcfghdr) - sizeof(((struct sfcfghdr *)0)->data) + tmp.length;
	if(do_ioctl(SFCFG_PRIV_IOCTL_ATE,&wrq) < 0){
		printf("SFCFG_CMD_ATE_GET_INFO ioctl failed\n");
		return -1;
	}
	memset(&user_reg,0,sizeof(struct rwnx_ioctl_ate_dump_info));
	memcpy(&user_reg, wrq.u.data.pointer, sizeof(struct rwnx_ioctl_ate_dump_info));
	printf("register value = 0x%x\n",user_reg.reg_val);
#endif
	return 0;
}

int32_t write_iw_reg(char *tmpdata)
{
	struct iwreq wrq;
	struct sfcfghdr tmp;
	memset(&wrq, 0, sizeof(struct iwreq));
	memset(&tmp, 0, sizeof(struct sfcfghdr));
	// cmd ate read reg
	tmp.magic_no = SFCFG_MAGIC_NO;
	tmp.comand_type = SFCFG_CMD_ATE_WRITE_REG;
	tmp.comand_id   = SFCFG_CMD_ATE_WRITE_REG;
	tmp.length      = 21;
	tmp.sequence    = 1;

	memcpy(tmp.data,tmpdata,tmp.length);
	printf("tmp.data= %s---------\n",tmp.data);

	wrq.u.data.pointer = (caddr_t)&tmp;
	wrq.u.data.length = sizeof(struct sfcfghdr) - sizeof(((struct sfcfghdr *)0)->data) + tmp.length;

	if(do_ioctl(SFCFG_PRIV_IOCTL_ATE, &wrq) < 0){
		printf("SFCFG_PRIV_IOCTL_ATE ioctl SFCFG_CMD_ATE_WRITE_REG failed\n");
		return -1;
	}
#if 0
	//get the results
	memset(&wrq, 0, sizeof(struct iwreq));
	memset(&tmp, 0, sizeof(struct sfcfghdr));

	tmp.magic_no = SFCFG_MAGIC_NO;
	tmp.comand_type = SFCFG_CMD_ATE_GET_INFO;
	tmp.comand_id   = SFCFG_CMD_ATE_GET_INFO;
	tmp.length      = 10;
	tmp.sequence    = 1;
	wrq.u.data.pointer = (caddr_t)&tmp;
	wrq.u.data.length = sizeof(struct sfcfghdr) - sizeof(((struct sfcfghdr *)0)->data) + tmp.length;
	if(do_ioctl(SFCFG_PRIV_IOCTL_ATE,&wrq) < 0){
		printf("SFCFG_CMD_ATE_GET_INFO ioctl failed\n");
		return -1;
	}
	memset(&user_reg,0,sizeof(struct rwnx_ioctl_ate_dump_info));
	memcpy(&user_reg, wrq.u.data.pointer, sizeof(struct rwnx_ioctl_ate_dump_info));
	if(user_reg.reg_status == 0){
		printf("write register %x done!\n",user_reg.reg_val);
	}
	else{
		printf("write register fail!\n");
	}
#endif
	return 0;
}

int32_t read_iw_bulk_reg(char *tmpdata)
{
	struct iwreq wrq;
	struct sfcfghdr tmp;
	struct rwnx_ioctl_ate_dump_info user_reg;
	int num = 0;
	int type = 0;
	int space = 0;
	int i = 0;
	char addrs[1000] = {0};
	char tmp_addr[9] = {0};
	if(tmpdata[0]=='0'){
		type = MAC_REG;
		space = 8;}
	else if(tmpdata[0]=='1'){
		type = PHY_REG;
		space = 8;}
	else{type = 2;
		space = 4;
	}
	num = atoi(tmpdata+2);
	memcpy(addrs,tmpdata+4,sizeof(addrs));
	while(i<num){
		memcpy(tmp_addr,tmpdata+4+i*(space+1),space);
		memset(&wrq, 0, sizeof(struct iwreq));
		memset(&tmp, 0, sizeof(struct sfcfghdr));

		// cmd ate read reg
		tmp.magic_no = SFCFG_MAGIC_NO;
		tmp.comand_type = SFCFG_CMD_ATE_READ_REG;
		tmp.comand_id   = SFCFG_CMD_ATE_READ_REG;
		tmp.length      = 8;
		tmp.sequence    = 1;

		memcpy(tmp.data,tmp_addr,tmp.length);

		wrq.u.data.pointer = (caddr_t)&tmp;
		wrq.u.data.length = sizeof(struct sfcfghdr) - sizeof(((struct sfcfghdr *)0)->data) + tmp.length;
		if(do_ioctl(SFCFG_PRIV_IOCTL_ATE, &wrq) < 0){
			printf("SFCFG_PRIV_IOCTL_ATE ioctl SFCFG_CMD_ATE_READ_BULK_REG failed\n");
			return -1;
		}
		//get the results
		memset(&wrq, 0, sizeof(struct iwreq));
		memset(&tmp, 0, sizeof(struct sfcfghdr));

		tmp.magic_no = SFCFG_MAGIC_NO;
		tmp.comand_type = SFCFG_CMD_ATE_GET_INFO;
		tmp.comand_id   = SFCFG_CMD_ATE_GET_INFO;
		tmp.length      = 10;
		tmp.sequence    = 1;
		wrq.u.data.pointer = (caddr_t)&tmp;
		wrq.u.data.length = sizeof(struct sfcfghdr) - sizeof(((struct sfcfghdr *)0)->data) + tmp.length;
		if(do_ioctl(SFCFG_PRIV_IOCTL_ATE,&wrq) < 0){
			printf("SFCFG_CMD_ATE_GET_INFO ioctl failed\n");
			return -1;
		}
		memset(&user_reg,0,sizeof(struct rwnx_ioctl_ate_dump_info));
		memcpy(&user_reg, wrq.u.data.pointer, sizeof(struct rwnx_ioctl_ate_dump_info));
		// printf("register:0x%s value = 0x%x\n",tmp_addr,user_reg.reg_val);
		i++;
	}
	return 0;
}


int32_t write_iw_bulk_reg(char *tmpdata)
{
	struct iwreq wrq;
	struct sfcfghdr tmp;
	struct rwnx_ioctl_ate_dump_info user_reg;
	int num = 0;
	int type = 0;
	int space = 0;
	int i = 0;
	char addrs[1000] = {0};
	char tmp_addr_val[18] = {0};
	char tmp_addr[9] = {0};
	if(tmpdata[0]=='0'){
		type = MAC_REG;
		space = 17;}
	else if(tmpdata[0]=='1'){
		type = PHY_REG;
		space = 17;}
	else{type = RF_REG;
		space = 9;
	}
	num = atoi(tmpdata+2);
	memcpy(addrs,tmpdata+4,sizeof(addrs));
	while(i<num){
		memcpy(tmp_addr_val,tmpdata+4+i*(space+1),space);
		memcpy(tmp_addr,tmp_addr_val,(space-1)/2);
		memset(&wrq, 0, sizeof(struct iwreq));
		memset(&tmp, 0, sizeof(struct sfcfghdr));

		// cmd ate write reg
		tmp.magic_no = SFCFG_MAGIC_NO;
		tmp.comand_type = SFCFG_CMD_ATE_WRITE_REG;
		tmp.comand_id   = SFCFG_CMD_ATE_WRITE_REG;
		tmp.length      = space;
		tmp.sequence    = 1;

		memcpy(tmp.data,tmp_addr_val,tmp.length);

		wrq.u.data.pointer = (caddr_t)&tmp;
		wrq.u.data.length = sizeof(struct sfcfghdr) - sizeof(((struct sfcfghdr *)0)->data) + tmp.length;
		if(do_ioctl(SFCFG_PRIV_IOCTL_ATE, &wrq) < 0){
			printf("SFCFG_PRIV_IOCTL_ATE ioctl SFCFG_CMD_ATE_WRITE_BULK_REG failed\n");
			return -1;
		}
		//get the results
		memset(&wrq, 0, sizeof(struct iwreq));
		memset(&tmp, 0, sizeof(struct sfcfghdr));

		tmp.magic_no = SFCFG_MAGIC_NO;
		tmp.comand_type = SFCFG_CMD_ATE_GET_INFO;
		tmp.comand_id   = SFCFG_CMD_ATE_GET_INFO;
		tmp.length      = 10;
		tmp.sequence    = 1;
		wrq.u.data.pointer = (caddr_t)&tmp;
		wrq.u.data.length = sizeof(struct sfcfghdr) - sizeof(((struct sfcfghdr *)0)->data) + tmp.length;
		if(do_ioctl(SFCFG_PRIV_IOCTL_ATE,&wrq) < 0){
			printf("SFCFG_CMD_ATE_GET_INFO ioctl failed\n");
			return -1;
		}
		memset(&user_reg,0,sizeof(struct rwnx_ioctl_ate_dump_info));
		memcpy(&user_reg, wrq.u.data.pointer, sizeof(struct rwnx_ioctl_ate_dump_info));
//		if(user_reg.reg_status == 0){
//			printf("write register:0x%s done!\n",tmp_addr);
//		}
//		else{
//			printf("write register:0x%s fail!\n",tmp_addr);
//		}
		i++;
	}
	return 0;
}


int32_t set_phy_bandwidth(char *tmpdata)
{
	struct iwreq wrq;
	struct sfcfghdr tmp;
	memset(&wrq, 0, sizeof(struct iwreq));
	memset(&tmp, 0, sizeof(struct sfcfghdr));
	// cmd ate bandwidth
	tmp.magic_no = SFCFG_MAGIC_NO;
	tmp.comand_type = SFCFG_CMD_ATE_SET_BANDWIDTH;
	tmp.comand_id   = SFCFG_CMD_ATE_SET_BANDWIDTH;
	tmp.length      = 10;
	tmp.sequence    = 1;

	memcpy(tmp.data,tmpdata,10);
	//printf("tmp.data= %s---------\n",tmp.data);

	wrq.u.data.pointer = (caddr_t)&tmp;
	wrq.u.data.length = sizeof(struct sfcfghdr) - sizeof(((struct sfcfghdr *)0)->data) + tmp.length;

	if(do_ioctl(SFCFG_PRIV_IOCTL_ATE, &wrq) < 0){
		printf("SFCFG_PRIV_IOCTL_ATE ioctl SFCFG_CMD_ATE_SET_BANDWIDTH failed\n");
		return -1;
	}

	return 0;
}


int32_t set_phy_chan_freq(char *tmpdata)
{
	struct iwreq wrq;
	struct sfcfghdr tmp;
	memset(&wrq, 0, sizeof(struct iwreq));
	memset(&tmp, 0, sizeof(struct sfcfghdr));
	// cmd ate band
	tmp.magic_no = SFCFG_MAGIC_NO;
	tmp.comand_type = SFCFG_CMD_ATE_SET_CHANNEL;
	tmp.comand_id   = SFCFG_CMD_ATE_SET_CHANNEL;
	tmp.length      = 10;
	tmp.sequence    = 1;

	strncpy(tmp.data,tmpdata,10);
	//printf("tmp.data= %s---------\n", tmp.data);

	wrq.u.data.pointer = (caddr_t)&tmp;
	wrq.u.data.length = sizeof(struct sfcfghdr) - sizeof(((struct sfcfghdr *)0)->data) + tmp.length;

	if(do_ioctl(SFCFG_PRIV_IOCTL_ATE, &wrq) < 0){
		printf("SFCFG_PRIV_IOCTL_ATE ioctl SFCFG_CMD_ATE_SET_CHANNEL failed\n");
		return -1;
	}
	return 0;

}


int32_t set_phy_center_freq1(char *tmpdata)
{
	struct iwreq wrq;
	struct sfcfghdr tmp;
	memset(&wrq, 0, sizeof(struct iwreq));
	memset(&tmp, 0, sizeof(struct sfcfghdr));
	// cmd ate band
	tmp.magic_no = SFCFG_MAGIC_NO;
	tmp.comand_type = SFCFG_CMD_ATE_SET_CENTER_FREQ1;
	tmp.comand_id   = SFCFG_CMD_ATE_SET_CENTER_FREQ1;
	tmp.length      = 10;
	tmp.sequence    = 1;

	strncpy(tmp.data,tmpdata,10);
	//printf("tmp.data= %s---------\n", tmp.data);

	wrq.u.data.pointer = (caddr_t)&tmp;
	wrq.u.data.length = sizeof(struct sfcfghdr) - sizeof(((struct sfcfghdr *)0)->data) + tmp.length;

	if(do_ioctl(SFCFG_PRIV_IOCTL_ATE, &wrq) < 0){
		printf("SFCFG_PRIV_IOCTL_ATE ioctl SFCFG_CMD_ATE_SET_CENTER_FREQ1 failed\n");
		return -1;
	}
	return 0;

}

int32_t set_phy_mode(char *tmpdata)
{
	struct iwreq wrq;
	struct sfcfghdr tmp;
	memset(&wrq, 0, sizeof(struct iwreq));
	memset(&tmp, 0, sizeof(struct sfcfghdr));
	// cmd ate band
	tmp.magic_no = SFCFG_MAGIC_NO;
	tmp.comand_type = SFCFG_CMD_ATE_SET_PHY_MODE;
	tmp.comand_id   = SFCFG_CMD_ATE_SET_PHY_MODE;
	tmp.length      = 10;
	tmp.sequence    = 1;

	memcpy(tmp.data,tmpdata, 10);
	//printf("tmp.data= %s---------\n", tmp.data);

	wrq.u.data.pointer = (caddr_t)&tmp;
	wrq.u.data.length = sizeof(struct sfcfghdr) - sizeof(((struct sfcfghdr *)0)->data) + tmp.length;

	if(do_ioctl(SFCFG_PRIV_IOCTL_ATE, &wrq) < 0){
		printf("SFCFG_PRIV_IOCTL_ATE ioctl SFCFG_CMD_ATE_SET_PHY_MODE failed\n");
		return -1;
	}

	return 0;

}


int32_t set_tx_nss(char *tmpdata)
{
	struct iwreq wrq;
	struct sfcfghdr tmp;
	memset(&wrq, 0, sizeof(struct iwreq));
	memset(&tmp, 0, sizeof(struct sfcfghdr));
	// cmd ate band
	tmp.magic_no = SFCFG_MAGIC_NO;
	tmp.comand_type = SFCFG_CMD_ATE_SET_NSS;
	tmp.comand_id   = SFCFG_CMD_ATE_SET_NSS;
	tmp.length      = 10;
	tmp.sequence    = 1;

	memcpy(tmp.data,tmpdata, 10);

	wrq.u.data.pointer = (caddr_t)&tmp;
	wrq.u.data.length = sizeof(struct sfcfghdr) - sizeof(((struct sfcfghdr *)0)->data) + tmp.length;

	if(do_ioctl(SFCFG_PRIV_IOCTL_ATE, &wrq) < 0){
		printf("SFCFG_PRIV_IOCTL_ATE ioctl SFCFG_CMD_ATE_SET_NSS failed\n");
		return -1;
	}

	return 0;

}

int32_t set_tx_filter(char *tmpdata)
{
	struct iwreq wrq;
	struct sfcfghdr tmp;
	memset(&wrq, 0, sizeof(struct iwreq));
	memset(&tmp, 0, sizeof(struct sfcfghdr));
	// cmd ate band
	tmp.magic_no = SFCFG_MAGIC_NO;
	tmp.comand_type = SFCFG_CMD_ATE_SET_TX_FILTER;
	tmp.comand_id   = SFCFG_CMD_ATE_SET_TX_FILTER;
	tmp.length      = 10;
	tmp.sequence    = 1;

	memcpy(tmp.data,tmpdata, 10);

	wrq.u.data.pointer = (caddr_t)&tmp;
	wrq.u.data.length = sizeof(struct sfcfghdr) - sizeof(((struct sfcfghdr *)0)->data) + tmp.length;

	if(do_ioctl(SFCFG_PRIV_IOCTL_ATE, &wrq) < 0){
		printf("SFCFG_PRIV_IOCTL_ATE ioctl SFCFG_CMD_ATE_SET_TX_FILTER failed\n");
		return -1;
	}

	return 0;

}

int32_t set_phy_rate_idx(char *tmpdata)
{
	struct iwreq wrq;
	struct sfcfghdr tmp;
	memset(&wrq, 0, sizeof(struct iwreq));
	memset(&tmp, 0, sizeof(struct sfcfghdr));
	// cmd ate band
	tmp.magic_no = SFCFG_MAGIC_NO;
	tmp.comand_type = SFCFG_CMD_ATE_SET_RATE;
	tmp.comand_id   = SFCFG_CMD_ATE_SET_RATE;
	tmp.length      = 10;
	tmp.sequence    = 1;

	memcpy(tmp.data,tmpdata, 10);
	//printf("tmp.data= %s---------\n", tmp.data);

	wrq.u.data.pointer = (caddr_t)&tmp;
	wrq.u.data.length = sizeof(struct sfcfghdr) - sizeof(((struct sfcfghdr *)0)->data) + tmp.length;

	if(do_ioctl(SFCFG_PRIV_IOCTL_ATE, &wrq) < 0){
		printf("SFCFG_PRIV_IOCTL_ATE ioctl SFCFG_CMD_ATE_SET_RATE failed\n");
		return -1;
	}
	return 0;

}


int32_t set_phy_use_sgi(char *tmpdata)
{
	struct iwreq wrq;
	struct sfcfghdr tmp;
	memset(&wrq, 0, sizeof(struct iwreq));
	memset(&tmp, 0, sizeof(struct sfcfghdr));
	// cmd ate band
	tmp.magic_no = SFCFG_MAGIC_NO;
	tmp.comand_type = SFCFG_CMD_ATE_SET_GI;
	tmp.comand_id   = SFCFG_CMD_ATE_SET_GI;
	tmp.length      = 10;
	tmp.sequence    = 1;

	memcpy(tmp.data,tmpdata, 10);
	//printf("tmp.data= %s---------\n", tmp.data);

	wrq.u.data.pointer = (caddr_t)&tmp;
	wrq.u.data.length = sizeof(struct sfcfghdr) - sizeof(((struct sfcfghdr *)0)->data) + tmp.length;

	if(do_ioctl(SFCFG_PRIV_IOCTL_ATE, &wrq) < 0){
		printf("SFCFG_PRIV_IOCTL_ATE ioctl SFCFG_CMD_ATE_SET_GI failed\n");
		return -1;
	}
	return 0;

}


int32_t set_phy_preamble(char *tmpdata)
{
	struct iwreq wrq;
	struct sfcfghdr tmp;
	memset(&wrq, 0, sizeof(struct iwreq));
	memset(&tmp, 0, sizeof(struct sfcfghdr));
	// cmd ate band
	tmp.magic_no = SFCFG_MAGIC_NO;
	tmp.comand_type = SFCFG_CMD_ATE_SET_PREAMBLE;
	tmp.comand_id   = SFCFG_CMD_ATE_SET_PREAMBLE;
	tmp.length      = 10;
	tmp.sequence    = 1;

	memcpy(tmp.data,tmpdata, 10);
	//printf("tmp.data= %s---------\n", tmp.data);

	wrq.u.data.pointer = (caddr_t)&tmp;
	wrq.u.data.length = sizeof(struct sfcfghdr) - sizeof(((struct sfcfghdr *)0)->data) + tmp.length;

	if(do_ioctl(SFCFG_PRIV_IOCTL_ATE, &wrq) < 0){
		printf("SFCFG_PRIV_IOCTL_ATE ioctl SFCFG_CMD_ATE_SET_PREAMBLE failed\n");
		return -1;
	}
	return 0;

}
/* *
 *gain mode value
 *0-----------use factory table
 *1-----------use low table
 *2-----------use sleep table
 *3-----------use normal tabele
 *band vlaue
 *0-----------undifferentiated antenna
 *1-----------ant1
 *2-----------ant2
 * */
int32_t get_factory_power(char *channel,char *bw,char *mode,char *mcs,char *gain_mode,char *band){
	struct iwreq wrq;
	struct sfcfghdr tmp;
	int power = 0;
	memset(&wrq, 0, sizeof(struct iwreq));
	memset(&tmp, 0, sizeof(struct sfcfghdr));
	//cmd get factory power;
	tmp.magic_no = SFCFG_MAGIC_NO;
	tmp.comand_type = SFCFG_CMD_ATE_GET_POWER;
	tmp.comand_id   = SFCFG_CMD_ATE_GET_POWER;
	tmp.length      = 20;
	tmp.sequence    = 1;

	sprintf(tmp.data,"%s %s %s %s %s %s",channel,bw,mode,mcs,gain_mode,band);
	printf("tmp.data==== %s--------------\n",tmp.data);
	wrq.u.data.pointer = (caddr_t)&tmp;
	wrq.u.data.length = sizeof(struct sfcfghdr) - sizeof(((struct sfcfghdr *)0)->data) + tmp.length;

#ifdef NAND_SUPPORT
	int _channel, _mode, _rate, _bw, _gain_mode, _ant;
	int offset = 0;
	int ret;

	sscanf(tmp.data, "%d %d %d %d %d %d", &_channel, &_bw, &_mode, &_rate, &_gain_mode, &_ant);
	// printk("_channel : %d,_bw :%d,_mode :%d,_rate:%d", _channel, _bw, _mode, _rate);
	if (_channel < 3000) {
		offset = (_channel - 2412) / 5 * 52;
		switch (_mode) {
		case 0:
			offset += _rate;
			break;
		case 2:
			if (_bw == 1)
				offset += 12 + _rate;
			else
				offset += 20 + _rate;
			break;
		case 5:
			if (_bw == 1)
				offset += 28 + _rate;
			else
				offset += 40 + _rate;
			break;
		}
	} else {
		switch (_channel) {
		case 5180:
			offset = 676 + 101 * 0;
			break;
		case 5200:
			offset = 676 + 101 * 1;
			break;
		case 5220:
			offset = 676 + 101 * 2;
			break;
		case 5240:
			offset = 676 + 101 * 3;
			break;
		case 5260:
			offset = 676 + 101 * 4;
			break;
		case 5280:
			offset = 676 + 101 * 5;
			break;
		case 5300:
			offset = 676 + 101 * 6;
			break;
		case 5320:
			offset = 676 + 101 * 7;
			break;
		case 5500:
			offset = 676 + 101 * 8;
			break;
		case 5520:
			offset = 676 + 101 * 9;
			break;
		case 5540:
			offset = 676 + 101 * 10;
			break;
		case 5560:
			offset = 676 + 101 * 11;
			break;
		case 5580:
			offset = 676 + 101 * 12;
			break;
		case 5600:
			offset = 676 + 101 * 13;
			break;
		case 5620:
			offset = 676 + 101 * 14;
			break;
		case 5640:
			offset = 676 + 101 * 15;
			break;
		case 5660:
			offset = 676 + 101 * 16;
			break;
		case 5680:
			offset = 676 + 101 * 17;
			break;
		case 5700:
			offset = 676 + 101 * 18;
			break;
		case 5720:
			offset = 676 + 101 * 19;
			break;
		case 5745:
			offset = 676 + 101 * 20;
			break;
		case 5765:
			offset = 676 + 101 * 21;
			break;
		case 5785:
			offset = 676 + 101 * 22;
			break;
		case 5805:
			offset = 676 + 101 * 23;
			break;
		case 5825:
			offset = 676 + 101 * 24;
			break;
		}
		switch (_mode) {
		case 0:
			offset += _rate;
			break;
		case 2:
			if (_bw == 1)
				offset += 8 + _rate;
			else
				offset += 16 + _rate;
			break;
		case 4:
			if (_bw == 1)
				offset += 24 + _rate;
			else if (_bw == 2)
				offset += 33 + _rate;
			else
				offset += 43 + _rate;
			break;
		case 5:
			if (_bw == 1)
				offset += 53 + _rate;
			else if (_bw == 2)
				offset += 65 + _rate;
			else if (_bw == 3)
				offset += 77 + _rate;
			else
				offset += 89 + _rate;
			break;
		}
	}

	if (_ant == 1) {
		offset += 2052;
	} else if (_ant == 2) {
		offset += 5253;
	}

	ret = flash_init();
	if (ret)
		return ret;
	ret = flash_read(wrq.u.data.pointer, offset, tmp.length);
	if (ret)
		return ret;
#else
	if(do_ioctl(SFCFG_PRIV_IOCTL_ATE, &wrq) < 0){
		printf("SFCFG_PRIV_IOCTL_ATE ioctl SFCFG_CMD_ATE_GET_POWER failed\n");
		return -1;
	}
#endif
	memcpy(&power, wrq.u.data.pointer, 4);

	return power;
}

int32_t get_data_from_factory(int32_t offset){
       struct iwreq wrq;
       struct sfcfghdr tmp;
       int32_t data = 0;
       memset(&wrq, 0, sizeof(struct iwreq));
       memset(&tmp, 0, sizeof(struct sfcfghdr));
       //cmd get factory power;
       tmp.magic_no = SFCFG_MAGIC_NO;
       tmp.comand_type = SFCFG_CMD_ATE_GET_DATA_FROM_FLASH;
       tmp.comand_id   = SFCFG_CMD_ATE_GET_DATA_FROM_FLASH;
       tmp.length      = 10;
       tmp.sequence    = 1;

       sprintf(tmp.data,"%d",offset);
       wrq.u.data.pointer = (caddr_t)&tmp;
       wrq.u.data.length = sizeof(struct sfcfghdr) - sizeof(((struct sfcfghdr *)0)->data) + tmp.length;

#ifdef NAND_SUPPORT
	int ret;

	ret = flash_init();
	if (ret)
		return ret;
	ret = flash_read(wrq.u.data.pointer, offset, tmp.length);
	if (ret)
		return ret;
#else
       if(do_ioctl(SFCFG_PRIV_IOCTL_ATE, &wrq) < 0){
               printf("SFCFG_PRIV_IOCTL_ATE ioctl SFCFG_CMD_ATE_GET_DATA_FROM_FLASH failed\n");
               return -1;
       }
#endif
       memcpy(&data, wrq.u.data.pointer, 4);
       return data;
}

int32_t get_temp_from_factory(void){
       int32_t offset = 174;
       int32_t data = 0;
       data = get_data_from_factory(offset);
       if(data == -1)
               return -1;
       printf("get temp is: %d\r\n",data);
       return 0;
}

int32_t get_refpower_from_factory(void){
       int32_t offset = 186;
       int32_t data = 0;
       data = get_data_from_factory(offset);
       if(data == -1)
               return -1;
       printf("get refpower is: %d\r\n",data);
       return 0;
}

int32_t get_calipower_from_factory(void){
       int32_t offset = 188;
       int32_t data5 = 0;
       int32_t data2 = 0;
       data5 = get_data_from_factory(offset);
       data2 = get_data_from_factory(offset + 1);
       if((data5 == -1) || (data2 == -1))
               return -1;
       printf("get calipower 5G is: %d 2.4G is: %d \r\n", data5, data2);
       return 0;
}

int32_t get_wifi_version_from_factory(void){
       int32_t offset = 2048;
       int32_t data[2] = {0};
	   char value[2];
	   int32_t i = 0;

       for(i = 0; i < 2; i ++){
               data[i] = get_data_from_factory(offset + i);
               if(data[i] == -1)
                       return -1;
               value[i] = toascii(data[i]);
       }
       printf("get wifi version is: %c%c\r\n", value[0], value[1]);

       return 0;
}

int32_t get_trx_delay_from_factory(void){
       int32_t offset = 178;
       int32_t data[4] = {0};
       char value[4];
       int32_t i = 0;

       for(i = 0; i < 4; i ++){
               data[i] = get_data_from_factory(offset + i);
               if(data[i] == -1)
                       return -1;
               value[i] = toascii(data[i]);
       }
       printf("get tx dalay is:0x%c%c\r\n",value[0],value[1]);
       printf("get rx dalay is:0x%c%c\r\n",value[2],value[3]);
       return 0;
}

int32_t get_XO_from_factory(void){
       int32_t offset = 2050;
       int32_t data[2] = {0};
       int32_t i = 0;

       for(i = 0; i < 2; i ++){
               data[i] = get_data_from_factory(offset + i);
               if(data[i] == -1)
                       return -1;
       }
       printf("get XO value is:%d %d\r\n",data[0],data[1]);
       return 0;
}

int32_t get_mac_from_factory(void)
{
	int32_t offset = 0;
	int32_t data[6] = {0};
	int32_t i = 0;

	for (i=0; i<6; i++) {
		data[i] = get_data_from_factory(offset + i);
		if(data[i] == -1)
			return -1;
	}
	printf("get macaddr is %x:%x:%x:%x:%x:%x \n", data[0], data[1], data[2], data[3], data[4], data[5]);

	return 0;
}

int32_t get_rssi_from_factory(void)
{
	int32_t offset = 182;
	int32_t data[4] = {0};
	int32_t i = 0;
	int result[4];

	for (i=0; i<4; i++) {
		data[i] = get_data_from_factory(offset + i);
		if(data[i] == -1)
			return -1;
		if (data[i] > 127)
			result[i] = data[i] - 256;
		else
			result[i] = data[i];
	}

	printf("get rssi offset: HB1=%d  HB2=%d  LB1=%d  LB2=%d\n", result[0], result[1], result[2], result[3]);
	return 0;
}

int32_t set_phy_power_verfication(char *channel,char *bw,char *mode,char *rate,uint32_t power,char *band)
{
       uint32_t power_ver = 0;
       uint32_t offset = 0;
       FILE *fp3;
       int i;
       uint32_t  channel_tmp = siwifi_char_to_int(channel);
       uint32_t mode_tmp = siwifi_char_to_int(mode);
       uint32_t bw_tmp = siwifi_char_to_int(bw);
       uint32_t rate_tmp = siwifi_char_to_int(rate);
       uint32_t band_tmp = siwifi_char_to_int(band);
       int tmp_power[CALI_TABLE_ONE_ANT] = {0};

       power_ver = power;

       if(power_ver == FAC_MODE)//-p 200
       {
               power_ver = get_factory_power(channel,bw,mode,rate,"3",band);
               printf("normal power is %d\r\n",power_ver);
       }
       else if(power_ver == NOR_MODE)// -p 201
       {
               power_ver = get_factory_power(channel,bw,mode,rate,"1",band);
               printf("low power is %d\r\n",power_ver);
       }
       else if(power_ver == SLE_MODE)// -p 202
       {
               power_ver = get_factory_power(channel,bw,mode,rate,"2",band);
               printf("sle power is %d\r\n",power_ver);
       }
       else if(power_ver == TMP_MODE)//-p 203
       {
               offset = get_power_offset(channel_tmp,mode_tmp,bw_tmp,rate_tmp);
	       if(band_tmp == ANT2)
                 fp3 = fopen("/etc/atetools/power_save_ant2.txt","r");
	       else
                 fp3 = fopen("/etc/atetools/power_save_ant1.txt","r");

               for(i = 0;i < CALI_TABLE_ONE_ANT;i++)
               {
                       fscanf(fp3,"%d",&tmp_power[i]);
               }
               fclose(fp3);
               power_ver = tmp_power[offset];
               printf("use tmp power is %d\r\n",power_ver);
       }
       else
       {
       		printf("use factory value\r\n");
       }

       return power_ver;
}

int32_t get_factory_info_from_factory(unsigned int offset, unsigned int length, enum factory_info_type type)
{
	int32_t data[33];
	char value[33];
	int i = 0;
	unsigned int int_value = 0;

	memset(data, 0, sizeof(data));
	memset(value, 0, sizeof(value));

	for (i = 0; i < length; i++) {
		data[i] = get_data_from_factory(offset + i);
		if (data[i] == -1)
			return -1;
	}

	printf("Get factory info : ");
	switch (type) {
	case TYPE_MAC:
		for (i = 0; i < length; i++) {
			printf("%02x", (unsigned char)data[i]);
			if (i != length - 1) {
				printf(":");
			}
		}
		printf("\n");
		break;
	case TYPE_HEX:
		printf("0x");
		for (i = 0; i < length; i++) {
			printf("%02x", (unsigned char)data[i]);
		}
		printf("\n");
		break;
	case TYPE_INT:
		int_value = 0;
		for (i = 0; i < length; i++) {
			int_value |= ((unsigned int)data[i] << (8 * i));
		}
		printf("%u\n", int_value);
		break;
	case TYPE_RSSI_OFFSET:
		if (length >= 4) {
			int8_t rssi_values[4];
			for (i = 0; i < 4; i++) {
				rssi_values[i] = (int8_t)data[i];
			}
			printf("%d %d %d %d\n",
				   rssi_values[0], rssi_values[1], 
				   rssi_values[2], rssi_values[3]);
		} else {
			printf("Invalid length for RSSI_OFFSET (expected at least 4 bytes)\n");
		}
		break;
	case TYPE_STR:
	default:
		for (i = 0; i < length; i++) {
			value[i] = toascii(data[i]);
		}
		value[length] = '\0';
		printf("%s\n", value);
		break;
	}

	return 0;
}

int32_t set_phy_power(char *tmpdata,char *channel,char *bw,char *mode,char *rate,char *band)
{
	struct iwreq wrq;
	struct sfcfghdr tmp;
	uint32_t  power = 0;
	uint32_t  power_ver = 0;
	uint32_t  channel_tmp = siwifi_char_to_int(channel);
	uint32_t mode_tmp = siwifi_char_to_int(mode);
	uint32_t bw_tmp = siwifi_char_to_int(bw);
	uint32_t rate_tmp = siwifi_char_to_int(rate);
	uint32_t band_tmp = siwifi_char_to_int(band);
	uint32_t offset  = 0;
	struct calibrate_info tx_info;
	FILE *fp,*fp2;
	int i;
	int save_power[CALI_TABLE_ONE_ANT] = {0};
	unsigned char buffer[6406] = {0};
	memset(&wrq, 0, sizeof(struct iwreq));
	memset(&tmp, 0, sizeof(struct sfcfghdr));
	// cmd ate band
	tmp.magic_no = SFCFG_MAGIC_NO;
	tmp.comand_type = SFCFG_CMD_ATE_SET_TX_POWER;
	tmp.comand_id   = SFCFG_CMD_ATE_SET_TX_POWER;
	tmp.length      = 20;
	tmp.sequence    = 1;

	if(band_tmp == ANT1)
	{
		printf("[ate_cmd]-----now test ant%d tx-----\r\n",band_tmp);
	}
	else if(band_tmp == ANT2)
	{
		printf("[ate_cmd]-----now test ant%d tx-----\r\n",band_tmp);
 	}
	else
	{
		printf("[ate_cmd]-----now test not diff ant-----\r\n");
	}

	if (!strcmp(tmpdata,"factory")){ // use factory power tx
	power = get_factory_power(channel,bw,mode,rate,"0","0");//-p factory
	printf("power is %d\r\n",power);
	snprintf(tmp.data,4,"%d",power);
	}
	else if(!strcmp(tmpdata,"read")){//read factory power
		power=get_factory_power(channel,bw,mode,rate,"0","0");
		printf("power is :%d\n",power);
		return 0;
	}
	else if(!strcmp(tmpdata,"read_all")){//read all power from factory
		if(!read_gain_table_from_mtd(buffer))
		printf("read_gain_table_over\n");
		return 0;
	}
	else if(!strcmp(tmpdata,"save_all")){//save all power to factory
		save_power_table_to_factory(CALI_TABLE_WHOLE,0);
		return 0;
	}
	else{
			memcpy(tmp.data,tmpdata, 20);
			power  = siwifi_char_to_int(tmpdata);
			if(power > CALI_TABLE_MAX_GAIN)
			{
				power_ver = set_phy_power_verfication(channel,bw,mode,rate,power,band);
				printf("get verfication power is %d\r\n",power_ver);
                                snprintf(tmp.data,4,"%d",power_ver);
			}
			else
			{
				memcpy(tmp.data,tmpdata, 20);
				if(channel_tmp != 0)
				{
					offset = get_power_offset(channel_tmp,mode_tmp,bw_tmp,rate_tmp);
				}
				else
				{
					tx_info = get_ate_calibrate_info();
					printf("channel_tx is %d\n",tx_info.channel);
					printf("mode_tx is %d\n",tx_info.mode);
					printf("bw_tx is %d\n",tx_info.bw);
					printf("rate_tx is %d\n",tx_info.rate);

					offset = get_power_offset(tx_info.channel,tx_info.mode,tx_info.bw,tx_info.rate);
				}
				power_table[offset] = power & 0xff;
				//printf("power :%d offset :%d power :%d \n",power,offset,power_table[offset]);
				/*read from power_save.txt*/
				if(band_tmp == ANT2)
				{
				  fp2 = fopen("/etc/atetools/power_save_ant2.txt","r");
				}
				else
				{
				  fp2 = fopen("/etc/atetools/power_save_ant1.txt","r");
				}

				for(i = 0;i < CALI_TABLE_ONE_ANT; i++)
				{
					fscanf(fp2,"%d",&save_power[i]);
				}
				fclose(fp2);
    				/*write to power_save.txt*/
				if(band_tmp == ANT2)
				{
				  fp = fopen("/etc/atetools/power_save_ant2.txt","w");
				}
				else
				{
				  fp = fopen("/etc/atetools/power_save_ant1.txt","w");
				}
				save_power[offset] = power;

				for(i = 0; i < CALI_TABLE_ONE_ANT; i++)
				{
					if (i < CALI_TABLE_2G_SIZE)
					{
						fprintf(fp,"%d ",save_power[i]);
							if((i+1)%52 == 0){
								if(i == 0)
									continue;
			  					fprintf(fp,"\n");
							}
					}
					else
					{
						fprintf(fp,"%d ",save_power[i]);
							if((i-675)%101 == 0)
                                                                fprintf(fp,"\n");
					}
				}
				fclose(fp);
		}
	}
	wrq.u.data.pointer = (caddr_t)&tmp;
	wrq.u.data.length = sizeof(struct sfcfghdr) - sizeof(((struct sfcfghdr *)0)->data) + tmp.length;

	if(do_ioctl(SFCFG_PRIV_IOCTL_ATE, &wrq) < 0){
		printf("SFCFG_PRIV_IOCTL_ATE ioctl SFCFG_CMD_ATE_SET_TX_POWER failed\n");
		return -1;
	}
	return 0;

}


int32_t set_pkg_bandwidth(char *tmpdata)
{
	struct iwreq wrq;
	struct sfcfghdr tmp;
	memset(&wrq, 0, sizeof(struct iwreq));
	memset(&tmp, 0, sizeof(struct sfcfghdr));
	// cmd ate bandwidth
	tmp.magic_no = SFCFG_MAGIC_NO;
	tmp.comand_type = SFCFG_CMD_ATE_TX_FRAME_BW;
	tmp.comand_id   = SFCFG_CMD_ATE_TX_FRAME_BW;
	tmp.length      = 10;
	tmp.sequence    = 1;

	memcpy(tmp.data,tmpdata,10);
	//printf("tmp.data= %s---------\n",tmp.data);

	wrq.u.data.pointer = (caddr_t)&tmp;
	wrq.u.data.length = sizeof(struct sfcfghdr) - sizeof(((struct sfcfghdr *)0)->data) + tmp.length;

	if(do_ioctl(SFCFG_PRIV_IOCTL_ATE, &wrq) < 0){
		printf("SFCFG_PRIV_IOCTL_ATE ioctl SFCFG_CMD_ATE_TX_FRAME_BW failed\n");
		return -1;
	}
	return 0;
}

int save_xo_to_file(char *tmpdata)
{
	int tmp_data;
	uint8_t xo_value = 0;
	FILE *fp = fopen("/lib/firmware/siwifi_settings.ini", "r+");

	tmp_data = siwifi_char_to_int(tmpdata);
	xo_value = tmp_data & 0x7f;

	printf("[***debug***] %s  tmp_data = %d  xo_value = 0x%x\n", __func__, tmp_data, xo_value);

	fseek(fp, -3, SEEK_END);
	fprintf(fp, "%x", xo_value);

	fclose(fp);

	return 0;
}

// TODO: save xo_value in factory partition.
int32_t set_xo_value(char *tmpdata)
{
	struct iwreq wrq;
	struct sfcfghdr tmp;
	printf("%s\n",__func__);
	memset(&wrq, 0, sizeof(struct iwreq));
	memset(&tmp, 0, sizeof(struct sfcfghdr));
	// cmd ate band
	tmp.magic_no = SFCFG_MAGIC_NO;
	tmp.comand_type = SFCFG_CMD_ATE_XO_VALUE;
	tmp.comand_id   = SFCFG_CMD_ATE_XO_VALUE;
	tmp.length      = 10;
	tmp.sequence    = 1;

	memcpy(tmp.data,tmpdata, 10);
	printf("XO_VALUE = %s---------\n", tmp.data);

	wrq.u.data.pointer = (caddr_t)&tmp;
	wrq.u.data.length = sizeof(struct sfcfghdr) - sizeof(((struct sfcfghdr *)0)->data) + tmp.length;

	if(do_ioctl(SFCFG_PRIV_IOCTL_ATE, &wrq) < 0){
		printf("SFCFG_PRIV_IOCTL_ATE ioctl SFCFG_CMD_ATE_XO_VALUE failed\n");
		return -1;
	}

	save_xo_to_file(tmpdata);

	return 0;

}

int32_t rf_temp_get(void)
{
	struct iwreq wrq;
	struct sfcfghdr tmp;
	//printf("%s\n",__func__);
	memset(&wrq, 0, sizeof(struct iwreq));
	memset(&tmp, 0, sizeof(struct sfcfghdr));
	// cmd ate band
	tmp.magic_no = SFCFG_MAGIC_NO;
	tmp.comand_type = SFCFG_CMD_ATE_RF_TEMP_GET;
	tmp.comand_id   = SFCFG_CMD_ATE_RF_TEMP_GET;
	tmp.length      = 10;
	tmp.sequence    = 1;

	//printf("AET_SETTING = %s---------\n", tmp.data);

	wrq.u.data.pointer = (caddr_t)&tmp;
	wrq.u.data.length = sizeof(struct sfcfghdr) - sizeof(((struct sfcfghdr *)0)->data) + tmp.length;

	if(do_ioctl(SFCFG_PRIV_IOCTL_ATE, &wrq) < 0){
		printf("SFCFG_PRIV_IOCTL_ATE ioctl RF_TEMP_GET fail\n");
		return -1;
	}
	return 0;
}

int32_t set_pkg_frame_length(char *tmpdata)
{
	struct iwreq wrq;
	struct sfcfghdr tmp;
#if PRINT_ATE_LOG
	printf("%s\n",__func__);
#endif
	memset(&wrq, 0, sizeof(struct iwreq));
	memset(&tmp, 0, sizeof(struct sfcfghdr));
	// cmd ate band
	tmp.magic_no = SFCFG_MAGIC_NO;
	tmp.comand_type = SFCFG_CMD_ATE_PAYLOAD_LENGTH;
	tmp.comand_id   = SFCFG_CMD_ATE_PAYLOAD_LENGTH;
	tmp.length      = 10;
	tmp.sequence    = 1;

	memcpy(tmp.data,tmpdata, 10);
	//printf("tmp.data= %s---------\n", tmp.data);

	wrq.u.data.pointer = (caddr_t)&tmp;
	wrq.u.data.length = sizeof(struct sfcfghdr) - sizeof(((struct sfcfghdr *)0)->data) + tmp.length;

	if(do_ioctl(SFCFG_PRIV_IOCTL_ATE, &wrq) < 0){
		printf("SFCFG_PRIV_IOCTL_ATE ioctl SFCFG_CMD_ATE_PAYLOAD_LENGTH failed\n");
		return -1;
	}
	return 0;

}

int32_t ant_num_set(char *tmpdata)
{
	struct iwreq wrq;
	struct sfcfghdr tmp;
#if PRINT_ATE_LOG
	printf("%s\n",__func__);
#endif
	memset(&wrq, 0, sizeof(struct iwreq));
	memset(&tmp, 0, sizeof(struct sfcfghdr));
	// cmd ate band
	tmp.magic_no = SFCFG_MAGIC_NO;
	tmp.comand_type = SFCFG_CMD_ATE_ANT_NUM;
	tmp.comand_id   = SFCFG_CMD_ATE_ANT_NUM;
	tmp.length      = 10;
	tmp.sequence    = 1;

	memcpy(tmp.data,tmpdata, 10);
	//printf("tmp.data= %s---------\n", tmp.data);

	wrq.u.data.pointer = (caddr_t)&tmp;
	wrq.u.data.length = sizeof(struct sfcfghdr) - sizeof(((struct sfcfghdr *)0)->data) + tmp.length;

	if(do_ioctl(SFCFG_PRIV_IOCTL_ATE, &wrq) < 0){
		printf("SFCFG_PRIV_IOCTL_ATE ioctl SFCFG_CMD_ATE_PAYLOAD_LENGTH failed\n");
		return -1;
	}
	return 0;

}

int32_t set_pkg_frame_num(char *tmpdata)
{
	struct iwreq wrq;
	struct sfcfghdr tmp;
	memset(&wrq, 0, sizeof(struct iwreq));
	memset(&tmp, 0, sizeof(struct sfcfghdr));
	// cmd ate num
	tmp.magic_no = SFCFG_MAGIC_NO;
	tmp.comand_type = SFCFG_CMD_ATE_TX_COUNT;
	tmp.comand_id   = SFCFG_CMD_ATE_TX_COUNT;
	tmp.length      = 10;
	tmp.sequence    = 1;

	strncpy(tmp.data,tmpdata,10);
	//printf("tmp.data= %s---------\n", tmp.data);

	wrq.u.data.pointer = (caddr_t)&tmp;
	wrq.u.data.length = sizeof(struct sfcfghdr) - sizeof(((struct sfcfghdr *)0)->data) + tmp.length;

	if(do_ioctl(SFCFG_PRIV_IOCTL_ATE, &wrq) < 0){
		printf("SFCFG_PRIV_IOCTL_ATE ioctl SFCFG_CMD_ATE_SET_TX_COUNT failed\n");
		return -1;
	}
	return 0;
}

int32_t set_pkg_macbypass_interval(char *tmpdata)
{
	struct iwreq wrq;
	struct sfcfghdr tmp;
	memset(&wrq, 0, sizeof(struct iwreq));
	memset(&tmp, 0, sizeof(struct sfcfghdr));
	// cmd ate num
	tmp.magic_no = SFCFG_MAGIC_NO;
	tmp.comand_type = SFCFG_CMD_ATE_MACBYPASS_INTERVAL;
	tmp.comand_id   = SFCFG_CMD_ATE_MACBYPASS_INTERVAL;
	tmp.length      = 10;
	tmp.sequence    = 1;

	strncpy(tmp.data,tmpdata,10);
	//printf("tmp.data= %s---------\n", tmp.data);

	wrq.u.data.pointer = (caddr_t)&tmp;
	wrq.u.data.length = sizeof(struct sfcfghdr) - sizeof(((struct sfcfghdr *)0)->data) + tmp.length;

	if(do_ioctl(SFCFG_PRIV_IOCTL_ATE, &wrq) < 0){
		printf("SFCFG_PRIV_IOCTL_ATE ioctl SFCFG_CMD_ATE_MACBYPASS_INTERVAL failed\n");
		return -1;
	}
	return 0;
}

int32_t set_pkg_fc(char *tmpdata)
{
	struct iwreq wrq;
	struct sfcfghdr tmp;
	memset(&wrq, 0, sizeof(struct iwreq));
	memset(&tmp, 0, sizeof(struct sfcfghdr));
	// cmd ate band
	tmp.magic_no = SFCFG_MAGIC_NO;
	tmp.comand_type = SFCFG_CMD_ATE_TX_FC;
	tmp.comand_id   = SFCFG_CMD_ATE_TX_FC;
	tmp.length      = 20;
	tmp.sequence    = 1;

	memcpy(tmp.data,tmpdata, 20);
	//printf("tmp.data= %s---------\n", tmp.data);

	wrq.u.data.pointer = (caddr_t)&tmp;
	wrq.u.data.length = sizeof(struct sfcfghdr) - sizeof(((struct sfcfghdr *)0)->data) + tmp.length;

	if(do_ioctl(SFCFG_PRIV_IOCTL_ATE, &wrq) < 0){
		printf("SFCFG_PRIV_IOCTL_ATE ioctl SFCFG_CMD_ATE_SET_TX_FC failed\n");
		return -1;
	}
	return 0;

}


int32_t set_pkg_dur(char *tmpdata)
{
	struct iwreq wrq;
	struct sfcfghdr tmp;
	memset(&wrq, 0, sizeof(struct iwreq));
	memset(&tmp, 0, sizeof(struct sfcfghdr));
	// cmd ate band
	tmp.magic_no = SFCFG_MAGIC_NO;
	tmp.comand_type = SFCFG_CMD_ATE_TX_DUR;
	tmp.comand_id   = SFCFG_CMD_ATE_TX_DUR;
	tmp.length      = 20;
	tmp.sequence    = 1;

	memcpy(tmp.data,tmpdata, 20);
	//printf("tmp.data= %s---------\n", tmp.data);

	wrq.u.data.pointer = (caddr_t)&tmp;
	wrq.u.data.length = sizeof(struct sfcfghdr) - sizeof(((struct sfcfghdr *)0)->data) + tmp.length;

	if(do_ioctl(SFCFG_PRIV_IOCTL_ATE, &wrq) < 0){
		printf("SFCFG_PRIV_IOCTL_ATE ioctl SFCFG_CMD_ATE_SET_TX_DUR failed\n");
		return -1;
	}
	return 0;

}


int32_t set_pkg_bssid(char *tmpdata)
{
	struct iwreq wrq;
	struct sfcfghdr tmp;
	memset(&wrq, 0, sizeof(struct iwreq));
	memset(&tmp, 0, sizeof(struct sfcfghdr));
	// cmd ate band
	tmp.magic_no = SFCFG_MAGIC_NO;
	tmp.comand_type = SFCFG_CMD_ATE_TX_BSSID;
	tmp.comand_id   = SFCFG_CMD_ATE_TX_BSSID;
	tmp.length      = 20;
	tmp.sequence    = 1;

	memcpy(tmp.data,tmpdata, 20);
	//printf("tmp.data= %s---------\n", tmp.data);

	wrq.u.data.pointer = (caddr_t)&tmp;
	wrq.u.data.length = sizeof(struct sfcfghdr) - sizeof(((struct sfcfghdr *)0)->data) + tmp.length;

	if(do_ioctl(SFCFG_PRIV_IOCTL_ATE, &wrq) < 0){
		printf("SFCFG_PRIV_IOCTL_ATE ioctl SFCFG_CMD_ATE_TX_BSSID failed\n");
		return -1;
	}
	return 0;

}


int32_t set_pkg_da(char *tmpdata)
{
	struct iwreq wrq;
	struct sfcfghdr tmp;
	memset(&wrq, 0, sizeof(struct iwreq));
	memset(&tmp, 0, sizeof(struct sfcfghdr));
	// cmd ate band
	tmp.magic_no = SFCFG_MAGIC_NO;
	tmp.comand_type = SFCFG_CMD_ATE_TX_DA;
	tmp.comand_id   = SFCFG_CMD_ATE_TX_DA;
	tmp.length      = 20;
	tmp.sequence    = 1;

	memcpy(tmp.data,tmpdata, 20);
	//printf("tmp.data= %s---------\n", tmp.data);

	wrq.u.data.pointer = (caddr_t)&tmp;
	wrq.u.data.length = sizeof(struct sfcfghdr) - sizeof(((struct sfcfghdr *)0)->data) + tmp.length;

	if(do_ioctl(SFCFG_PRIV_IOCTL_ATE, &wrq) < 0){
		printf("SFCFG_PRIV_IOCTL_ATE ioctl SFCFG_CMD_ATE_TX_DA failed\n");
		return -1;
	}
	return 0;

}


int32_t set_pkg_sa(char *tmpdata)
{
	struct iwreq wrq;
	struct sfcfghdr tmp;
	memset(&wrq, 0, sizeof(struct iwreq));
	memset(&tmp, 0, sizeof(struct sfcfghdr));
	// cmd ate band
	tmp.magic_no = SFCFG_MAGIC_NO;
	tmp.comand_type = SFCFG_CMD_ATE_TX_SA;
	tmp.comand_id   = SFCFG_CMD_ATE_TX_SA;
	tmp.length      = 20;
	tmp.sequence    = 1;

	memcpy(tmp.data,tmpdata, 20);
	//printf("tmp.data= %s---------\n", tmp.data);

	wrq.u.data.pointer = (caddr_t)&tmp;
	wrq.u.data.length = sizeof(struct sfcfghdr) - sizeof(((struct sfcfghdr *)0)->data) + tmp.length;

	if(do_ioctl(SFCFG_PRIV_IOCTL_ATE, &wrq) < 0){
		printf("SFCFG_PRIV_IOCTL_ATE ioctl SFCFG_CMD_ATE_TX_SA failed\n");
		return -1;
	}
	return 0;

}
int32_t set_txvector_param(char *tmpdata){
	struct iwreq wrq;
	struct sfcfghdr tmp;
	memset(&wrq, 0, sizeof(struct iwreq));
	memset(&tmp, 0, sizeof(struct sfcfghdr));
	// cmd ate band
	tmp.magic_no = SFCFG_MAGIC_NO;
	tmp.comand_type = SFCFG_CMD_ATE_TXVECTOR_PARAM;
	tmp.comand_id   = SFCFG_CMD_ATE_TXVECTOR_PARAM_NSS;
	tmp.length      = 20;
	tmp.sequence    = 1;

	memcpy(tmp.data,tmpdata, 20);
	//printf("tmp.data= %s---------\n", tmp.data);

	wrq.u.data.pointer = (caddr_t)&tmp;
	wrq.u.data.length = sizeof(struct sfcfghdr) - sizeof(((struct sfcfghdr *)0)->data) + tmp.length;

	if(do_ioctl(SFCFG_PRIV_IOCTL_ATE, &wrq) < 0){
		printf("SFCFG_PRIV_IOCTL_ATE ioctl SFCFG_CMD_ATE_TX_SA failed\n");
		return -1;
	}
	return 0;
}

int32_t set_pkg_seqc(char *tmpdata)
{
	struct iwreq wrq;
	struct sfcfghdr tmp;
	memset(&wrq, 0, sizeof(struct iwreq));
	memset(&tmp, 0, sizeof(struct sfcfghdr));
	// cmd ate band
	tmp.magic_no = SFCFG_MAGIC_NO;
	tmp.comand_type = SFCFG_CMD_ATE_TX_SEQC;
	tmp.comand_id   = SFCFG_CMD_ATE_TX_SEQC;
	tmp.length      = 20;
	tmp.sequence    = 1;

	memcpy(tmp.data,tmpdata, 20);
	//printf("tmp.data= %s---------\n", tmp.data);

	wrq.u.data.pointer = (caddr_t)&tmp;
	wrq.u.data.length = sizeof(struct sfcfghdr) - sizeof(((struct sfcfghdr *)0)->data) + tmp.length;

	if(do_ioctl(SFCFG_PRIV_IOCTL_ATE, &wrq) < 0){
		printf("SFCFG_PRIV_IOCTL_ATE ioctl SFCFG_CMD_ATE_SET_PHY_MODE failed\n");
		return -1;
	}
	return 0;
}

int32_t get_rx_info(void)
{
	struct iwreq wrq;
	struct sfcfghdr tmp;
	struct rwnx_ioctl_ate_dump_info user;
	//read results from driver
	//printf("show PER, RSSI!\n");
	memset(&wrq, 0, sizeof(struct iwreq));
	memset(&tmp, 0, sizeof(struct sfcfghdr));
	memset(&user, 0, sizeof(struct rwnx_ioctl_ate_dump_info));

	tmp.magic_no = SFCFG_MAGIC_NO;
	tmp.comand_type = SFCFG_CMD_ATE_GET_INFO;
	tmp.comand_id   = SFCFG_CMD_ATE_GET_INFO;
	tmp.length      = 10;
	tmp.sequence    = 1;

	wrq.u.data.pointer = (caddr_t)&tmp;
	wrq.u.data.length = sizeof(struct sfcfghdr) - sizeof(((struct sfcfghdr *)0)->data) + tmp.length;
	if(do_ioctl(SFCFG_PRIV_IOCTL_ATE,&wrq) < 0){
		printf("SFCFG_CMD_ATE_GET_INFO ioctl failed\n");
		return -1;
	}

	memset(&user, 0, sizeof(struct rwnx_ioctl_ate_dump_info));
	memcpy(&user, wrq.u.data.pointer, sizeof(struct rwnx_ioctl_ate_dump_info));

	printf("receive 20M OK = %d, receive 40M OK = %d, receive 80M OK = %d, receive 160M OK = %d\n", user.mib_20m, user.mib_40m, user.mib_80m, user.mib_160m);
	printf("rssi_1 = %d, rssi_2 = %d\n", user.rssi_1, user.rssi_2);

	// printf("receive data = %d,fcs ok  = %d,fcs error = %d,phy error = %d,fcs_ok_for_mac_addr = %d,fcs_group = %d\n",user.rec_rx_count, user.fcs_ok, user.fcs_err, user.phy_err, user.fcs_ok_for_macaddr, user.fcs_group);
	return 0;
}


int32_t set_pkg_whole_frame(char beacon[], int length_array)
{
	struct iwreq wrq;
	struct sfcfghdr tmp;
	int i = 0;
	//fill beacon mac header
	memset(&wrq, 0, sizeof(struct iwreq));
	memset(&tmp, 0, sizeof(struct sfcfghdr));
	//cmd ate band
	tmp.magic_no = SFCFG_MAGIC_NO;
	tmp.comand_type = SFCFG_CMD_ATE_WHOLE_FRAME;
	tmp.comand_id   = SFCFG_CMD_ATE_WHOLE_FRAME;
	tmp.length      = length_array;
	tmp.sequence    = 1;

	printf("%s: length_array = %d\n",__func__,length_array);
	memcpy(tmp.data, beacon, length_array);

	for(i = 0; i <= length_array; i++)
	{
		printf("%s: tmp.data[%d] = 0x%02x\n",__func__,i,tmp.data[i]);
	}

	wrq.u.data.pointer = (caddr_t)&tmp;
	wrq.u.data.length = sizeof(struct sfcfghdr) - sizeof(((struct sfcfghdr *)0)->data) + tmp.length;

	if(do_ioctl(SFCFG_PRIV_IOCTL_ATE, &wrq) < 0){
		printf("SFCFG_PRIV_IOCTL_ATE ioctl SFCFG_CMD_ATE_SET_PHY_MODE failed\n");
		return -1;
	}
	return 0;
}


int32_t set_pkg_payload(char *tmpdata)
{
	struct iwreq wrq;
	struct sfcfghdr tmp;
	//int i = 0;
	printf("~~~set_pkg_payload,%u\n",sizeof(tmpdata));
	//fill the frame payload
	memset(&wrq, 0, sizeof(struct iwreq));
	memset(&tmp, 0, sizeof(struct sfcfghdr));

	//cmd ate band
	tmp.magic_no = SFCFG_MAGIC_NO;
	tmp.comand_type = SFCFG_CMD_ATE_PAYLOAD;
	tmp.comand_id   = SFCFG_CMD_ATE_PAYLOAD;
	tmp.length      = sizeof(tmpdata);
	tmp.sequence    = 1;

	memcpy(tmp.data, tmpdata, tmp.length);
	wrq.u.data.pointer = (caddr_t)&tmp;
	wrq.u.data.length = sizeof(struct sfcfghdr) - sizeof(((struct sfcfghdr *)0)->data) + tmp.length;
	if(do_ioctl(SFCFG_PRIV_IOCTL_ATE, &wrq) < 0){
		printf("SFCFG_PRIV_IOCTL_ATE ioctl SFCFG_CMD_ATE_SET_PAYLOAD failed\n");
		return -1;
	}
	return 0;
}

int32_t send_tx_test_tone_start()
{
	struct iwreq wrq;
	struct sfcfghdr tmp;

	printf("%s>>\n", __func__);

	memset(&wrq, 0, sizeof(struct iwreq));
	memset(&tmp, 0, sizeof(struct sfcfghdr));

	tmp.magic_no = SFCFG_MAGIC_NO;
	tmp.comand_type = SFCFG_CMD_ATE_TX_TEST_TONE_START;
	tmp.comand_id   = SFCFG_CMD_ATE_TX_TEST_TONE_START;
	tmp.length      = 10;
	tmp.sequence    = 1;

	strncpy(tmp.data, "true", 10);

	wrq.u.data.pointer = (caddr_t)&tmp;

	//12 is the sizeof sfcfghdr except data field, 10 is the actual data
	wrq.u.data.length = sizeof(struct sfcfghdr) - sizeof(((struct sfcfghdr *)0)->data) + tmp.length;

	if(do_ioctl(SFCFG_PRIV_IOCTL_ATE, &wrq) < 0){
		printf("SFCFG_PRIV_IOCTL_ATE ioctl SFCFG_CMD_ATE_TX_TEST_TONE_START failed\n");
		return -1;
	}
	return 0;
}

int32_t send_tx_test_tone_stop()
{
	struct iwreq wrq;
	struct sfcfghdr tmp;

	printf("%s>>\n", __func__);

	memset(&wrq, 0, sizeof(struct iwreq));
	memset(&tmp, 0, sizeof(struct sfcfghdr));

	tmp.magic_no = SFCFG_MAGIC_NO;
	//first call SFCFG_CMD_ATE_START
	tmp.comand_type = SFCFG_CMD_ATE_TX_TEST_TONE_STOP;
	tmp.comand_id   = SFCFG_CMD_ATE_TX_TEST_TONE_STOP;
	tmp.length      = 10;
	tmp.sequence    = 1;

	strncpy(tmp.data, "true", 10);

	wrq.u.data.pointer = (caddr_t)&tmp;

	//12 is the sizeof sfcfghdr except data field, 10 is the actual data
	wrq.u.data.length = sizeof(struct sfcfghdr) - sizeof(((struct sfcfghdr *)0)->data) + tmp.length;

	if(do_ioctl(SFCFG_PRIV_IOCTL_ATE, &wrq) < 0){
		printf("SFCFG_PRIV_IOCTL_ATE ioctl SFCFG_CMD_ATE_TX_TEST_TONE_STOP failed\n");
		return -1;
	}
	return 0;
}

int32_t send_ate_start()
{
	struct iwreq wrq;
	struct sfcfghdr tmp;
	//struct tx_frame_param *tx_frame_param = NULL;
	// struct rwnx_ate_config_params *ate_config_params = NULL;
#if PRINT_ATE_LOG
	printf("%s>>\n", __func__);
#endif
	memset(&wrq, 0, sizeof(struct iwreq));
	memset(&tmp, 0, sizeof(struct sfcfghdr));

	tmp.magic_no = SFCFG_MAGIC_NO;
	//first call SFCFG_CMD_ATE_START
	tmp.comand_type = SFCFG_CMD_ATE_START;
	tmp.comand_id   = SFCFG_CMD_ATE_START;
	tmp.length      = 10;
	tmp.sequence    = 1;

	strncpy(tmp.data, "true", 10);

	wrq.u.data.pointer = (caddr_t)&tmp;

	//12 is the sizeof sfcfghdr except data field, 10 is the actual data
	wrq.u.data.length = sizeof(struct sfcfghdr) - sizeof(((struct sfcfghdr *)0)->data) + tmp.length;

	if(do_ioctl(SFCFG_PRIV_IOCTL_ATE, &wrq) < 0){
		//printf("SFCFG_PRIV_IOCTL_ATE ioctl SFCFG_CMD_ATE_START failed\n");
		return -1;
	}
	return 0;
}

int32_t send_ate_stop()
{
	struct iwreq wrq;
	struct sfcfghdr tmp;
	//struct tx_frame_param *tx_frame_param = NULL;
	// struct rwnx_ate_config_params *ate_config_params = NULL;

	printf("%s>>\n", __func__);

	memset(&wrq, 0, sizeof(struct iwreq));
	memset(&tmp, 0, sizeof(struct sfcfghdr));

	tmp.magic_no = SFCFG_MAGIC_NO;
	//first call SFCFG_CMD_ATE_START
	tmp.comand_type = SFCFG_CMD_ATE_STOP;
	tmp.comand_id   = SFCFG_CMD_ATE_STOP;
	tmp.length      = 10;
	tmp.sequence    = 1;

	strncpy(tmp.data, "true", 10);

	wrq.u.data.pointer = (caddr_t)&tmp;

	//12 is the sizeof sfcfghdr except data field, 10 is the actual data
	wrq.u.data.length = sizeof(struct sfcfghdr) - sizeof(((struct sfcfghdr *)0)->data) + tmp.length;

	if(do_ioctl(SFCFG_PRIV_IOCTL_ATE, &wrq) < 0){
		printf("SFCFG_PRIV_IOCTL_ATE ioctl SFCFG_CMD_ATE_STOP failed\n");
		return -1;
	}
	return 0;
}


int32_t send_tx_start(char *bufs)
{
	struct iwreq wrq;
	struct sfcfghdr tmp;
	//struct tx_frame_param *tx_frame_param = NULL;
	// struct rwnx_ate_config_params *ate_config_params = NULL;
	int bufs_length;

	printf("%s>>\n", __func__);

	memset(&wrq, 0, sizeof(struct iwreq));
	memset(&tmp, 0, sizeof(struct sfcfghdr));

	tmp.magic_no = SFCFG_MAGIC_NO;
	//first call SFCFG_CMD_ATE_START
	tmp.comand_type = SFCFG_CMD_ATE_TX_START;
	tmp.comand_id   = SFCFG_CMD_ATE_TX_START;
	tmp.length      = 10;
	tmp.sequence    = 1;

	if(bufs != NULL){
		bufs_length=sizeof(bufs);
		memcpy(tmp.data, bufs, bufs_length);
	}
	else
		strncpy(tmp.data, "true", 10);

	wrq.u.data.pointer = (caddr_t)&tmp;

	//12 is the sizeof sfcfghdr except data field, 10 is the actual data
	wrq.u.data.length = sizeof(struct sfcfghdr) - sizeof(((struct sfcfghdr *)0)->data) + tmp.length;

	if(do_ioctl(SFCFG_PRIV_IOCTL_ATE, &wrq) < 0){
		printf("SFCFG_PRIV_IOCTL_ATE ioctl SFCFG_CMD_ATE_START failed\n");
		return -1;
	}
	return 0;
}


int32_t send_tx_rx_start_by_macbypass()
{
	struct iwreq wrq;
	struct sfcfghdr tmp;
	struct rwnx_ioctl_ate_dump_info user;

	printf("%s>>\n", __func__);

	memset(&wrq, 0, sizeof(struct iwreq));
	memset(&tmp, 0, sizeof(struct sfcfghdr));

	tmp.magic_no = SFCFG_MAGIC_NO;
	//first call SFCFG_CMD_ATE_START
	tmp.comand_type = SFCFG_CMD_ATE_MACBYPASS_TX_RX_START;
	tmp.comand_id   = SFCFG_CMD_ATE_MACBYPASS_TX_RX_START;
	tmp.length      = 10;
	tmp.sequence    = 1;

	strncpy(tmp.data, "true", 10);

	wrq.u.data.pointer = (caddr_t)&tmp;

	//12 is the sizeof sfcfghdr except data field, 10 is the actual data
	wrq.u.data.length = sizeof(struct sfcfghdr) - sizeof(((struct sfcfghdr *)0)->data) + tmp.length;

	if(do_ioctl(SFCFG_PRIV_IOCTL_ATE, &wrq) < 0){
		printf("SFCFG_PRIV_IOCTL_ATE ioctl SFCFG_CMD_ATE_MACBYPASS_TX_RX_START failed\n");
		return -1;
	}
	//read results from driver
#if PRINT_ATE_LOG
	printf("show RX result, RSSI!\n");
#endif
	g_clock_flag = 1;
	return 0;
}


int32_t send_tx_rx_stop_by_macbypass()
{
	struct iwreq wrq;
	struct sfcfghdr tmp;

	printf("%s>>\n", __func__);

	memset(&wrq, 0, sizeof(struct iwreq));
	memset(&tmp, 0, sizeof(struct sfcfghdr));

	tmp.magic_no = SFCFG_MAGIC_NO;
	tmp.comand_type = SFCFG_CMD_ATE_MACBYPASS_TX_RX_STOP;
	tmp.comand_id   = SFCFG_CMD_ATE_MACBYPASS_TX_RX_STOP;
	tmp.length      = 10;
	tmp.sequence    = 1;

	strncpy(tmp.data, "false", 10);

	wrq.u.data.pointer = (caddr_t)&tmp;
	wrq.u.data.length = tmp.length + 16;//12 is the sizeof sfcfghdr except data field, 10 is the actual data

	if(do_ioctl(SFCFG_PRIV_IOCTL_ATE, &wrq) < 0){
		printf("SFCFG_PRIV_IOCTL_ATE ioctl SFCFG_CMD_ATE_MACBYPASS_TX_RX_STOP failed\n");
		return -1;
	}
	return 0;
}


int32_t send_tx_start_by_macbypass()
{
	struct iwreq wrq;
	struct sfcfghdr tmp;

	printf("%s>>\n", __func__);

	memset(&wrq, 0, sizeof(struct iwreq));
	memset(&tmp, 0, sizeof(struct sfcfghdr));

	tmp.magic_no = SFCFG_MAGIC_NO;
	//first call SFCFG_CMD_ATE_START
	tmp.comand_type = SFCFG_CMD_ATE_MACBYPASS_TX_START;
	tmp.comand_id   = SFCFG_CMD_ATE_MACBYPASS_TX_START;
	tmp.length      = 10;
	tmp.sequence    = 1;

	strncpy(tmp.data, "true", 10);

	wrq.u.data.pointer = (caddr_t)&tmp;

	//12 is the sizeof sfcfghdr except data field, 10 is the actual data
	wrq.u.data.length = sizeof(struct sfcfghdr) - sizeof(((struct sfcfghdr *)0)->data) + tmp.length;

	if(do_ioctl(SFCFG_PRIV_IOCTL_ATE, &wrq) < 0){
		printf("SFCFG_PRIV_IOCTL_ATE ioctl SFCFG_CMD_ATE_MACBYPASS_TX_START failed\n");
		return -1;
	}
	return 0;
}


int32_t send_rx_start()
{
	struct iwreq wrq;
	struct sfcfghdr tmp;
	struct rwnx_ioctl_ate_dump_info user;
	uint32_t tmp1 = 0;
	uint32_t tmp2 = 0;
	uint32_t tmp3 = 0;
//	uint32_t per = 0;
	uint32_t tmp_f = 0;
	uint32_t tmp_p = 0;
	uint32_t tmp_r = 0;
	uint32_t tmp_k =0 ;
#if PRINT_ATE_LOG
	printf("%s>>\n", __func__);
#endif
	memset(&wrq, 0, sizeof(struct iwreq));
	memset(&tmp, 0, sizeof(struct sfcfghdr));

	tmp.magic_no = SFCFG_MAGIC_NO;
	//first call SFCFG_CMD_ATE_START
	tmp.comand_type = SFCFG_CMD_ATE_RX_START;
	tmp.comand_id   = SFCFG_CMD_ATE_RX_START;
	tmp.length      = 10;
	tmp.sequence    = 1;

	strncpy(tmp.data, "true", 10);
	wrq.u.data.pointer = (caddr_t)&tmp;

	//12 is the sizeof sfcfghdr except data field, 10 is the actual data
	wrq.u.data.length = sizeof(struct sfcfghdr) - sizeof(((struct sfcfghdr *)0)->data) + tmp.length;

	if(do_ioctl(SFCFG_PRIV_IOCTL_ATE, &wrq) < 0){
		printf("SFCFG_PRIV_IOCTL_ATE ioctl SFCFG_CMD_ATE_RX_START failed\n");
		return -1;
	}

	//read results from driver
#if PRINT_ATE_LOG
	printf("show RX result, RSSI!\n");
#endif
	g_clock_flag = 1;
#if 0
	while(g_clock_flag){
		sleep(3);
		memset(&wrq, 0, sizeof(struct iwreq));
		memset(&tmp, 0, sizeof(struct sfcfghdr));

		tmp.magic_no = SFCFG_MAGIC_NO;
		tmp.comand_type = SFCFG_CMD_ATE_GET_INFO;
		tmp.comand_id   = SFCFG_CMD_ATE_GET_INFO;
		tmp.length      = 10;
		tmp.sequence    = 1;

		wrq.u.data.pointer = (caddr_t)&tmp;
		wrq.u.data.length = sizeof(struct sfcfghdr) - sizeof(((struct sfcfghdr *)0)->data) + tmp.length;
		if(do_ioctl(SFCFG_PRIV_IOCTL_ATE,&wrq) < 0){
			printf("SFCFG_CMD_ATE_GET_INFO ioctl failed\n");
			return -1;
		}
		memset(&user, 0, sizeof(struct rwnx_ioctl_ate_dump_info));
		memcpy(&user, wrq.u.data.pointer, sizeof(struct rwnx_ioctl_ate_dump_info));

		tmp_f = user.fcs_err - tmp1;
		tmp_p = user.phy_err - tmp2;
		tmp_r = user.rec_rx_count - tmp3;
		tmp_k = user.fcs_ok;
		tmp1 = user.fcs_err;
		tmp2 = user.phy_err;
		tmp3 = user.rec_rx_count;

		printf("receive data = %d,fcs_ok= %d,fcs error = %d,phy error = %d\n",user.rec_rx_count,user.fcs_ok,user.fcs_err, user.phy_err);
	}
#endif
	return 0;
}

int32_t send_rx_start_by_macbypass()
{
	struct iwreq wrq;
	struct sfcfghdr tmp;
	struct rwnx_ioctl_ate_dump_info user;
	uint32_t tmp1 = 0;
	uint32_t tmp2 = 0;
	uint32_t tmp3 = 0;
//	uint32_t per = 0;
	uint32_t tmp_f = 0;
	uint32_t tmp_p = 0;
	uint32_t tmp_r = 0;
	uint32_t tmp_k = 0;
	printf("%s>>\n", __func__);

	memset(&wrq, 0, sizeof(struct iwreq));
	memset(&tmp, 0, sizeof(struct sfcfghdr));

	tmp.magic_no = SFCFG_MAGIC_NO;
	tmp.comand_type = SFCFG_CMD_ATE_MACBYPASS_RX_START;
	tmp.comand_id   = SFCFG_CMD_ATE_MACBYPASS_RX_START;
	tmp.length      = 20;
	tmp.sequence    = 1;

	strncpy(tmp.data, "rx start macby", tmp.length);

	wrq.u.data.pointer = (caddr_t)&tmp;

	//12 is the sizeof sfcfghdr except data field, 10 is the actual data
	wrq.u.data.length = sizeof(struct sfcfghdr) - sizeof(((struct sfcfghdr *)0)->data) + tmp.length;

	if(do_ioctl(SFCFG_PRIV_IOCTL_ATE, &wrq) < 0){
		printf("SFCFG_PRIV_IOCTL_ATE ioctl SFCFG_CMD_ATE_MACBYPASS_RX_START failed\n");
		return -1;
	}
	//read results from driver
#if 0
    printf("show PER, RSSI!\n");
	g_clock_flag = 1;
	while(g_clock_flag){
		sleep(3);
		memset(&wrq, 0, sizeof(struct iwreq));
		memset(&tmp, 0, sizeof(struct sfcfghdr));

		tmp.magic_no = SFCFG_MAGIC_NO;
		tmp.comand_type = SFCFG_CMD_ATE_GET_INFO;
		tmp.comand_id   = SFCFG_CMD_ATE_GET_INFO;
		tmp.length      = 10;
		tmp.sequence    = 1;

		wrq.u.data.pointer = (caddr_t)&tmp;
		wrq.u.data.length = sizeof(struct sfcfghdr) - sizeof(((struct sfcfghdr *)0)->data) + tmp.length;
		if(do_ioctl(SFCFG_PRIV_IOCTL_ATE,&wrq) < 0){
			printf("SFCFG_CMD_ATE_GET_INFO ioctl failed\n");
			return -1;
		}
		memset(&user, 0, sizeof(struct rwnx_ioctl_ate_dump_info));
		memcpy(&user, wrq.u.data.pointer, sizeof(struct rwnx_ioctl_ate_dump_info));
		printf("-----------------------------------------------\n");
		tmp_f = user.fcs_err - tmp1;
		tmp_p = user.phy_err - tmp2;
		tmp_r = user.rec_rx_count - tmp3;
		tmp_k = user.fcs_ok;

		tmp1 = user.fcs_err;
		tmp2 = user.phy_err;
		tmp3 = user.rec_rx_count;
		printf("receive data = %d,fcs ok = %d, fcs error = %d,phy error = %d\n",user.rec_rx_count, user.fcs_ok,user.fcs_err, user.phy_err);
	}
#endif
	return 0;
}


int32_t send_tx_stop()
{
	struct iwreq wrq;
	struct sfcfghdr tmp;
	memset(&wrq, 0, sizeof(struct iwreq));
	memset(&tmp, 0, sizeof(struct sfcfghdr));

	tmp.magic_no    = SFCFG_MAGIC_NO;
	tmp.comand_type = SFCFG_CMD_ATE_TX_STOP;
	tmp.comand_id   = SFCFG_CMD_ATE_TX_STOP;
	tmp.length      = 10;
	tmp.sequence    = 1;
	strncpy(tmp.data, "false", 10);

	wrq.u.data.pointer = (caddr_t)&tmp;
	wrq.u.data.length = tmp.length + 16;//12 is the sizeof sfcfghdr except data field, 10 is the actual data

	//call SFCFG_CMD_ATE_TX_STOP
	if(do_ioctl(SFCFG_PRIV_IOCTL_ATE, &wrq) < 0){
		printf("SFCFG_PRIV_IOCTL_ATE ioctl SFCFG_CMD_ATE_TX_STOP failed\n");
		return -1;
	}
	return 0;
}

int32_t send_tx_stop_by_macbypass()
{
	struct iwreq wrq;
	struct sfcfghdr tmp;
	memset(&wrq, 0, sizeof(struct iwreq));
	memset(&tmp, 0, sizeof(struct sfcfghdr));

	tmp.magic_no    = SFCFG_MAGIC_NO;
	tmp.comand_type = SFCFG_CMD_ATE_MACBYPASS_TX_STOP;
	tmp.comand_id   = SFCFG_CMD_ATE_MACBYPASS_TX_STOP;
	tmp.length      = 10;
	tmp.sequence    = 1;
	strncpy(tmp.data, "false", 10);

	wrq.u.data.pointer = (caddr_t)&tmp;
	wrq.u.data.length = tmp.length + 16;//12 is the sizeof sfcfghdr except data field, 10 is the actual data

	//call SFCFG_CMD_ATE_TX_STOP
	if(do_ioctl(SFCFG_PRIV_IOCTL_ATE, &wrq) < 0){
		printf("SFCFG_PRIV_IOCTL_ATE ioctl SFCFG_CMD_ATE_MACBYPASS_TX_STOP failed\n");
		return -1;
	}
	return 0;
}

int32_t send_rx_stop()
{
	struct iwreq wrq;
	struct sfcfghdr tmp;

	g_clock_flag = false;
	memset(&wrq, 0, sizeof(struct iwreq));
	memset(&tmp, 0, sizeof(struct sfcfghdr));

	tmp.magic_no    = SFCFG_MAGIC_NO;
	tmp.comand_type = SFCFG_CMD_ATE_RX_STOP;
	tmp.comand_id   = SFCFG_CMD_ATE_RX_STOP;
	tmp.length      = 10;
	tmp.sequence    = 1;
	strncpy(tmp.data, "false", 10);

	wrq.u.data.pointer = (caddr_t)&tmp;
	wrq.u.data.length = tmp.length + 16;//12 is the sizeof sfcfghdr except data field, 10 is the actual data

	//call SFCFG_CMD_ATE_RX_STOP
	if(do_ioctl(SFCFG_PRIV_IOCTL_ATE, &wrq) < 0){
		printf("SFCFG_PRIV_IOCTL_ATE ioctl SFCFG_CMD_ATE_RX_FRAME_START failed\n");
		return -1;
	}
	return 0;
}

int32_t send_rx_stop_by_macbypass()
{
	struct iwreq wrq;
	struct sfcfghdr tmp;
	g_clock_flag = false;
	memset(&wrq, 0, sizeof(struct iwreq));
	memset(&tmp, 0, sizeof(struct sfcfghdr));

	tmp.magic_no    = SFCFG_MAGIC_NO;
	tmp.comand_type = SFCFG_CMD_ATE_MACBYPASS_RX_STOP;
	tmp.comand_id   = SFCFG_CMD_ATE_MACBYPASS_RX_STOP;
	tmp.length      = 20;
	tmp.sequence    = 1;
	strncpy(tmp.data, "rx stop macbypass", tmp.length);

	wrq.u.data.pointer = (caddr_t)&tmp;
	wrq.u.data.length = tmp.length + 16;//12 is the sizeof sfcfghdr except data field, 10 is the actual data

	//call SFCFG_CMD_ATE_TX_STOP
	if(do_ioctl(SFCFG_PRIV_IOCTL_ATE, &wrq) < 0){
		printf("SFCFG_PRIV_IOCTL_ATE ioctl SFCFG_CMD_ATE_MACBYPASS_RX_STOP failed\n");
		return -1;
	}
	return 0;
}

int32_t save_temp_to_flash(char *tmpdata)
{
	struct iwreq wrq;
	struct sfcfghdr tmp;
	int offset = 174;
	int length = 2;
	memset(&wrq, 0, sizeof(struct iwreq));
	memset(&tmp, 0, sizeof(struct sfcfghdr));
	// cmd ate band
	tmp.magic_no = SFCFG_MAGIC_NO;
	tmp.comand_type = SFCFG_CMD_ATE_SAVE_DATA_TO_FLASH;
	tmp.comand_id   = SFCFG_CMD_ATE_SAVE_DATA_TO_FLASH;
	tmp.length      = 2;
	tmp.sequence    = 1;

	sprintf(tmp.data,"%d",offset);
	memcpy(&tmp.data[4],tmpdata,length);
	printf("tmpdata= %d---------\n", tmpdata[0]);

	wrq.u.data.pointer = (caddr_t)&tmp;
	wrq.u.data.length = sizeof(struct sfcfghdr) - sizeof(((struct sfcfghdr *)0)->data) + 10;

#ifdef NAND_SUPPORT
	int ret;

	ret = flash_init();
	if (ret)
		return ret;

	flash_write(tmpdata, offset, length);
	flash_deinit();
#else
	if (do_ioctl(SFCFG_PRIV_IOCTL_ATE, &wrq) < 0) {
		printf("SFCFG_PRIV_IOCTL_ATE ioctl SFCFG_CMD_ATE_SAVE_DATA_TO_FLASH failed\n");
		return -1;
	}
#endif
	return 0;
}

int32_t save_refpower_to_flash(char *tmpdata, int offset)
{
	struct iwreq wrq;
	struct sfcfghdr tmp;
	int length = 2;
	memset(&wrq, 0, sizeof(struct iwreq));
	memset(&tmp, 0, sizeof(struct sfcfghdr));
	// cmd ate band
	tmp.magic_no = SFCFG_MAGIC_NO;
	tmp.comand_type = SFCFG_CMD_ATE_SAVE_DATA_TO_FLASH;
	tmp.comand_id   = SFCFG_CMD_ATE_SAVE_DATA_TO_FLASH;
	tmp.length      = 2;
	tmp.sequence    = 1;

	sprintf(tmp.data,"%d",offset);
	memcpy(&tmp.data[4],tmpdata,length);
	printf("tmpdata= %d---------%d------\n", tmpdata[0], tmpdata[1]);

	wrq.u.data.pointer = (caddr_t)&tmp;
	wrq.u.data.length = sizeof(struct sfcfghdr) - sizeof(((struct sfcfghdr *)0)->data) + 10;

#ifdef NAND_SUPPORT
	int ret;

	ret = flash_init();
	if (ret)
		return ret;

	flash_write(tmpdata, offset, length);
	flash_deinit();
#else
	if (do_ioctl(SFCFG_PRIV_IOCTL_ATE, &wrq) < 0) {
		printf("SFCFG_PRIV_IOCTL_ATE ioctl SFCFG_CMD_ATE_SAVE_DATA_TO_FLASH failed\n");
		return -1;
	}
#endif
	return 0;
}

int32_t save_mac_to_factory(char *tmpdata)
{
	struct iwreq wrq;
	struct sfcfghdr tmp;
	int offset = 0;
	int length = 12;
	memset(&wrq, 0, sizeof(struct iwreq));
	memset(&tmp, 0, sizeof(struct sfcfghdr));

	tmp.magic_no = SFCFG_MAGIC_NO;
	tmp.comand_type = SFCFG_CMD_ATE_SAVE_DATA_TO_FLASH;
	tmp.comand_id   = SFCFG_CMD_ATE_SAVE_DATA_TO_FLASH;
	tmp.length      = length;
	tmp.sequence    = 1;

	sprintf(tmp.data, "%d", offset);
	memcpy(&tmp.data[4], tmpdata, length);
	printf("--- tmpdata = %s ---\n", &tmp.data[4]);

	wrq.u.data.pointer = (caddr_t)&tmp;
	wrq.u.data.length = sizeof(struct sfcfghdr) - sizeof(((struct sfcfghdr *)0)->data) + 16;

#ifdef NAND_SUPPORT
	int ret;

	ret = flash_init();
	if (ret)
		return ret;

	flash_write(tmpdata, offset, length);
	flash_deinit();
#else
	if (do_ioctl(SFCFG_PRIV_IOCTL_ATE, &wrq) < 0) {
		printf("SFCFG_PRIV_IOCTL_ATE ioctl SFCFG_CMD_ATE_SAVE_DATA_TO_FLASH failed\n");
		return -1;
	}
#endif
	return 0;
}

int32_t save_XO_config_to_flash(char *tmpdata)
{
	struct iwreq wrq;
	struct sfcfghdr tmp;
	memset(&wrq, 0, sizeof(struct iwreq));
	memset(&tmp, 0, sizeof(struct sfcfghdr));
	// cmd ate band
	tmp.magic_no = SFCFG_MAGIC_NO;
	tmp.comand_type = SFCFG_CMD_ATE_SAVE_TO_FLASH;
	tmp.comand_id   = SFCFG_CMD_ATE_SAVE_TO_FLASH;
	tmp.length      = 20;
	tmp.sequence    = 1;

	memcpy(tmp.data,tmpdata, tmp.length);
	printf("tmp.data= %s---------\n", tmp.data);

	wrq.u.data.pointer = (caddr_t)&tmp;
	wrq.u.data.length = sizeof(struct sfcfghdr) - sizeof(((struct sfcfghdr *)0)->data) + tmp.length;

#ifdef NAND_SUPPORT
	int ret;
	unsigned char num;

	ret = flash_init();
	if (ret)
		return ret;

	num = 0xff & strtoul(tmpdata, NULL, 16);
	tmpdata[0] = num;
	tmpdata[1] = tmpdata[0];
	tmpdata[2] = '\0';

	flash_write(tmpdata, 2048 + 2, 2);
	flash_deinit();
#else
	if(do_ioctl(SFCFG_PRIV_IOCTL_ATE, &wrq) < 0){
		printf("SFCFG_PRIV_IOCTL_ATE ioctl SFCFG_CMD_ATE_SAVE_TO_FLASH failed\n");
		return -1;
	}
#endif
	return 0;
}

int parse_rssi_offset(const char *input, char *output, int length)
{
    int i, values[4];

    if (sscanf(input, "%d,%d,%d,%d",
               &values[0], &values[1], &values[2], &values[3]) != 4) {
        return 0;
    }

    for (i = 0; i < 4; i++) {
        if (values[i] < -127 || values[i] > 127) {
            return 0;
        }
    }

    for (i = 0; i < 4; i++) {
        output[i] = (char)(values[i] & 0xFF);
    }

    return 1;
}

int parse_mac_address(const char *input, char *output)
{
	int i;
	unsigned int bytes[6];

	if (sscanf(input, "%02x:%02x:%02x:%02x:%02x:%02x",
			   &bytes[0], &bytes[1], &bytes[2],
			   &bytes[3], &bytes[4], &bytes[5]) != 6) {
		return 0;
	}

	for (i = 0; i < 6; i++) {
		output[i] = (char)bytes[i];
	}

	return 1;
}

int parse_hex_string(const char *input, char *output, int length)
{
	int i;
	unsigned int byte;

	if (strncmp(input, "0x", 2) != 0) {
		return 0;
	}
	input += 2;

	if (strlen(input) != length * 2) {
		return 0;
	}

	for (i = 0; i < length; i++) {
		if (sscanf(input + (i * 2), "%02x", &byte) != 1) {
			return 0;
		}
		output[i] = (char)byte;
	}

	return 1;
}

int parse_int_string(const char *input, char *output, int length)
{
	int i;
	unsigned int num;

	if (sscanf(input, "%u", &num) != 1) {
		return 0;
	}

	for (i = 0; i < length; i++) {
		output[i] = (char)(num >> (8 * i) & 0xff);
	}

	return 1;
}

int32_t save_factory_info_to_factory(char *tmpdata, unsigned int offset, unsigned int length)
{
	struct iwreq wrq;
	struct sfcfghdr tmp;

	memset(&wrq, 0, sizeof(struct iwreq));
	memset(&tmp, 0, sizeof(struct sfcfghdr));
	// cmd ate band
	tmp.magic_no    = SFCFG_MAGIC_NO;
	tmp.comand_type = SFCFG_CMD_ATE_SAVE_DATA_TO_FLASH;
	tmp.comand_id   = SFCFG_CMD_ATE_SAVE_DATA_TO_FLASH;
	tmp.length      = length;
	tmp.sequence    = 1;

#ifdef NAND_SUPPORT
	int ret;

	ret = flash_init();
	if (ret)
		return ret;

	flash_write(tmpdata, offset, length);
	flash_deinit();
#else
	sprintf(tmp.data, "%d", offset);
	memcpy(&tmp.data[4], tmpdata, length);

	wrq.u.data.pointer = (caddr_t)&tmp;
	wrq.u.data.length = sizeof(struct sfcfghdr) - sizeof(((struct sfcfghdr *)0)->data) + tmp.length + 4;

	if (do_ioctl(SFCFG_PRIV_IOCTL_ATE, &wrq) < 0) {
		printf("SFCFG_PRIV_IOCTL_ATE ioctl SFCFG_CMD_ATE_SAVE_DATA_TO_FLASH failed\n");
		return -1;
	}
#endif
	return 0;
}

int cali_rssi_offset(char *band, char *tmpdata)
{
	struct iwreq wrq;
	struct sfcfghdr tmp;
	int rssi_offset1, rssi_offset2;
	int rssi_result1, rssi_result2;
	int rssi_power = 0;
	int offset = 0;
	int flag = 0;
	int i = 0;
	int phy_num[2];
	char buffer[64] = "";
	char *dir_path = "/sys/kernel/debug/ieee80211";
	char file_path[64] = "";
	DIR *dir = NULL;
	struct dirent *dirent = NULL;
	FILE *fp = NULL;
	int rssi1total = 0,rssi2total = 0;
	int valid_value1 = 0,valid_value2 = 0;

	memset(&wrq, 0, sizeof(struct iwreq));
	memset(&tmp, 0, sizeof(struct sfcfghdr));

	if (strcmp(band, "hb") == 0) {
		flag = 0;
		offset = 182;
	} else if (strcmp(band, "lb") == 0) {
		flag = 1;
		offset = 184;
	} else {
		printf("band error! it must be 'hb' or 'lb'!\n");
		return -1;
	}

	dir = opendir(dir_path);
	while ((dirent = readdir(dir)) != NULL) {
		if (strncasecmp(dirent->d_name, "phy", 3) == 0) {
			sscanf(dirent->d_name, "phy%d", &phy_num[i]);
			i++;
		} else {
			continue;
		}
	}
	closedir(dir);

	if (i == 0) {
		printf("there is no phy in /sys/kernel/debug/ieee80211/ directory!\n");
		return -1;
	} else if (i == 1) {
		sprintf(file_path, "%s/phy%d/siwifi/rssi_inbandpower_20p", dir_path, phy_num[0]);
	} else if (i == 2) {
		if (flag == 0) {
			sprintf(file_path, "%s/phy%d/siwifi/rssi_inbandpower_20p", dir_path,
				 (phy_num[0]<phy_num[1] ? phy_num[0] : phy_num[1]));
		} else if (flag == 1) {
			sprintf(file_path, "%s/phy%d/siwifi/rssi_inbandpower_20p", dir_path,
				 (phy_num[0]<phy_num[1] ? phy_num[1] : phy_num[0]));
		}
	}

	rssi_power = atoi(tmpdata);
	for(int i=0;i<15;i++)
	{
		fp = fopen(file_path, "r");
		fgets(buffer, sizeof(buffer), fp);
		printf("%s",buffer);
		fclose(fp);
		sscanf(buffer, "rssi1_inbandpower20p(dBm)=%d rssi2_inbandpower20p(dBm)=%d", &rssi_result1, &rssi_result2);

		if(abs(rssi_power - rssi_result1)<=15)
		{
			rssi1total +=rssi_result1;
			valid_value1++;
		}
		if(abs(rssi_power - rssi_result2)<=15)
		{
			rssi2total +=rssi_result2;
			valid_value2++;
		}
		usleep(10000);
	}
	if(valid_value1 && valid_value2)
	{
		rssi_result1 = rssi1total / valid_value1;
		rssi_result2 = rssi2total / valid_value2;
	}
	else
	{
		printf("!!!!!!!!!!Rx-Tx power diff. > 10\n");
		return -1;
	}
	rssi_offset1 = rssi_power - rssi_result1;
	rssi_offset2 = rssi_power - rssi_result2;

	rssi_offset1 = rssi_offset1 & 0xff;
	rssi_offset2 = rssi_offset2 & 0xff;

	printf("input power is %d. get result is %d %d. rssi offset is 0x%02x 0x%02x.\n", rssi_power, rssi_result1, rssi_result2, rssi_offset1, rssi_offset2);

	tmp.magic_no 	= SFCFG_MAGIC_NO;
	tmp.comand_type = SFCFG_CMD_ATE_SAVE_DATA_TO_FLASH;
	tmp.comand_id   = SFCFG_CMD_ATE_SAVE_DATA_TO_FLASH;
	tmp.length      = 2;
	tmp.sequence    = 1;

	sprintf(tmp.data, "%d", offset);
	tmp.data[4] = (char)rssi_offset1;
	tmp.data[5] = (char)rssi_offset2;
	tmp.data[6] = '\0';

	wrq.u.data.pointer = (caddr_t)&tmp;
	wrq.u.data.length = sizeof(struct sfcfghdr) - sizeof(((struct sfcfghdr *)0)->data) + tmp.length + 4;

#ifdef NAND_SUPPORT
	int ret;

	ret = flash_init();
	if (ret)
		return ret;

	flash_write(&tmp.data[4], offset, tmp.length);
	flash_deinit();
#else
	if (do_ioctl(SFCFG_PRIV_IOCTL_ATE, &wrq) < 0) {
		printf("SFCFG_PRIV_IOCTL_ATE ioctl SFCFG_CMD_ATE_SAVE_DATA_TO_FLASH failed\n");
		return -1;
	}
#endif

	return 0;
}

//get power from factory and return the power as int param.
/*int32_t get_factory_power(char *channel,char *bw,char *mode,char *mcs){
	struct iwreq wrq;
	struct sfcfghdr tmp;
	int power = 0;
	memset(&wrq, 0, sizeof(struct iwreq));
	memset(&tmp, 0, sizeof(struct sfcfghdr));
	//cmd get factory power;
	tmp.magic_no = SFCFG_MAGIC_NO;
	tmp.comand_type = SFCFG_CMD_ATE_GET_POWER;
	tmp.comand_id   = SFCFG_CMD_ATE_GET_POWER;
	tmp.length      = 10;
	tmp.sequence    = 1;

	sprintf(tmp.data,"%s %s %s %s",channel,bw,mode,mcs);
	printf("tmp.data==== %s--------------\n",tmp.data);
	wrq.u.data.pointer = (caddr_t)&tmp;
	wrq.u.data.length = sizeof(struct sfcfghdr) - sizeof(((struct sfcfghdr *)0)->data) + tmp.length;

	if(do_ioctl(SFCFG_PRIV_IOCTL_ATE, &wrq) < 0){
	  printf("SFCFG_PRIV_IOCTL_ATE ioctl SFCFG_CMD_ATE_GET_POWER failed\n");
		return -1;
	 }
	 memcpy(&power, wrq.u.data.pointer, 4);

	 return power;
}*/

int save_rssi_offset(char *rssi)
{
	struct iwreq wrq;
	struct sfcfghdr tmp;

	memset(&wrq, 0, sizeof(struct iwreq));
	memset(&tmp, 0, sizeof(struct sfcfghdr));

#ifdef NAND_SUPPORT
	int ret;

	ret = flash_init();
	if (ret)
		return ret;

	flash_write(rssi, 182, 4);
	flash_deinit();
#else
	tmp.magic_no    = SFCFG_MAGIC_NO;
	tmp.comand_type = SFCFG_CMD_ATE_SAVE_DATA_TO_FLASH;
	tmp.comand_id   = SFCFG_CMD_ATE_SAVE_DATA_TO_FLASH;
	tmp.length      = 2;
	tmp.sequence    = 1;

	sprintf(tmp.data, "%d", 182);
	tmp.data[4] = (char)rssi[0];
	tmp.data[5] = (char)rssi[1];
	printf("offset= %s-----hb rssi offset hb1=%02x hb2=%02x-----\n", tmp.data, (char)tmp.data[4], (char)tmp.data[5]);

	wrq.u.data.pointer = (caddr_t)&tmp;
	wrq.u.data.length = sizeof(struct sfcfghdr) - sizeof(((struct sfcfghdr *)0)->data) + tmp.length + 4;

	if (do_ioctl(SFCFG_PRIV_IOCTL_ATE, &wrq) < 0) {
		printf("SFCFG_PRIV_IOCTL_ATE ioctl SFCFG_CMD_ATE_SAVE_DATA_TO_FLASH failed\n");
		return -1;
	}

	sprintf(tmp.data, "%d", 184);
	tmp.data[4] = (char)rssi[2];
	tmp.data[5] = (char)rssi[3];
	printf("offset= %s-----lb rssi offset lb1=%02x lb2=%02x-----\n", tmp.data, (char)tmp.data[4], (char)tmp.data[5]);

	wrq.u.data.pointer = (caddr_t)&tmp;
	wrq.u.data.length = sizeof(struct sfcfghdr) - sizeof(((struct sfcfghdr *)0)->data) + tmp.length + 4;

	if (do_ioctl(SFCFG_PRIV_IOCTL_ATE, &wrq) < 0) {
		printf("SFCFG_PRIV_IOCTL_ATE ioctl SFCFG_CMD_ATE_SAVE_DATA_TO_FLASH failed\n");
		return -1;
	}

#endif

	return 0;
}

int32_t set_power_config(void)
{

	FILE *fp1;
	char text[1024];

	printf("start load wifi tx power table config...\n");
	//change tx_power.txt to tx_power.bin
	system("/usr/bin/txpower_calibrate_table.sh");
	//TODO:check bin does it exist
	//reload wifi driver
	system("sfwifi reset fmac");
	sleep(3);
	//printf using wifi tx power table
	printf("the using tx power table is: \n");
	fp1=fopen("/usr/bin/txpower_calibrate_table.txt","r");
	while(fscanf(fp1,"%[^\n]",text)!=EOF)
	{
		getc(fp1);
		puts(text);
		//if need put map to another txt
		//fputs(text,fp2);
	}
	fclose(fp1);
	return 0;
}

int32_t save_power_config_to_flash(uint16_t tmpdata, uint16_t channel, uint16_t bw, uint16_t mode, uint16_t rate, uint16_t ant)
{
	struct iwreq wrq;
	struct sfcfghdr tmp;
	uint32_t offset = 0;

	memset(&wrq, 0, sizeof(struct iwreq));
	memset(&tmp, 0, sizeof(struct sfcfghdr));

	// cmd ate band
	tmp.magic_no = SFCFG_MAGIC_NO;
	tmp.comand_type = SFCFG_CMD_ATE_SAVE_TO_FLASH;
	tmp.comand_id   = SFCFG_CMD_ATE_SAVE_TO_FLASH;
	tmp.length      = 20;
	tmp.sequence    = 1;

/*
	if (channel < 2485){
		switch(channel){
			case 2412:offset = (((channel - 2412) / 5)*28);break;
			case 2417:offset = (((channel - 2412) / 5)*28);break;
			case 2422:offset = (((channel - 2412) / 5)*28);break;
			case 2427:offset = (((channel - 2412) / 5)*28);break;
			case 2432:offset = (((channel - 2412) / 5)*28);break;
			case 2437:offset = (((channel - 2412) / 5)*28);break;
			case 2442:offset = (((channel - 2412) / 5)*28);break;
			case 2447:offset = (((channel - 2412) / 5)*28);break;
			case 2452:offset = (((channel - 2412) / 5)*28);break;
			case 2457:offset = (((channel - 2412) / 5)*28);break;
			case 2462:offset = (((channel - 2412) / 5)*28);break;
			case 2467:offset = (((channel - 2412) / 5)*28);break;
			case 2472:offset = (((channel - 2412) / 5)*28);break;
			default:
				  return -1;
			}
			if (mode == 0){
				offset += rate;
			}
			else if (mode == 2){
				switch(bw)
				{
					case 1: offset += 12 + rate;break;
					case 2: offset += 20 + rate;break;
					default: return -1;
				}
			}
			if (mode !=0 && mode != 2)
			      return -1;
	}
	else{
		switch(channel){
			case 5180:offset = 364 + 53*0;break;
			case 5200:offset = 364 + 53*1;break;
			case 5220:offset = 364 + 53*2;break;
			case 5240:offset = 364 + 53*3;break;
			case 5260:offset = 364 + 53*4;break;
			case 5280:offset = 364 + 53*5;break;
			case 5300:offset = 364 + 53*6;break;
			case 5320:offset = 364 + 53*7;break;
			case 5500:offset = 364 + 53*8;break;
			case 5520:offset = 364 + 53*9;break;
			case 5540:offset = 364 + 53*10;break;
			case 5560:offset = 364 + 53*11;break;
			case 5580:offset = 364 + 53*12;break;
			case 5600:offset = 364 + 53*13;break;
			case 5620:offset = 364 + 53*14;break;
			case 5640:offset = 364 + 53*15;break;
			case 5660:offset = 364 + 53*16;break;
			case 5680:offset = 364 + 53*17;break;
			case 5700:offset = 364 + 53*18;break;
			case 5720:offset = 364 + 53*19;break;
			case 5745:offset = 364 + 53*20;break;
			case 5765:offset = 364 + 53*21;break;
			case 5785:offset = 364 + 53*22;break;
			case 5805:offset = 364 + 53*23;break;
			case 5825:offset = 364 + 53*24;break;
			default: return -1;
			}
		if (mode == 0)
		      offset += rate;
		if (mode == 2){
			switch(bw){
				case 1:offset += 8  + rate;break;
				case 2:offset += 16 + rate;break;
				default: return -1;
			}
		}
		if (mode ==4){
			switch(bw){
				case 1:offset += 24 + rate;break;
				case 2:offset += 33 + rate;break;
				case 3:offset += 43 + rate;break;
				default: return -1;
			}
		if (!(mode ==0||mode ==2||mode ==4)){
				return -1;
			}
		}
	}
*/
	offset = get_power_offset(channel, mode, bw, rate);
	printf("get power offset:%d\n",offset);
	if (ant == 1) {
		sprintf(tmp.data,"1%03hu0%4d",tmpdata,offset + 2048 + 4);	//factory form 2048 begin and 4 bytes form 2048 to 2052  save xo;
	}
	else if (ant == 2) {
		sprintf(tmp.data,"1%03hu0%4d",tmpdata,offset + 2048 + 4 + 3201);
	}
	printf("tmp.data= %s---------\n", tmp.data);

	wrq.u.data.pointer = (caddr_t)&tmp;
	wrq.u.data.length = sizeof(struct sfcfghdr) - sizeof(((struct sfcfghdr *)0)->data) + tmp.length;

#ifdef NAND_SUPPORT
	int ret;

	ret = flash_init();
	if (ret)
		return ret;

	if (ant == 1) {
		flash_write(tmp.data, 2048 + 4, tmp.length);
	} else if (ant == 2) {
		flash_write(tmp.data, 2048 + 4 + 3201, tmp.length);
	}
	flash_deinit();
#else
	if(do_ioctl(SFCFG_PRIV_IOCTL_ATE, &wrq) < 0){
		printf("SFCFG_PRIV_IOCTL_ATE ioctl SFCFG_CMD_ATE_SAVE_TO_FLASH failed\n");
		return -1;
	}
#endif
	return 0;
}

uint32_t _offset_result(int ant)
{
	FILE *fp,*fp2,*fp3;
	int i,j,k;
        /*2.4G calibration value line*/
	int g_24_channel[CALI_TABLE_2G_LINE] = {0};
        /*5G calibration value line*
         *11ac(line 25) + 11ax(line 25)*/
	int g_5_channel[CALI_TABLE_5G_LINE] = {0};
#if AX80CALI
        /*5G calibration value line 2*/
	int g_5_channel_t[CALI_TABLE_5G_LINE] = {0};
#endif
        /*one 24g channel calibration value*/
	int g_one_24g_channel_power[CALI_TABLE_24G_CH_SIZE] = {0};
        /*one 5g channel calibratioen value(11ax/11ac/n/a)*/
	int g_one_5g_channel_power[CALI_TABLE_5G_CH_SIZE] = {0};
        /*one calitable size*/
	int save_power[CALI_TABLE_ONE_ANT] = {0};
        /*gain offset size*/
	int gain_offset[CALI_TABLE_ONE_ANT] = {0};
	/*TODO:open config for reset*/
	if(ant == ANT2)
	{
		fp = fopen("/etc/atetools/power_save_ant2.txt","r");
	}else
	{
		fp = fopen("/etc/atetools/power_save_ant1.txt","r");
	}

	for(i = 0; i < CALI_TABLE_ONE_ANT; i++)
	{
		fscanf(fp,"%d",&save_power[i]);
	}
	/*how to offset,fill up the array */
	//TODO
	/*1,offset set one 24g channel*/
	/*2,copy to other channel*/
	/*2.4g*/
	for(i = 0; i < 13; i++)
	{
		if(i <= 4)
		{
                        if(i == 0)
                        {
		                g_24_channel[i] = save_power[51+i*52];
                                if(g_24_channel[i] == 0)
                                        printf("[ate_cmd]:Warning!!!! Maybe ch1 not calibrate? or the value is 0? please confirm!\r\n");
                                /*now use 11ax 40M mcs11 value as base calibration point*/
                                /*TODO: Maybe ,need change base point by final hardware test data*/
                                for(j=0;j<52;j++)
                                {
			                g_one_24g_channel_power[j] = save_power[51+i*52];
			        }
                        }
                        /*copy ch1 to ch2 ch3 ch4 ch5*/
			memcpy((void *)&save_power[i*52],(void *)&g_one_24g_channel_power[0],52*sizeof(int));
		}
		else if( i > 4 && i < 9)
		{
			if(i == 6)
                        {
				memset(g_one_24g_channel_power,0,52*sizeof(int));
			        for(j=0;j<52;j++)
			        {
					g_one_24g_channel_power[j] = save_power[51+i*52];
			        }
                                for(k=5;k<9;k++)
                                {
                                        /*copy ch7 to ch6 ch8 ch9*/
			                memcpy((void *)&save_power[k*52],(void *)&g_one_24g_channel_power[0],52*sizeof(int));
                                }
                        }
		}
		else
		{
			if(i == 12)
                        {
				memset(g_one_24g_channel_power,0,52*sizeof(int));
			        for(j=0;j<52;j++)
			        {
                                        g_one_24g_channel_power[j] = save_power[51+i*52];
			        }
			        for(k=9;k<=12;k++)
                                {
			                memcpy((void *)&save_power[k*52],(void *)&g_one_24g_channel_power[0],52*sizeof(int));
                                }
                        }
		}
	}
	/*5g 11a_n_ac_ax*/
	for(i = 0; i < 25; i++)
	{
		if(i < 4 )//ch36-ch48 use ch36
		{
			if(i == 0)//use calibrate value to fill ch36 line
			{
                                g_5_channel[i] = save_power[CALI_TABLE_2G_SIZE+100+i*101];
#if AX80CALI
                                g_5_channel[i] = save_power[CALI_TABLE_2G_SIZE+88+i*101];
#endif
                                if(g_5_channel[i] == 0)
                                        printf("[ate_cmd]:Warning!!!! Maybe ch36 not calibrate? or the value is 0? please confirm!\r\n");
                                /*now use 11ax 160M mcs11 value as base calibration point*/
                                /*TODO: Maybe ,need change base point by final hardware test data*/

				for(j=0;j<101;j++)
				{
#if AX80CALI
					if(j < 16)//11a 11n_20
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+64+i*101];
					else if(j >= 16 && j < 24)//11n_40
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+88+i*101];
					else if(j >= 24 && j < 33)//11ac_20
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+64+i*101];
					else if(j >= 33 && j < 43)//11ac_40
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+88+i*101];
					else if(j >= 43 && j < 53)//11ac_80
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+88+i*101];
					else if(j >= 53 && j < 65)//11ax_20
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+64+i*101];
					else if(j >= 65 && j < 77)//11ax_40
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+88+i*101];
					else if(j >= 77 && j < 89)//11ax_80
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+88+i*101];
					else//11ax_160
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+100+i*101];
#else
					if(j < 16)//11a 11n_20
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+100+i*101];
					else if(j >= 16 && j < 24)//11n_40
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+100+i*101];
					else if(j >= 24 && j < 33)//11ac_20
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+100+i*101];
					else if(j >= 33 && j < 43)//11ac_40
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+100+i*101];
					else if(j >= 43 && j < 53)//11ac_80
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+100+i*101];
					else if(j >= 53 && j < 65)//11ax_20
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+100+i*101];
					else if(j >= 65 && j < 77)//11ax_40
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+100+i*101];
					else if(j >= 77 && j < 89)//11ax_80
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+100+i*101];
					else//11ax_160
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+100+i*101];
#endif
				}
			}
#if AX80CALI //only 5180 need calibrate 11ax 20M mcs11，so we need to keep 5200\5220\5240 use 11ax 80M
            else {
				for(j=0;j<101;j++)
				{
					if(j < 16)//11a 11n_20
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+88];
					else if(j >= 16 && j < 24)//11n_40
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+88];
					else if(j >= 24 && j < 33)//11ac_20
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+88];
					else if(j >= 33 && j < 43)//11ac_40
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+88];
					else if(j >= 43 && j < 53)//11ac_80
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+88];
					else if(j >= 53 && j < 65)//11ax_20
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+88];
					else if(j >= 65 && j < 77)//11ax_40
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+88];
					else if(j >= 77 && j < 89)//11ax_80
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+88];
					else//11ax_160
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+100];
				}
		    }
		    memcpy((void *)&save_power[CALI_TABLE_2G_SIZE+i*101],(void *)&g_one_5g_channel_power[0],101*sizeof(int));
#endif
		}
		else if(i >= 4 && i < 8)//ch52-ch64  use ch64
		{
			if(i == 7)//use calibrate value to fill ch64 line
			{
				memset(g_one_5g_channel_power,0,53*sizeof(int));
				for(j=0;j<101;j++)
				{
#if AX80CALI
					if(j < 16)//11a 11n_20
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+88+i*101];
					else if(j >= 16 && j < 24)//11n_40
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+88+i*101];
					else if(j >= 24 && j < 33)//11ac_20
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+88+i*101];
					else if(j >= 33 && j < 43)//11ac_40
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+88+i*101];
					else if(j >= 43 && j < 53)//11ac_80
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+88+i*101];
					else if(j >= 53 && j < 65)//11ax_20
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+88+i*101];
					else if(j >= 65 && j < 77)//11ax_40
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+88+i*101];
					else if(j >= 77 && j < 89)//11ax_80
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+88+i*101];
					else                      //11ax_160
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+100+i*101];
#else
					if(j < 16)//11a 11n_20
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+100+i*101];
					else if(j >= 16 && j < 24)//11n_40
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+100+i*101];
					else if(j >= 24 && j < 33)//11ac_20
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+100+i*101];
					else if(j >= 33 && j < 43)//11ac_40
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+100+i*101];
					else if(j >= 43 && j < 53)//11ac_80
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+100+i*101];
					else if(j >= 53 && j < 65)//11ax_20
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+100+i*101];
					else if(j >= 65 && j < 77)//11ax_40
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+100+i*101];
					else if(j >= 77 && j < 89)//11ax_80
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+100+i*101];
					else                      //11ax_160
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+100+i*101];
#endif
				}
			}
		}
#if AX80CALI
		else if( i >= 8 && i < 12 )//if need calibrate 5580, 5500 just copy for ch100-ch112
#else
		else if( i >= 8 && i < 16 )//for 5500,if calibrate use 5500 gain,if not copy 5320 for ch100-ch128
#endif
		{
			if(i == 8)
			{
				memset(g_one_5g_channel_power,0,101*sizeof(int));
				for(j=0;j<101;j++)
				{
#if AX80CALI
					if(j < 16)//11a 11n_20
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+88+i*101];
					else if(j >= 16 && j < 24)//11n_40
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+88+i*101];
					else if(j >= 24 && j < 33)//11ac_20
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+88+i*101];
					else if(j >= 33 && j < 43)//11ac_40
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+88+i*101];
					else if(j >= 43 && j < 53)//11ac_80
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+88+i*101];
					else if(j >= 53 && j < 65)//11ax_20
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+88+i*101];
					else if(j >= 65 && j < 77)//11ax_40
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+88+i*101];
					else if(j >= 77 && j < 89)//11ax_80
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+88+i*101];
					else                      //11ax_160
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+100+i*101];
#else
					if(j < 16)//11a 11n_20
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+100+i*101];
					else if(j >= 16 && j < 24)//11n_40
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+100+i*101];
					else if(j >= 24 && j < 33)//11ac_20
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+100+i*101];
					else if(j >= 33 && j < 43)//11ac_40
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+100+i*101];
					else if(j >= 43 && j < 53)//11ac_80
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+100+i*101];
					else if(j >= 53 && j < 65)//11ax_20
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+100+i*101];
					else if(j >= 65 && j < 77)//11ax_40
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+100+i*101];
					else if(j >= 77 && j < 89)//11ax_80
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+100+i*101];
					else                      //11ax_160
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+100+i*101];
#endif
				}
			}
		}
#if AX80CALI
		else if( i >= 12 && i < 16 )//for 5500,if calibrate use 5500 gain,if not copy 5320 for ch100-ch128
		{
			if(i == 12)
			{
				memset(g_one_5g_channel_power,0,101*sizeof(int));
				for(j=0;j<101;j++)
				{
					if(j < 16)//11a 11n_20
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+88+i*101];
					else if(j >= 16 && j < 24)//11n_40
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+88+i*101];
					else if(j >= 24 && j < 33)//11ac_20
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+88+i*101];
					else if(j >= 33 && j < 43)//11ac_40
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+88+i*101];
					else if(j >= 43 && j < 53)//11ac_80
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+88+i*101];
					else if(j >= 53 && j < 65)//11ax_20
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+88+i*101];
					else if(j >= 65 && j < 77)//11ax_40
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+88+i*101];
					else if(j >= 77 && j < 89)//11ax_80
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+88+i*101];
					else                      //11ax_160 use 5500
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+100+8*101];
				}
			}
		}
#endif
		else if( i >= 16 && i < 20 )//for 5660,if calibrate use 5660 gain,if not copy 5660 for ch132-ch144,use 11ax 80M mcs11
		{
			if(i == 16)
			{
				memset(g_one_5g_channel_power,0,101*sizeof(int));
				for(j=0;j<89;j++)
				{
					if(j < 16)//11a 11n_20
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+88+i*101];
					else if(j >= 16 && j < 24)//11n_40
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+88+i*101];
					else if(j >= 24 && j < 33)//11ac_20
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+88+i*101];
					else if(j >= 33 && j < 43)//11ac_40
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+88+i*101];
					else if(j >= 43 && j < 53)//11ac_80
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+88+i*101];
					else if(j >= 53 && j < 65)//11ax_20
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+88+i*101];
					else if(j >= 65 && j < 77)//11ax_40
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+88+i*101];
					else//11ax_80
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+88+i*101];
				}
			}
		}
		else //ch149-ch165 use ch149
		{
			if(i == 20)//use calibrate value to fill ch149 line
			{
				memset(g_one_5g_channel_power,0,101*sizeof(int));
				for(j=0;j<89;j++)
				{
					if(j < 16)//11a 11n_20
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+88+i*101];
					else if(j >= 16 && j < 24)//11n_40
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+88+i*101];
					else if(j >= 24 && j < 33)//11ac_20
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+88+i*101];
					else if(j >= 33 && j < 43)//11ac_40
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+88+i*101];
					else if(j >= 43 && j < 53)//11ac_80
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+88+i*101];
					else if(j >= 53 && j < 65)//11ax_20
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+88+i*101];
					else if(j >= 65 && j < 77)//11ax_40
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+88+i*101];
					else//11ax_80
						g_one_5g_channel_power[j] = save_power[CALI_TABLE_2G_SIZE+88+i*101];
				}
			}
		}
		if(i == 7)//copy ch64 calibration value for ch52 ch56 ch60
		{
			for(k=4;k<7;k++)
			{
				memcpy((void *)&save_power[CALI_TABLE_2G_SIZE+k*101],(void *)&g_one_5g_channel_power[0],101*sizeof(int));
			}
		}
		memcpy((void *)&save_power[CALI_TABLE_2G_SIZE+i*101],(void *)&g_one_5g_channel_power[0],101*sizeof(int));
	}
	fclose(fp);
/*calculate power_save.txt by gain_offset*/
	fp3 = fopen("/etc/atetools/gain_offset.txt","r");
	for(i = 0; i < CALI_TABLE_ONE_ANT; i++)
	{
		fscanf(fp3,"%d",&gain_offset[i]);
	}
	for(i = 0; i < CALI_TABLE_ONE_ANT; i++)
	{
		save_power[i]+=gain_offset[i];
		if( save_power[i] > CALI_TABLE_MAX_GAIN)
		{
		    save_power[i] = CALI_TABLE_MAX_GAIN;
		}
	}
	fclose(fp3);

	// set gain to 0 mean it is not support
	for (i=0; i<25; i++) {
		if (i < 16) {
			continue;
		} else if (i>=16 && i<24) {
			// ch 132,136,140,144,149,153,157,161 do not have 160M BandWidth
			for (j=0; j<101; j++) {
				if (j>=89 && j<101) {
					save_power[CALI_TABLE_2G_SIZE+i*101+j] = 0;
				}
			}
		} else if (i==24) {
			// ch165 only have 20M BandWidth
			for (j=0; j<101; j++) {
				if ((j>=16 && j<24) || (j>=33 && j<53) || (j>=65 && j<101)) {
					save_power[CALI_TABLE_2G_SIZE+i*101+j] = 0;
				}
			}
		}
	}

/*save the final gain to power_save.txt*/
	if(ant == ANT2)
	{
		fp2 = fopen("/etc/atetools/power_save_ant2.txt","w");
	}else
	{
		fp2 = fopen("/etc/atetools/power_save_ant1.txt","w");
	}

	for(i = 0; i < CALI_TABLE_ONE_ANT; i++)
	{
			if(i < CALI_TABLE_2G_SIZE)
			{
				fprintf(fp2,"%d ",save_power[i]);
					if((i+1)%52 == 0){
						if(i == 0)
							continue;
			  			fprintf(fp2,"\n");
					}
			}
			else
			{
				fprintf(fp2,"%d ",save_power[i]);
					if((i-675)%101 == 0)
			  			fprintf(fp2,"\n");
			}
	}

	fclose(fp2);
	return 0;
}

uint32_t offset_result()
{
	int ret = 0;
	ret = _offset_result(ANT1);
	if(ret)
	{
		printf("offset ant1 table result failed!\r\n");
		return -1;
	}
	else
	{
		printf("offset ant1 table over!\r\n");
	}
	ret = _offset_result(ANT2);
	if(ret)
	{
		printf("offset ant2 table result failed!\r\n");
		return -1;
	}
	else
	{
		printf("offset ant2 table over!\r\n");
	}
	return 0;
}

static void usage()
{
	printf("Usage:\n"
			"  ate_cmd wlan0/1 set/read/write "
			"ATEXXX = XXX \\\n"
			"\n"
		  );
	printf(" ate_cmd wlan0 set ATE = ATESTART, ATE start\n");
	printf(" ate_cmd wlan0 set ATE = ATETXSTART, Tx frame start (-t)\n");
	printf(" ate_cmd wlan0 set ATE = ATETXSTOP, Stop current TX action (-s)\n");
	printf(" ate_cmd wlan0 set ATE = ATERXSTART, RX frame start (-r)\n");
	printf(" ate_cmd wlan0 set ATE = ATERXSTOP, Stop current RX action (-k)\n");
	printf(" ate_cmd wlan0 set ATE = ATESHOW, Show Setting info\n");
	printf(" ate_cmd wlan0 set ATEPHYBW = X,(0~3) Set ATE PHY Bandwidth (-w)\n");
	printf(" ate_cmd wlan0 set ATETXBW = X,(0~3) Set ATE Tx Frame Bandwidth (-u)\n");
	printf(" ate_cmd wlan0 set ATECHAN = X, Set ATE Channel(primmary freq), decimal(-f)\n");
	printf(" ate_cmd wlan0 set ATECENTERFREQ1 = X, Set ATE center freq1, decimal(-c)\n");
	printf(" ate_cmd wlan0 set ATETXNUM = X, Set ATE Tx Frame Number(-n)\n");
	printf(" ate_cmd wlan0 set ATELEN = X, Set ATE Frame Length(-l)\n");
	printf(" ate_cmd wlan0 set ATETXMODE = X,(0~4) Set ATE Tx Mode(-m)\n");
	printf(" ate_cmd wlan0 set ATETXMCS = X, Set ATE Tx MCS type(-i)\n");
	printf(" ate_cmd wlan0 set ATETXGI = X,(0~1) Set ATE Tx Guard Interval(-g)\n");
	printf(" ate_cmd wlan0 set ATETXPOW = XX,(0~31) Set ATE Tx power(-p)\n");
	printf(" ate_cmd wlan0 set ATETXPREAM = X,(0~1) Set ATE Tx pream\n");
	printf(" ate_cmd wlan0 set ATETXDA = XX:XX:XX:XX:XX:XX, Set da\n");
	printf(" ate_cmd wlan0 set ATETXSA = XX:XX:XX:XX:XX:XX, Set sa\n");
	printf(" ate_cmd wlan0 set ATETXBSSID = XX:XX:XX:XX:XX:XX, Set bssid\n");
	printf(" ate_cmd wlan0 read ATEREG = X, Read the Value of One Register, X must be 0xYYYYYYYY or 0xYYYY\n");
	printf(" ate_cmd wlan0 write ATEREG = X(addr)_X(val), Write the Value of One Rergister\n");
	//printf(" ate_cmd wlan0 read ATEBULKREG = type_num_X1_X1_..., Read Many Registers\n");
	//printf(" ate_cmd wlan0 write ATEBULKREG = type_num_addr1_val1_X2_X2..., Write Many Registers\n");
	printf(" ate_cmd wlan0 set ATE = ATEMACHDR -f 0x0008 -t 0x0000 -b 00:aa:bb:cc:dd:ee -d 16:88:aa:bb:cc:dd -s 16:88:aa:bb:cc:dd -q 0x5e20, Set the frame mac header\n");
	printf(" ate_cmd wlan1 fastconfig -v 0xb790b100 //read reg value \n");
	printf(" ate_cmd wlan1 fastconfig -z 0xb7990b100_0x00101010 //write reg value\n" );
	printf(" ate_cmd wlan1 fastconfig -n 0 -l 1024 -f 5500 -c 5530 -w 3 -u 3 -m 4 -i 9 -g 0 -p 28 -t, Transmit frames by fast setting on HB\n");
	printf(" ate_cmd wlan1 fastconfig -l 1024 -f 5500 -c 5530 -w 3 -u 3 -m 4 -i 9 -g 0 -p 28 -b 4096 -y, Transmit frames by macbypass on HB\n");
	printf(" ate_cmd wlan1 fastconfig -q, stop tx by macbypass on HB!!\n");
	printf(" ate_cmd wlan1 fastconfig -f 5180 -c 5180 -w 0 -u 0 -h,Receive frames by macbypass on HB\n");
	printf(" ate_cmd wlan1 fastconfig -j, stop rx by macbypass on HB!!\n");
	printf(" ate_cmd wlan1 fastconfig -f 5500 -c 5500 -w 3 -u 3 -p 28 -o, Transmit test tone on HB\n");
	printf(" ate_cmd wlan1 fastconfig -n 0 -l 1024 -f 5500 -c 5500 -w 1 -u 1 -m 2 -i 7 -g 1 -p 28 -a 16, Transmit aggregated frames by fast setting on HB, 16 is the max agg buffer size\n");
	printf(" ate_cmd save 0x1a1b, Save XO cali caonfig  0x1a1b into falsh\n");
	printf(" ate_cmd wlan1 fastconfig -O 1 , 1  is xo calibrate value from 0 to 255\n");
	printf(" ate_cmd save pow 1 2412 0  0  0 1 ,the parameters are power channel bw mode rate ant;\n" );
	printf(" ate_cmd read pow 2412 0  0  0 1 ,the parameters are channel bw mode rate ant; \n" );
	printf(" if you use (ate_init lb/hb) to active ate tool, only use wlan0\n");
	printf(" if you use (ate_init) to active ate tool, use wlan0 for 2.4G test and wlan1 for 5G test\n");
	printf(" ate_cmd wlan1 fastconfig -p save_all/read_all ,read all_power or save_all power\n");
}

/*
 * getopt --
 *	Parse argc/argv argument vector.
 */

static char *optarg_tmp=NULL;
static int opting_tmp=0;
static int optopt_tmp=0;

/*
char *strchra(const char *arr ,int optopt_tmp){
		char *p=NULL;
		int size=50;
		int i=0;
		for ( i = 0; i < size - 1; i++) {
				if (arr[i] == optopt_tmp) {
						p =(char *)&arr[i];
						return p;
				}
		}
		return NULL;
}
*/

int getopt_tmp(int nargc,char* const nargv[],const char* ostr)

{
	char *place = "";		/* option letter processing */
	char *oli;				/* option letter list index */
	static int optreset=0;
//	printf("%s\n",__func__);
	if (optreset || !*place) {		/* update scanning pointer */
		optreset = 0;
		if (opting_tmp >= nargc){
			optarg_tmp=NULL;
			opting_tmp=0;
			optopt_tmp=0;
			return -1;
		}
		if(*(place =nargv[opting_tmp]) != '-') {
					place = "";
					++opting_tmp;
					return 0;
			}
			if (place[1] && *++place == '-'	/* found "--" */
									&& place[1] == '\0') {
					++opting_tmp;
					place = "";
					return (-1);
			}
	}	/* option letter okay? */
	if ((optopt_tmp = (int)*place++) == (int)':' ||
							!(oli = strchr(ostr, optopt_tmp))) {
			/*
			 * if the user didn't specify '-' as an option,
			 * assume it means -1.
			 */
			if (optopt_tmp == (int)'-')
				  return (-1);
			if (!*place)
				  ++opting_tmp;
			return (-1);
	}
	if (*++oli != ':') {			/* don't need argument */
			optarg_tmp = NULL;
			if (!*place)
				  ++opting_tmp;
	}
	else {					/* need an argument */
			if (*place)			/* no white space */
				  optarg_tmp = place;
			else if (nargc <= ++opting_tmp) {	/* no arg */
					place = "";
					if (*ostr == ':')
						  return (-1);
					return (-1);
			}
			else				/* white space */
				  optarg_tmp = nargv[opting_tmp];
			place = "";
			++opting_tmp;
	}
//	printf("opting %d opt %c optopt_arg %s\n",opting_tmp,optopt_tmp,optarg_tmp);
	return (optopt_tmp);			/* dump back option letter */
}

int32_t main(int32_t argc, char *argv[])
{
	uint8_t j = 1;
	uint8_t i = 0;
	char *bufs="\0";
	char *channel = "\0";
	char *mode= "\0";
	char *rate="\0";
	char *bw ="\0";
	char *nss = "\0";
	char *band = "\0";
	char temp[2] = "00";
	char refpower[2] = "00";
	char calipower[2] = "00";
	char rssi[4] = "0000";
	struct timeval start;
	struct timeval end;
	float use_time;

	gettimeofday(&start,NULL);
    if(argc < 2){
        usage();
        return -1;
    }

	if(strcmp(argv[1],"set_power") == 0){
		set_power_config();
		return 0;
	}

	if(strcmp(argv[1],"tx_calibrate_over") == 0)
	{
		if(offset_result()==0)
			printf("offset calibration value over! now you can wirte to flash..\n");
		return 0;
	}
	if(strcmp(argv[1],"init_power_save") == 0)
	{
		system("cp /usr/bin/power_save_ant1.txt /etc/atetools/");
		system("cp /usr/bin/power_save_ant2.txt /etc/atetools/");
		return 0;
	}

	send_ate_start();
	if(strcmp(argv[1],"save") == 0){
		if (strcmp(argv[2],"pow") == 0){
				uint16_t value;//power
				uint16_t value2;//channel
				uint16_t value3;//bw
				uint16_t value4;//mode
				uint16_t value5;//rate
				uint16_t value6;//ant
				value = atoi(argv[3]);
				value2 = atoi(argv[4]);
				value3 = atoi(argv[5]);
				value4 = atoi(argv[6]);
				value5 = atoi(argv[7]);
				value6 = atoi(argv[8]);
				if (save_power_config_to_flash(value,value2,value3,value4,value5,value6) == -1){
				      printf("save pow fail\r\n");
					return -1;
				}
				goto DONE;
					}
		else if (strcmp(argv[2],"ver") == 0)
		{
			char value[] = "0";
			strcpy(value, argv[3]);
			printf("version is %s\n",value);
			if(save_version_to_factory(value) == -1)
			{
		 		printf("save version fail\r\n");
				return -1;
			}
	         	 goto DONE;
		}
		/* *ate_cmd save temp [value]
		 * *value: use decimal number
		 * */
		else if (strcmp(argv[2],"temp") == 0)
		{
			//get temp
			temp[0] = atoi(argv[3]);
			if(temp[0] < 0)
			{
			   printf("----temperature is %d degrees below zero!!----\n",temp[0]);
			  temp[0] = ~temp[0] + 1;
			  //if use temp below zero ,set the flag 1,default 0;
			  temp[1] = 1;
			}
			if(!save_temp_to_flash(temp))
				printf("----save temperature ok!----\n");
			goto DONE;
		}
		/* *ate_cmd save refpower [value]
		 * *value: use decimal number
		 * */
		else if (strcmp(argv[2],"refpower") == 0)
		{
			//save refpower
			refpower[0] = atoi(argv[3]);
			if(!save_refpower_to_flash(refpower, REFPOWER_OFFSET))
				printf("----save refpower ok!----\n");
			goto DONE;
		}
		else if (strcmp(argv[2],"calipower") == 0)
		{
			//save 5G calipower: 11AX 80M MCS11
			calipower[0] = atoi(argv[3]);
			//save 2.4G calipower: 11AX 40M MCS11
			calipower[1] = atoi(argv[4]);
			if(!save_refpower_to_flash(calipower, CALIPOWER_OFFSET))
				printf("----save calipower ok!----\n");
			goto DONE;
		}
		else if (strcmp(argv[2], "mac") == 0) {
			char value[16] = {0};
			strcpy(value, argv[3]);
			printf("--- want to save mac %s ---\n", value);
			if (save_mac_to_factory(value)) {
				printf("--- save mac failed!!! ---\n");
				return -1;
			}
			printf("--- save mac OK! ---\n");
			goto DONE;
		} else if (strcmp(argv[2], "factory_info") == 0) {
			if (argv[3]) {
				const factory_info_entry *entry = get_factory_info_entry(argv[3]);
				if (entry) {
					int length = entry->length;
					char value[33];

					memset(value, 0, sizeof(value));

					if (argv[4]) {
						switch (entry->type) {
						case TYPE_MAC:
							if (!parse_mac_address(argv[4], value)) {
								printf("--- invalid MAC format (expected xx:xx:xx:xx:xx:xx) ---\n");
								return -1;
							}
							break;
						case TYPE_HEX:
							if (!parse_hex_string(argv[4], value, length)) {
								printf("--- invalid HEX format (expected 0x...) ---\n");
								return -1;
							}
							break;
						case TYPE_INT:
							if (!parse_int_string(argv[4], value, length)) {
								printf("--- invalid INT format (expected decimal number) ---\n");
								return -1;
							}
							break;
						case TYPE_RSSI_OFFSET:
							if (!parse_rssi_offset(argv[4], value, length)) {
								printf("--- invalid RSSI_OFFSET format (expected four decimal numbers between -127 and 127) ---\n");
								printf("--- Example: HB1,HB2,LB1,LB2 <==> 15,15,-15,-15 ---\n");
								return -1;
							}
							break;
						case TYPE_STR:
						default:
							if (strlen(argv[4]) > length) {
								printf("--- input value too long for %s (max length: %u) ---\n",
									argv[3], length);
								printf("Current value length: %zu\n", strlen(argv[4]));
								return -1;
							}
							strncpy(value, argv[4], length);
							value[length] = '\0';
							break;
						}
					}

					printf("--- want to save factory_info %s (offset: %u, length: %u) ---\n",
						argv[3], entry->offset, entry->length);

					printf("Input: %s → Binary: ", argv[4] ? argv[4] : "(null)");
					for (i = 0; i < length; i++) {
						printf("%02x ", (unsigned char)value[i]);
					}
					printf("\n");

					if (save_factory_info_to_factory(value, entry->offset, entry->length)) {
						printf("--- save factory_info failed!!! ---\n");
						return -1;
					}
					printf("--- save factory_info OK! ---\n");
					goto DONE;
				} else {
					printf("--- invalid factory_info field: %s ---\n", argv[3]);
					printf("Valid fields are:\n");
					for (i = 0; i < FACTORY_INFO_COUNT; i++) {
						printf("  %s (offset: %u, length: %u)\n",
							factory_info_table[i].name,
							factory_info_table[i].offset,
							factory_info_table[i].length);
					}
					return -1;
				}
			}
		} else if (strcmp(argv[2], "rssi") == 0) {
			/* ate_cmd save rssi [hb1] [hb2] [lb1] [lb2]
			 * value: use decimal number
			 */
			//get rssi
			rssi[0] = atoi(argv[3]);
			rssi[1] = atoi(argv[4]);
			rssi[2] = atoi(argv[5]);
			rssi[3] = atoi(argv[6]);
			if (save_rssi_offset(rssi) == -1) {
				printf("----save rssi offset failed!----\n");
				return -1;
			}
			printf("----save rssi offset ok!----\n");
			goto DONE;
		} else {
			char value[] = "0";
			strcpy(value, argv[2]);
			if (save_XO_config_to_flash(value))
				return -3;
			goto DONE;
		}
	}

	if(strcmp(argv[1],"read") == 0){
		if (strcmp(argv[2],"pow") == 0){
			if (get_factory_power(argv[3],argv[4],argv[5],argv[6],"0",argv[7]) == -1){
				printf("read pow fail\r\n");
				return -1;
			}
			goto DONE;
			}
        if (strcmp(argv[2],"temp") == 0){
            if (get_temp_from_factory() == -1){
				printf("read temp fail\r\n");
                return -1;
            }
            goto DONE;
        }
		if (strcmp(argv[2],"trx_delay") == 0){
            if (get_trx_delay_from_factory() == -1){
                printf("read trx delay fail\r\n");
                return -1;
            }
            goto DONE;
            }
		if (strcmp(argv[2],"XO_value") == 0){
            if (get_XO_from_factory() == -1){
                printf("read XO value fail\r\n");
                return -1;
            }
            goto DONE;
        }
		else if (strcmp(argv[2], "ver") == 0) {
			if (get_wifi_version_from_factory() == -1) {
				printf("read wifi version fail\r\n");
				return -1;
			}
			goto DONE;
		}
		else if (strcmp(argv[2],"refpower") == 0){
			if (get_refpower_from_factory() == -1){
				printf("read refpower fail\r\n");
				return -1;
			}
			goto DONE;
		}
		else if (strcmp(argv[2],"calipower") == 0){
			if (get_calipower_from_factory() == -1){
				printf("read calipower fail\r\n");
				return -1;
			}
			goto DONE;
		}
		else if (strcmp(argv[2], "mac") == 0) {
			if (get_mac_from_factory() == -1) {
				printf("read macaddr failed!\n");
				return -1;
			}
			goto DONE;
		}
		else if (strcmp(argv[2], "rssi") == 0) {
			if (get_rssi_from_factory() == -1) {
				printf("read rssi offset failed!\n");
				return -1;
			}
			goto DONE;
		} else if (strcmp(argv[2], "factory_info") == 0) {
			if (argv[3]) {
				int i;
				const factory_info_entry *entry = get_factory_info_entry(argv[3]);

				if (entry) {
					if (get_factory_info_from_factory(entry->offset, entry->length, entry->type) == -1) {
						printf("read factory_info failed!\n");
						return -1;
					}
				} else {
					printf("--- invalid factory_info field: %s ---\n", argv[3]);
					printf("Valid fields are:\n");
					for (i = 0; i < FACTORY_INFO_COUNT; i++) {
						printf("  %s (offset: %u, length: %u)\n",
							factory_info_table[i].name,
							factory_info_table[i].offset,
							factory_info_table[i].length);
					}
					return -1;
				}
			}
			goto DONE;
		}
	}

	if (strcmp(argv[1], "cali") == 0) {
		if (strcmp(argv[2], "rssi") == 0) {
			if (cali_rssi_offset(argv[3], argv[4]) == -1) {
				printf("calibrate rssi offset failed!\n");
				return -1;
			}
			goto DONE;
		}
	}

	if(strcmp(argv[1],"fastconfig") != 0)
	{
		strcpy(ifname, argv[1]);
		j=2;
	}
	send_ate_start();
	if(strcmp(argv[j],"fastconfig") == 0)
	{
		int opt;
		char options[] = "trskyqYKoxhjRTa:l:f:c:w:u:m:i:g:B:p:v:z:F:e:d:b:V:O:N:";
#if 0
		set_pkg_da(da);
		set_pkg_sa(sa);
		set_pkg_bssid(bssid);
#endif
		// parse parameters
		//./ate_cmd fastconfig -t -n 10 -l 1024 -f 5520 -w 0 -m 0 -i 0 -g 0

	//	printf("getopt------argc %d  argv %s\n",argc_tmp,argv[j+1]);
		while ((opt = getopt_tmp (argc, argv, options)) != -1)
		{
	//		printf("switch--------\n");
			switch (opt)
			{
				case 't':
					g_tx_frame_flag = true;
					printf("%s -t param g_tx_frame_flag=%d\n", __func__, g_tx_frame_flag);
					break;
				case 'r':
					g_rx_frame_flag = true;
					printf("%s -r param g_rx_frame_flag=%d\n", __func__, g_rx_frame_flag);
					break;
				case 's':
					g_stop_tx_frame_flag = true;
					printf("%s -s param g_stop_tx_frame_flag=%d\n", __func__, g_stop_tx_frame_flag);
					break;
				case 'k':
					g_stop_rx_frame_flag = true;
					printf("%s -k param g_stop_rx_frame_flag=%d\n", __func__, g_stop_rx_frame_flag);
					break;
				case 'y':
					g_start_tx_frame_by_macbypass_flag = true;
					printf("%s -y param g_start_tx_frame_by_macbypass_flag=%d\n", __func__, g_start_tx_frame_by_macbypass_flag);
					break;
				case 'q':
					g_stop_tx_frame_by_macbypass_flag = true;
					printf("%s -q param g_stop_tx_frame_by_macbypass_flag=%d\n", __func__, g_stop_tx_frame_by_macbypass_flag);
					break;
				case 'Y':
					g_start_tx_rx_by_macbypass_flag = true;
					printf("%s -Y param g_start_tx_rx_by_macbypass_flag=%d\n", __func__, g_start_tx_rx_by_macbypass_flag);
					break;
				case 'K':
					g_stop_tx_rx_by_macbypass_flag = true;
					printf("%s -K param g_stop_tx_rx_by_macbypass_flag=%d\n", __func__, g_stop_tx_rx_by_macbypass_flag);
					break;
				case 'o':
					g_tx_test_tone_flag = true;
					printf("%s -o param g_tx_test_tone_flag=%d\n", __func__, g_tx_test_tone_flag);
					break;
				case 'x':
					g_stop_tx_test_tone_flag = true;
					printf("%s -x param g_stop_tx_test_tone_flag=%d\n", __func__, g_stop_tx_test_tone_flag);
					break;
				case 'h':
					g_start_rx_by_macbypass_flag = true;
					printf("%s -h param g_start_rx_by_macbypass_flag=%d\n", __func__, g_start_rx_by_macbypass_flag);
					break;
				case 'j':
					g_stop_rx_by_macbypass_flag = true;
					printf("%s -j param g_stop_rx_by_macbypass_flag=%d\n", __func__, g_stop_rx_by_macbypass_flag);
					break;
				case 'a':
					if(ant_num_set(optarg_tmp))
						return -1;
					break;
				case 'n':
					if(set_pkg_frame_num(optarg_tmp))
						return -1;
					break;
				case 'b':
					if(set_pkg_macbypass_interval(optarg_tmp))
						return -1;
					break;
				case 'l':
					if(set_pkg_frame_length(optarg_tmp))
						return -1;
					break;
				case 'f':
					channel = optarg_tmp;
					if(set_phy_chan_freq(optarg_tmp))
						return -1;
					break;
				case 'c':
					if(set_phy_center_freq1(optarg_tmp))
						return -1;
					break;
				case 'w':
					bw = optarg_tmp;
					if(set_phy_bandwidth(optarg_tmp))
						return -1;
					break;
				case 'u':
					if(set_pkg_bandwidth(optarg_tmp))
						return -1;
					break;
				case 'm':
					mode  = optarg_tmp;
					if(set_phy_mode(optarg_tmp))
						return -1;
					break;
				case 'N':
					nss  = optarg_tmp;
					if(set_tx_nss(optarg_tmp))
						return -1;
					break;
				case 'i':
					rate = optarg_tmp;
					if(set_phy_rate_idx(optarg_tmp))
						return -1;
					break;
				case 'g':
					if(set_phy_use_sgi(optarg_tmp))
						return -1;
					break;
				case 'B':
					band = optarg_tmp;
					printf("####test band%s####\r\n",band);
					break;
				case 'p':
					if(set_phy_power(optarg_tmp,channel,bw,mode,rate,band))
						return -1;
					break;
				case 'v':
					if(read_iw_reg(optarg_tmp))
						return -1;
					break;
				case 'z':
					if(write_iw_reg(optarg_tmp))
						return -1;
					break;
				case 'e':
					if(read_iw_bulk_reg(optarg_tmp))
						return -1;
					break;
				case 'd':
					if(write_iw_bulk_reg(optarg_tmp))
						return -1;
					break;
				case 'V':
					if(set_txvector_param(optarg_tmp))
					      return -1;
					break;
				case 'O':
					if(set_xo_value(optarg_tmp))
					  return -1;
					break;
				case 'R':
					//ioctl get rx result 0/1
					if(get_rx_info())
						return -1;
					break;
				case 'T':
					if(rf_temp_get())
						return -1;
					break;
				case 'F':
					if(set_tx_filter(optarg_tmp))
						return -1;
					break;
				case '?':
				case ':':
					usage();
					break;
			}
		}
		if (g_tx_frame_flag) {
			if(send_tx_start(NULL))
				return -1;
			goto DONE;
		}

		if (g_rx_frame_flag) {
			if(send_rx_start())
				return -1;
			goto DONE;
		}

		if (g_stop_tx_frame_flag) {
			if(send_tx_stop())
				return -1;
			goto DONE;
		}

		if (g_stop_rx_frame_flag) {
			if(send_rx_stop())
				return -1;
			goto DONE;
		}

		if (g_start_tx_rx_by_macbypass_flag) {
			if(send_tx_rx_start_by_macbypass())
				return -1;
			goto DONE;
		}

		if (g_stop_tx_rx_by_macbypass_flag) {
			if(send_tx_rx_stop_by_macbypass())
				return -1;
			goto DONE;
		}

		if (g_start_tx_frame_by_macbypass_flag) {
			if(send_tx_start_by_macbypass())
				return -1;
			goto DONE;
		}

		if (g_stop_tx_frame_by_macbypass_flag) {
			if(send_tx_stop_by_macbypass())
				return -1;
			goto DONE;
		}

		if (g_start_rx_by_macbypass_flag) {
			if(send_rx_start_by_macbypass())
				return -1;
			goto DONE;
		}

		if (g_stop_rx_by_macbypass_flag) {
			if(send_rx_stop_by_macbypass())
				return -1;
			goto DONE;
		}

		if (g_tx_test_tone_flag) {
			if(send_tx_test_tone_start())
				return -1;
			g_tx_test_tone_flag = false;
			goto DONE;
		}

		if (g_stop_tx_test_tone_flag) {
			if(send_tx_test_tone_stop())
				return -1;
			g_stop_tx_test_tone_flag = false;
			goto DONE;
		}

		if (g_agg_tx_frame_flag) {
			if(send_tx_start(bufs))
				return -1;
			g_agg_tx_frame_flag = false;
			goto DONE;
		}
		goto DONE;
	}

	// ./ate_cmd wlan0 set XXX = XXX
	if(argc > 5 && strcmp(argv[0],"ate_cmd") == 0 && strcmp(argv[4],"=") == 0)
	{
		char *tmpdata;
	    int length_array = sizeof(beacon);
		// ./ate_cmd wlanx set XXX = XXX
		tmpdata=(char *)malloc(200);

		tmpdata = argv[5];

		if(strcmp(argv[3],"ATE") == 0 && strcmp(argv[5],"ATESTART") == 0){
			if(send_ate_start())
				return -1;
		}
		if(strcmp(argv[3],"ATE") == 0 && strcmp(argv[5],"ATESTOP") == 0){
			if(send_ate_stop())
				return -1;
		}
		//command id SFCFG_CMD_ATE_START 0x0000

		if(strcmp(argv[3],"ATE") == 0 && strcmp(argv[5],"ATETXSTART") == 0){
			if(send_tx_start(NULL))
				return -1;
		}
		//command id SFCFG_CMD_ATE_TX_START 0x0002

		if(strcmp(argv[3],"ATE") == 0 && strcmp(argv[5],"ATETXSTOP") == 0){
			if(send_tx_stop())
				return -1;
		}
		if(strcmp(argv[3],"ATE") == 0 && strcmp(argv[5],"ATERXSTART") == 0){
			if(send_rx_start())
				return -1;
		}
		if(strcmp(argv[3],"ATE") == 0 && strcmp(argv[5],"ATERXSTOP") == 0){
			if(send_rx_stop())
				return -1;
		}

		if(strcmp(argv[3],"ATE") == 0 && strcmp(argv[5],"ATESHOW") == 0){
			if(get_ate_info())
				return -1;
		}
		//config the frame mac head content
		//./ate_cmd wlan0 set ATE = ATEMACHDR -f 0x0080 -t 0x0000 -b 16:88:aa:bb:cc:dd -d 16:88:aa:bb:cc:dd -s 16:88:aa:bb:cc:dd -q 0x5e20
		if( strcmp(argv[5],"ATEMACHDR") == 0)
		{
			int opt;
			char options[] ="f:t:b:d:s:q:";
			while ((opt = getopt_tmp (argc, argv, options)) != -1)
			{
				switch (opt)
				{
					case 'f':
						set_pkg_fc(optarg_tmp);
						break;
					case 't':
						set_pkg_dur(optarg_tmp);
						break;
					case 'b':
						set_pkg_bssid(optarg_tmp);
						break;
					case 'd':
						set_pkg_da(optarg_tmp);
						break;
					case 's':
						set_pkg_sa(optarg_tmp);
						break;
					case 'q':
						set_pkg_seqc(optarg_tmp);
						break;
					case ':':
						usage();
						break;
				}
			}
		}

		//./ate_cmd wlan0 set ATETXFC = 0x0080
		if(strcmp(argv[3],"ATETXFC") == 0){
			if(set_pkg_fc(tmpdata))
				return -1;
		}
		//./ate_cmd wlan0 set ATETXDUR = 0x0000
		if(strcmp(argv[3],"ATETXDUR") == 0){
			if(set_pkg_dur(tmpdata))
				return -1;
		}
		//./ate_cmd wlan0 set ATETXDA = 00:16:88:08:07:f9
		if(strcmp(argv[3],"ATETXDA") == 0){
			if(set_pkg_da(tmpdata))
				return -1;
		}
		if(strcmp(argv[3],"ATETXSA") == 0){
			if(set_pkg_sa(tmpdata))
				return -1;
		}
		if(strcmp(argv[3],"ATETXBSSID") == 0){
			if(set_pkg_bssid(tmpdata))
				return -1;
		}
		//./ate_cmd wlan0 set ATETXSEQC = 0x5e20
		if(strcmp(argv[3],"ATETXSEQC") == 0){
			if(set_pkg_seqc(tmpdata))
				return -1;
		}
		if(strcmp(argv[3],"ATEPHYBW") == 0){
			if(set_phy_bandwidth(tmpdata))
				return -1;
		}
		if(strcmp(argv[3],"ATETXBW") == 0){
			if(set_pkg_bandwidth(tmpdata))
				return -1;
		}
		if(strcmp(argv[3],"ATETXNUM") == 0){
			if(set_pkg_frame_num(tmpdata))
				return -1;
		}
		if(strcmp(argv[3],"ATECHAN") == 0){
			if(set_phy_chan_freq(tmpdata))
				return -1;
		}
		if(strcmp(argv[3],"ATECENTERFREQ1") == 0){
			if(set_phy_center_freq1(tmpdata))
				return -1;
		}
		if(strcmp(argv[3],"ATETXMODE") == 0){
			if(set_phy_mode(tmpdata))
				return -1;
		}
		if(strcmp(argv[3],"ATETXMCS") == 0){
			if(set_phy_rate_idx(tmpdata))
				return -1;
		}
		if(strcmp(argv[3],"ATETXGI") == 0){
			if(set_phy_use_sgi(tmpdata))
				return -1;
		}
		if(strcmp(argv[3],"ATETXPREAM") == 0){
			if(set_phy_preamble(tmpdata))
				return -1;
		}
		if(strcmp(argv[3],"ATETXPOW") == 0){
			if(set_phy_power(tmpdata,channel,bw,mode,rate,"0"))
				return -1;
		}
		if(strcmp(argv[3],"ATELEN") == 0){
			if(set_pkg_frame_length(tmpdata))
				return -1;
		}
		// ./ate_cmd wlan0 set ATE = ATEWHOLEFRAME
		if(strcmp(argv[5],"ATEWHOLEFRAME") == 0){
			if(set_pkg_whole_frame(beacon,length_array))
				return -1;
		}
		if(strcmp(argv[3],"ATEPAYLOAD") == 0){
			if(set_pkg_payload(tmpdata))
				return -1;
		}
		if(strcmp(argv[3],"ATEREG") == 0 && strcmp(argv[2],"read") ==0){
			if(read_iw_reg(tmpdata))
				return -1;
		}
		if(strcmp(argv[3],"ATEREG") == 0 && strcmp(argv[2],"write") == 0){
			if(write_iw_reg(tmpdata))
				return -1;
		}
		if(strcmp(argv[3],"ATEBULKREG") == 0 && strcmp(argv[2],"read") ==0){
			if(read_iw_bulk_reg(tmpdata))
				return -1;
		}
		if(strcmp(argv[3],"ATEBULKREG") == 0 && strcmp(argv[2],"write") == 0){
			if(write_iw_bulk_reg(tmpdata))
				return -1;
		}
	}else{
		usage();
    }

DONE:
	close_ioctl_socket();
	gettimeofday(&end,NULL);
	use_time=(end.tv_sec-start.tv_sec)*1000000+(end.tv_usec-start.tv_usec);
//	printf("time_use is %.10f\n",use_time);
	return 0;
}
