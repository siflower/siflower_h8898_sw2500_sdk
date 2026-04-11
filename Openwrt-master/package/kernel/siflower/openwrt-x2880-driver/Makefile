include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=sf_wifi
PKG_VERSION:=1.0
PKG_RELEASE:=$(AUTORELEASE)
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

# Default enable for Siflower's own device configurations
CONFIG_PACKAGE_SFUMAC_DPNS_ENABLE=y

define KernelPackage/sf_wifi
  SUBMENU:=Wireless Drivers
  TITLE:=SIFLOWER wireless driver
  DEPENDS:=+kmod-cfg80211 +@DRIVER_11N_SUPPORT +@DRIVER_11AC_SUPPORT +@DRIVER_11AX_SUPPORT

ifdef CONFIG_PACKAGE_SFUMAC_DPNS_ENABLE
  DEPENDS += +kmod-siflower-eth-dma +kmod-dpns
endif

  FILES +=\
          $(PKG_BUILD_DIR)/siwifi_fmac.ko
  AUTOLOAD:=$(call AutoLoad, 60, siwifi_fmac)
endef

NOSTDINC_FLAGS := \
    $(KERNEL_NOSTDINC_FLAGS) \
    -I$(PKG_BUILD_DIR) \
    -I$(STAGING_DIR)/usr/include/mac80211-backport/uapi \
    -I$(STAGING_DIR)/usr/include/mac80211-backport \
    -I$(STAGING_DIR)/usr/include/mac80211/uapi \
    -I$(STAGING_DIR)/usr/include/mac80211 \
    -include backport/backport.h

define KernelPackage/sf_wifi/config
	if PACKAGE_kmod-sf_wifi
        config PACKAGE_SFUMAC_RF_VERSION_1
			bool "rf version 01"
			default "n"
	endif
endef

EXTRA_KCONFIG += \
	CONFIG_SIWIFI_FMAC=y

ifdef CONFIG_PACKAGE_SFUMAC_DPNS_ENABLE
EXTRA_KCONFIG += CONFIG_PACKAGE_SFUMAC_DPNS_ENABLE=y
endif

PROFILE=$(call qstrip,$(CONFIG_TARGET_PROFILE))

ifeq ($(filter $(PROFILE), DEVICE_siflower-bpi-nor DEVICE_6826sgsrb DEVICE_6826sg), $(PROFILE))
NOSTDINC_FLAGS += -DCONFIG_SIWIFI_RF_XO_OFF
ifneq ($(filter $(PROFILE), DEVICE_6826sg), $(PROFILE))
CONFIG_SIWIFI_SRB_CHIP=y
NOSTDINC_FLAGS += -DCONFIG_SIWIFI_SRB_CHIP
endif
endif

KO_FILE_DIR="wifi_ko"

CONFIG_PACKAGE_WLAN_VENDOR_SIFLOWER=y
ifdef CONFIG_PACKAGE_WLAN_VENDOR_SIFLOWER
SF_ARCH=wlan_vendor_siflower
endif
CONFIG_PACKAGE_SFUMAC_WIFI_TEST_SCRIPTS=y

NOSTDINC_FLAGS += -DCONFIG_SF16A18_WIFI_ATE_TOOLS
#NOSTDINC_FLAGS += -DATE_MAC_CONFIG

define KernelPackage/sf_wifi/description
 Kernel module to siflower fullmac wlan driver.
endef

ifdef CONFIG_PACKAGE_SFUMAC_RF_VERSION_1
NOSTDINC_FLAGS += -DCONFIG_PACKAGE_SFUMAC_RF_VERSION_1
endif

SFWIFI_COMPILE_TIME=$(shell date +%s)
NOSTDINC_FLAGS += -DCONFIG_COMPILE_TIME=$(SFWIFI_COMPILE_TIME)

NOSTDINC_FLAGS += \
	-I$(PKG_BUILD_DIR)  \
    -Werror

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -r ./config $(PKG_BUILD_DIR)/
	$(CP) -r ./test $(PKG_BUILD_DIR)/
	$(CP) ./$(KO_FILE_DIR)/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
