#
# Copyright (C) 2010-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=rm500u-cnv_demo
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/rm500u-cnv_demo
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=rm500u-cnv_demo
endef

define Package/rm500u-cnv_demo/description
	this is a 5G module of Quectel RM500U-CNV supporting package
endef

define Build/Compile
endef

define Build/Configure
endef

define Package/rm500u-cnv_demo/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) ./files/active_ep_mode.sh $(1)/bin/active_ep_mode.sh
	$(INSTALL_BIN) ./files/auto_dial_up_networking.sh $(1)/bin/auto_dial_up_networking.sh
	$(INSTALL_BIN) ./files/manual_dial_up_networking.sh $(1)/bin/manual_dial_up_networking.sh

endef

$(eval $(call BuildPackage,rm500u-cnv_demo))

