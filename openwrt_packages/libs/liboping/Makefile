#
# Copyright (C) 2009-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=liboping
PKG_VERSION:=1.6.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=http://verplant.org/liboping/files
PKG_MD5SUM:=1f5cfb17feb38b5aec94fbd36607cbed

PKG_FIXUP:=libtool

PKG_INSTALL:=1

include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/package.mk

define Package/liboping/Default
  URL:=http://verplant.org/liboping/
endef

define Package/liboping
$(call Package/liboping/Default)
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=C library to generate ICMP echo requests.
endef

define Package/oping
$(call Package/liboping/Default)
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Send ICMP echo request to network hosts
  DEPENDS+= +liboping
endef

define Package/noping
$(call Package/liboping/Default)
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Ncurses application to send ICMP echo request to network hosts
  DEPENDS+= +liboping +libncurses
endef

TARGET_CFLAGS += $(FPIC)

CONFIGURE_ARGS += \
	--without-perl-bindings \
	--enable-shared \
	--enable-static

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/oping.h $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/liboping.{a,so*} $(1)/usr/lib/
endef

define Package/liboping/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/liboping.so.* $(1)/usr/lib/
endef

define Package/oping/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/oping $(1)/usr/bin/
endef

define Package/noping/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/noping $(1)/usr/bin/
endef

$(eval $(call BuildPackage,liboping))
$(eval $(call BuildPackage,oping))
$(eval $(call BuildPackage,noping))
