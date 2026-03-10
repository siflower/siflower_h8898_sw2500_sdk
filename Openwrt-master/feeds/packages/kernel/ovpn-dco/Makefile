#
# Copyright (C) 2021 Jianhui Zhao <zhaojh329@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=ovpn-dco
PKG_UPSTREAM_KERNEL_VERSION:=development-net-6.16.0-rc3
PKG_VERSION:=2025070100
PKG_RELEASE:=1
PKG_BUILD_PARALLEL:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL=https://codeload.github.com/OpenVPN/ovpn-backports/tar.gz/refs/tags/ovpn-net-next/$(PKG_UPSTREAM_KERNEL_VERSION)-$(PKG_VERSION)?
PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/ovpn-backports-ovpn-net-next-$(PKG_UPSTREAM_KERNEL_VERSION)-$(PKG_VERSION)
PKG_HASH:=5e30e4808764d1a29b284f5a9151eaae2c51a49ad340176936339296929fdccc

PKG_MAINTAINER:=Jianhui Zhao <zhaojh329@gmail.com>
PKG_LICENSE:=GPL-2.0-only


include $(INCLUDE_DIR)/package.mk

define KernelPackage/ovpn-backports
  SUBMENU:=Network Support
  TITLE:=OpenVPN data channel offload
  DEPENDS:=+kmod-crypto-aead +kmod-udptunnel4 +IPV6:kmod-udptunnel6 +kmod-crypto-gcm +kmod-crypto-lib-chacha20poly1305 +kmod-crypto-chacha20poly1305
  FILES:=$(PKG_BUILD_DIR)/drivers/net/ovpn/ovpn.ko
  KCONFIG:=CONFIG_STREAM_PARSER=y
  AUTOLOAD:=$(call AutoLoad,30,ovpn)
endef

define KernelPackage/ovpn-backports/description
  This module enhances the performance of the OpenVPN userspace software
  by offloading the data channel processing to kernelspace.
endef

NOSTDINC_FLAGS += \
	$(KERNEL_NOSTDINC_FLAGS) \
	-I$(PKG_BUILD_DIR)/include \
	-I$(PKG_BUILD_DIR)/compat-include \
	-include $(PKG_BUILD_DIR)/linux-compat.h

EXTRA_KCONFIG:= \
	CONFIG_OVPN=m

PKG_EXTMOD_SUBDIRS = drivers/net/ovpn

MAKE_OPTS:= \
	$(KERNEL_MAKE_FLAGS) \
	M="$(PKG_BUILD_DIR)/drivers/net/ovpn" \
	NOSTDINC_FLAGS="$(NOSTDINC_FLAGS)" \
	$(EXTRA_KCONFIG)

define Build/Compile
	+$(MAKE) $(PKG_JOBS) -C "$(LINUX_DIR)" \
		$(MAKE_OPTS) \
		modules
endef

$(eval $(call KernelPackage,ovpn-backports))
