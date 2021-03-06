#
# Copyright (C) 2006-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=miredo
PKG_VERSION:=1.2.3
PKG_RELEASE:=1

PKG_SOURCE_URL:=http://www.remlab.net/files/miredo/
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_MD5SUM:=6ba484ad5e331514a8a3452da2b144e7

PKG_FIXUP:=libtool
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/miredo/Default
  SECTION:=ipv6
  CATEGORY:=IPv6
  SUBMENU:=miredo: Teredo (IPv6 tunneling over UDP through NAT)
  URL:=http://www.remlab.net/miredo/
endef

define Package/miredo-common
$(call Package/miredo/Default)
  TITLE:=Teredo (IPv6 tunneling over UDP through NAT) shared libraries
  DEPENDS+= +libpthread +uclibcxx +kmod-ipv6 +kmod-tun
endef

define Package/miredo-server
$(call Package/miredo/Default)
  TITLE:=Teredo (IPv6 tunneling over UDP through NAT) server daemon
  DEPENDS+= +miredo-common
endef

define Package/miredo-server/conffiles
/etc/miredo/miredo-server.conf
endef

define Package/miredo-client
$(call Package/miredo/Default)
  TITLE:=Teredo (IPv6 tunneling over UDP through NAT) client and relay daemon
  DEPENDS+= +miredo-common
endef

define Package/miredo-client/conffiles
/etc/miredo/miredo.conf
endef

CONFIGURE_ARGS += \
	--enable-shared \
	--enable-static \
	--with-gnu-ld \
	--disable-rpath \
	--disable-chroot \
	--enable-teredo-client \
	--enable-miredo-user=root \
	--without-Judy \

CONFIGURE_VARS += \
	CPPFLAGS="$$$$CPPFLAGS -I$(STAGING_DIR)/usr/include/uClibc++" \
	CXXFLAGS="$$$$CXXFLAGS -fno-builtin -fno-rtti -nostdinc++"  \
	LIBS="-nodefaultlibs -luClibc++ -lm" \
	ac_cv_file__proc_self_maps=yes\

# XXX: override (:=), don't append (+=)
MAKE_FLAGS := \
	CXXLINK="\$$$$(LINK)" \

define Package/miredo-common/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/lib{teredo,tun6}.so.* $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/miredo
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/miredo/miredo-privproc $(1)/usr/lib/miredo
endef

define Package/miredo-server/install
	$(INSTALL_DIR) $(1)/etc/miredo
	$(CP) $(PKG_INSTALL_DIR)/usr/share/doc/miredo/examples/miredo-server.conf $(1)/etc/miredo/miredo-server.conf
	$(INSTALL_BIN) ./files/client-hook $(1)/etc/miredo/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/miredo-server.init $(1)/etc/init.d/miredo-server
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/miredo-server $(1)/usr/sbin/
endef

define Package/miredo-client/install
	$(INSTALL_DIR) $(1)/etc/miredo
	$(CP) $(PKG_INSTALL_DIR)/usr/share/doc/miredo/examples/miredo.conf $(1)/etc/miredo/miredo.conf
	$(INSTALL_BIN) ./files/client-hook $(1)/etc/miredo/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/miredo.init $(1)/etc/init.d/miredo
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/miredo $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,miredo-common))
$(eval $(call BuildPackage,miredo-server))
$(eval $(call BuildPackage,miredo-client))