endef

define KernelPackage/sf_wifi/install
	$(INSTALL_DIR) $(1)/lib/firmware

	cp $(PKG_BUILD_DIR)/config/sfwifi.sh $(1)/lib/sfwifi.sh
	$(INSTALL_DIR) $(1)/bin
	cp $(PKG_BUILD_DIR)/config/sfwifi $(1)/bin/
	cp $(PKG_BUILD_DIR)/config/reload_wifi.sh $(1)/bin/
	cp $(PKG_BUILD_DIR)/config/$(SF_ARCH)/sfwifi_lb_fmac.bin $(1)/lib/firmware/
	cp $(PKG_BUILD_DIR)/config/$(SF_ARCH)/sfwifi_hb_fmac.bin $(1)/lib/firmware/
	cp $(PKG_BUILD_DIR)/config/$(SF_ARCH)/agcram_lb.bin $(1)/lib/firmware/
	cp $(PKG_BUILD_DIR)/config/$(SF_ARCH)/agcram_lb40.bin $(1)/lib/firmware/
	cp $(PKG_BUILD_DIR)/config/$(SF_ARCH)/agcram_hb.bin $(1)/lib/firmware/
	cp $(PKG_BUILD_DIR)/config/$(SF_ARCH)/agcram_hb20.bin $(1)/lib/firmware/
	cp $(PKG_BUILD_DIR)/config/$(SF_ARCH)/agcram_hb40.bin $(1)/lib/firmware/
	cp $(PKG_BUILD_DIR)/config/$(SF_ARCH)/iq_engine.bin $(1)/lib/firmware/
	cp $(PKG_BUILD_DIR)/config/siwifi_settings.ini $(1)/lib/firmware/
	cp $(PKG_BUILD_DIR)/config/siwifi_blacklist_lb.ini $(1)/lib/firmware/
	cp $(PKG_BUILD_DIR)/config/siwifi_blacklist_hb.ini $(1)/lib/firmware/
	cp $(PKG_BUILD_DIR)/config/siwifi_whitelist_lb.ini $(1)/lib/firmware/
	cp $(PKG_BUILD_DIR)/config/siwifi_whitelist_hb.ini $(1)/lib/firmware/
	cp $(PKG_BUILD_DIR)/config/txp_offset_sleep.ini $(1)/lib/firmware/
	cp $(PKG_BUILD_DIR)/config/txp_offset_low.ini $(1)/lib/firmware/
	cp $(PKG_BUILD_DIR)/config/siwifi_fem_ctrl.ini $(1)/lib/firmware/
	cp $(PKG_BUILD_DIR)/config/la_diags/*.txt $(1)/lib/firmware/

ifdef CONFIG_PACKAGE_SFUMAC_RF_VERSION_1
	cp $(PKG_BUILD_DIR)/config/$(SF_ARCH)/sfwifi_iq_calibrate_rf01.bin $(1)/lib/firmware/sfwifi_iq_calibrate.bin
else
	cp $(PKG_BUILD_DIR)/config/$(SF_ARCH)/sfwifi_iq_calibrate.bin $(1)/lib/firmware/sfwifi_iq_calibrate.bin
endif

ifdef CONFIG_SIWIFI_SRB_CHIP
	cp $(PKG_BUILD_DIR)/config/$(SF_ARCH)/srb/* $(1)/lib/firmware/
endif

	$(INSTALL_DIR) $(1)/lib/preinit

ifdef CONFIG_PACKAGE_SFUMAC_WIFI_TEST_SCRIPTS
	cp -r $(PKG_BUILD_DIR)/test/. $(1)/bin/
endif

ifndef CONFIG_PACKAGE_siwifitools
	cp $(PKG_BUILD_DIR)/config/siwifi_umh_nothing.sh  $(1)/bin/siwifi_umh.sh
endif

endef

$(eval $(call KernelPackage,sf_wifi))
