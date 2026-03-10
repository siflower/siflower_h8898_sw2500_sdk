include $(TOPDIR)/rules.mk

PKG_NAME:=sf_kernel_common
PKG_RELEASE:=$(AUTORELEASE)

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

define KernelPackage/sf_kernel_common
  SUBMENU:=Network Devices
  TITLE:=SIFLOWER Kernel Common API Driver
  FILES:= $(PKG_BUILD_DIR)/sf_kernel_common.ko
  AUTOLOAD:=$(call AutoLoad, 40, sf_kernel_common)
endef


EXTRA_KCONFIG:= CONFIG_SIFLOWER_KERNEL_COMMON=m

ifdef CONFIG_KERNEL_KALLSYMS
NOSTDINC_FLAGS += -DCONFIG_KALLSYMS
endif

INC_DIR:=$(shell pwd)/../include
NOSTDINC_FLAGS += \
    $(KERNEL_NOSTDINC_FLAGS) \
    -I$(PKG_BUILD_DIR) \
    -I$(INC_DIR) \
    -Werror

define KernelPackage/sf_kernel_common/description
 Kernel module to siflower sf_kernel_common driver.
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


$(eval $(call KernelPackage,sf_kernel_common))
