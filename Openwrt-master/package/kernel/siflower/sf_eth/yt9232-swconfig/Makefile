include $(TOPDIR)/rules.mk

PKG_NAME:=yt9232-swconfig
PKG_RELEASE:=1.1.005

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

define KernelPackage/yt9232-swconfig
  SUBMENU:=Network Devices
  TITLE:=Motorcomm YT9232 Ethernet Switch Driver
  FILES:=$(PKG_BUILD_DIR)/yt9232_swconfig.ko
  AUTOLOAD:=$(call AutoLoad,48,yt9232_swconfig)
endef

define KernelPackage/yt9232-swconfig/description
  swconfig driver for YT9232
endef

define KernelPackage/yt9232-swconfig/config
	if PACKAGE_kmod-yt9232-swconfig
		config SIFLOWER_MPW1_ESWITCH_LIGHT
			bool "build light eswitch driver"
			default "n"
	endif
endef

COPTS = -DBOARD_MAX_UNIT_NUM=1

ifdef CONFIG_SIFLOWER_FULLMASK_ESWITCH_LIGHT
EXTRA_KCONFIG += CONFIG_SIFLOWER_FULLMASK_ESWITCH_LIGHT=y
NOSTDINC_FLAGS += -DCONFIG_SIFLOWER_FULLMASK_ESWITCH_LIGHT
endif

ifneq ($(findstring bpi,$(PROFILE)),)
KO_FILE_NAME="bpi_ko"
endif
ifneq ($(findstring bpi-rv2-nand,$(PROFILE)),)
KO_FILE_NAME="bpi-rv2-nand_ko"
endif
ifneq ($(findstring siflower-one-nand,$(PROFILE)),)
KO_FILE_NAME="one-nand_ko"
endif
ifneq ($(findstring siflower-one-nor,$(PROFILE)),)
KO_FILE_NAME="one-nor_ko"
endif
ifneq ($(findstring gw-bc-nand,$(PROFILE)),)
KO_FILE_NAME="gw-bc-nand_ko"
endif
define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./$(KO_FILE_NAME)/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
endef

$(eval $(call KernelPackage,yt9232-swconfig))
