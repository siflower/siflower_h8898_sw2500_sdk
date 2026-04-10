#
# Copyright (C) 2015-2018 neil.zhang@siflower.com.cn
#
# sifiower Software.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ate_cmd
PKG_VERSION:=1
PKG_RELEASE:=1
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
PKG_BUILD_DEPENDS:=mtd-utils

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/ate_cmd
SECTION:=utils
CATEGORY:=Utilities
TITLE:=atetools.
MAINTAINER:=Star jiang <star.jiang@siflower.com.cn>
DEPENDS:=+libc +libpthread +librt +libubus
endef

define Package/ate_cmd/description
atetest tool.
endef

define Build/Configure
	rm -f $(PKG_BUILD_DIR)/CMakeCache.txt
	rm -rf $(PKG_BUILD_DIR)/CMakeFiles

	$(call Build/Configure/Default)
endef

define Package/ate_cmd/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/etc/atetools
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/ate_cmd $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/ate_server $(1)/usr/bin
	$(CP) ./files/*  $(1)/etc/atetools/
	$(CP) ./files/*  $(1)/usr/bin/
endef


$(eval $(call BuildPackage,ate_cmd))
