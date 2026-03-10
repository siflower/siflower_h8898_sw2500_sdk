include $(TOPDIR)/rules.mk

PKG_NAME:=sf_genl_user
PKG_VERSION:=1.0
PKG_RELEASE:=1
PKG_BUILD_DIR:= $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

define Package/sf_genl_user
  CATEGORY:=Siflower
  TITLE:=Siflower Generic Netlink UBUS Interface
  DEPENDS:=+libnl +libnl-genl +libubus +libubox
endef

define Package/sf_genl_user/description
  Siflower Generic Netlink UBUS Interface
endef

define Build/Configure
endef

ifdef CONFIG_SIFLOWER_DPNS_TMU
TARGET_CFLAGS += -DCONFIG_SIFLOWER_GENL_TMU
endif

ifdef CONFIG_SIFLOWER_DPNS_MCAST
TARGET_CFLAGS += -DCONFIG_SIFLOWER_GENL_MCAST
endif

ifdef CONFIG_SIFLOWER_DPNS_ACL
TARGET_CFLAGS += -DCONFIG_SIFLOWER_GENL_ACL
endif

DPNS_INC_DIR := $(shell pwd)/../dpns/src

ifneq ($(findstring bpi,$(PROFILE)),)
EXE_FILE_NAME="bpi_exe"
endif
ifneq ($(findstring bpi-rv2-nand,$(PROFILE)),)
EXE_FILE_NAME="bpi-rv2-nand_exe"
endif
ifneq ($(findstring siflower-one-nand,$(PROFILE)),)
EXE_FILE_NAME="one-nand_exe"
endif
ifneq ($(findstring siflower-one-nor,$(PROFILE)),)
EXE_FILE_NAME="one-nor_exe"
endif
ifneq ($(findstring gw-bc-nand,$(PROFILE)),)
EXE_FILE_NAME="gw-bc-nand_exe"
endif
define Build/Compile
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./$(EXE_FILE_NAME)/* $(PKG_BUILD_DIR)/
endef

define Package/sf_genl_user/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/sf_genl_user $(1)/usr/sbin
	$(INSTALL_BIN) ./files/limit.sh $(1)/usr/sbin
	$(INSTALL_BIN) ./files/dev.sh $(1)/usr/sbin
	$(INSTALL_BIN) ./files/genl_init $(1)/etc/init.d/
	$(INSTALL_BIN) ./files/devlist $(1)/etc/config/
endef

$(eval $(call BuildPackage,sf_genl_user))
