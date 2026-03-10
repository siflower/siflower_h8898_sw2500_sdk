include $(TOPDIR)/rules.mk

PKG_NAME:=dpns
PKG_RELEASE:=$(AUTORELEASE)
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

define KernelPackage/dpns
  SUBMENU:=Network Devices
  TITLE:=SIFLOWER DPNS Driver
  DEPENDS:=+kmod-siflower-eth-dma
  FILES:=$(PKG_BUILD_DIR)/dpns.ko
  AUTOLOAD:=$(call AutoLoad, 50, dpns)
endef

define KernelPackage/dpns/config
if PACKAGE_kmod-dpns
	config SIFLOWER_DPNS_GENL
		bool "Siflower DPNS COMMON generic netlink support"
		default "y"

	config SIFLOWER_DPNS_PROC
		bool "Siflower DPNS COMMON generic proc_fs support"
		default "y"

	config SIFLOWER_DPNS_VLAN
		bool "Siflower DPNS vlan support"
		default "y"

	config SIFLOWER_DPNS_L2
		bool "Siflower DPNS l2 support"
		default "y"

	config SIFLOWER_DPNS_NAT
		bool "Siflower DPNS nat support"
		default "y"

	config ELK_DSNAT_SINGLE
		bool "use the same memory area for both external D/SNAPT"
		depends on SIFLOWER_DPNS_NAT

	config SIFLOWER_DPNS_ROUTER
		bool "Siflower DPNS router support"
		default "y"

	config SIFLOWER_DPNS_MCAST
		bool "Siflower DPNS mcast support"
		default "y"

	config SIFLOWER_DPNS_ACL
		bool "Siflower DPNS acl support"
		default "y"

	config SIFLOWER_DPNS_TMU
		bool "Siflower DPNS tmu support"
		default "y"

	choice
		prompt "dpns tmu buffer config"
		default DPNS_THROUGHPUT_BALANCE
		depends on SIFLOWER_DPNS_TMU
		help
		  This option allows you to select tmu buffer config mode.

	config DPNS_THROUGHPUT_BALANCE
		bool "Config DPNS TMU buffer balanced between wifi and eth"
		help
		  Tmu buffer configed balanced between wifi and eth.

	config DPNS_THROUGHPUT_WIFI_BEST
		bool "Config DPNS TMU buffer wifi best"
		help
		  Tmu buffer configed wifi best.

	config DPNS_THROUGHPUT_ETH_BEST
		bool "Config DPNS TMU buffer eth best"
		help
		  Tmu buffer configed ethernet best.
	endchoice
endif
endef

EXTRA_KCONFIG += CONFIG_SIFLOWER_DPNS=m
NOSTDINC_FLAGS += -DCONFIG_SIFLOWER_DPNS_COMMON

ifdef CONFIG_SIFLOWER_DPNS_GENL
EXTRA_KCONFIG += CONFIG_SIFLOWER_DPNS_GENL=y
NOSTDINC_FLAGS += -DCONFIG_SIFLOWER_DPNS_GENL
endif

ifdef CONFIG_SIFLOWER_DPNS_PROC
EXTRA_KCONFIG += CONFIG_SIFLOWER_DPNS_PROC=y
NOSTDINC_FLAGS += -DCONFIG_SIFLOWER_DPNS_PROC
endif

ifdef CONFIG_SIFLOWER_DPNS_VLAN
EXTRA_KCONFIG += CONFIG_SIFLOWER_DPNS_VLAN=y
NOSTDINC_FLAGS += -DCONFIG_SIFLOWER_DPNS_VLAN
endif

ifdef CONFIG_SIFLOWER_DPNS_NAT
EXTRA_KCONFIG += CONFIG_SIFLOWER_DPNS_NAT=y
NOSTDINC_FLAGS += -DCONFIG_SIFLOWER_DPNS_NAT
endif

ifdef CONFIG_ELK_DSNAT_SINGLE
NOSTDINC_FLAGS += -DCONFIG_ELK_DSNAT_SINGLE
endif

ifdef CONFIG_SIFLOWER_DPNS_L2
EXTRA_KCONFIG+= CONFIG_SIFLOWER_DPNS_L2=y
NOSTDINC_FLAGS += -DCONFIG_SIFLOWER_DPNS_L2
endif

ifdef CONFIG_SIFLOWER_DPNS_ROUTER
EXTRA_KCONFIG += CONFIG_SIFLOWER_DPNS_ROUTER=y
NOSTDINC_FLAGS += -DCONFIG_SIFLOWER_DPNS_ROUTER
endif

ifdef CONFIG_SIFLOWER_DPNS_MCAST
EXTRA_KCONFIG += CONFIG_SIFLOWER_DPNS_MCAST=y
NOSTDINC_FLAGS += -DCONFIG_SIFLOWER_DPNS_MCAST
endif

ifdef CONFIG_SIFLOWER_DPNS_ACL
EXTRA_KCONFIG += CONFIG_SIFLOWER_DPNS_ACL=y
NOSTDINC_FLAGS += -DCONFIG_SIFLOWER_DPNS_ACL
endif

ifdef CONFIG_SIFLOWER_DPNS_TMU
EXTRA_KCONFIG += CONFIG_SIFLOWER_DPNS_TMU=y
NOSTDINC_FLAGS += -DCONFIG_SIFLOWER_DPNS_TMU
endif

ifdef CONFIG_DPNS_THROUGHPUT_BALANCE
NOSTDINC_FLAGS += -DCONFIG_DPNS_THROUGHPUT_BALANCE
endif

ifdef CONFIG_DPNS_THROUGHPUT_WIFI_BEST
NOSTDINC_FLAGS += -DCONFIG_DPNS_THROUGHPUT_WIFI_BEST
endif

ifdef CONFIG_DPNS_THROUGHPUT_ETH_BEST
NOSTDINC_FLAGS += -DCONFIG_DPNS_THROUGHPUT_ETH_BEST
endif

ifdef CONFIG_NET_SIFLOWER_ETH_TSO
NOSTDINC_FLAGS += -DCONFIG_NET_SIFLOWER_ETH_TSO
endif

PKG_CONFIG_DEPENDS := \
	CONFIG_SIFLOWER_DPNS_GENL \
	CONFIG_SIFLOWER_DPNS_PROC \
	CONFIG_SIFLOWER_DPNS_VLAN \
	CONFIG_SIFLOWER_DPNS_NAT \
	CONFIG_ELK_DSNAT_SINGLE \
	CONFIG_SIFLOWER_DPNS_L2 \
	CONFIG_SIFLOWER_DPNS_ROUTER \
	CONFIG_SIFLOWER_DPNS_MCAST \
	CONFIG_SIFLOWER_DPNS_ACL \
	CONFIG_SIFLOWER_DPNS_TMU \
	CONFIG_DPNS_THROUGHPUT_BALANCE \
	CONFIG_DPNS_THROUGHPUT_WIFI_BEST \
	CONFIG_DPNS_THROUGHPUT_ETH_BEST \
	CONFIG_NET_SIFLOWER_ETH_TSO

INC_DIR := $(shell pwd)/../include
LIF_DIR := $(shell pwd)/../sf_lif/src
NOSTDINC_FLAGS += \
    $(KERNEL_NOSTDINC_FLAGS) \
    -I$(PKG_BUILD_DIR) \
    -I$(INC_DIR) \
    -I$(LIF_DIR) \
    -Werror

define KernelPackage/dpns/description
 Kernel module to siflower dpns driver.
endef

define KernelPackage/dpns/install
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) ./files/* $(1)/sbin
endef

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

define Build/Clean
        rm -rf $(dir $(PKG_BUILD_DIR))dpns*
        rm -rf $(dir $(PKG_BUILD_DIR))sf*
        rm -rf $(dir $(PKG_BUILD_DIR))siflower-*
endef

$(eval $(call KernelPackage,dpns))
