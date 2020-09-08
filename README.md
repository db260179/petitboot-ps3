# Quick build howto for PS3 petitboot Openwrt

## If you want to add more packages

cd openwrt/package

`ln -sf ../../openwrt_packages/libs/jpeg jpeg

ln -sf ../../openwrt_packages/libs/libpng libpng

ln -sf ../../openwrt_packages/libs/libtwin libtwin

ln -sf ../../openwrt_packages/utils/petitboot petitboot

ln -sf ../../openwrt_packages/utils/zip zip

ln -sf ../../openwrt_packages/utils/unzip unzip

ln -sf ../../openwrt_packages/utils/vim vim

ln -sf ../../openwrt_packages/utils/ntfs-3g ntfs-3g

ln -sf ../../openwrt_packages/utils/coreutils coreutils

ln -sf ../../openwrt_packages/utils/tar tar

ln -sf ../../openwrt_packages/utils/gzip gzip

ln -sf ../../openwrt_packages/utils/bzip2 bzip2

ln -sf ../../openwrt_packages/utils/dosfstools dosfstools

ln -sf ../../openwrt_packages/utils/less less

ln -sf ../../openwrt_packages/utils/sed sed

ln -sf ../../openwrt_packages/utils/sdparm sdparm

ln -sf ../../openwrt_packages/utils/hdparm hdparm

ln -sf ../../openwrt_packages/utils/bash bash

ln -sf ../../openwrt_packages/net/wget wget

ln -sf ../../openwrt_packages/net/dhcpcd dhcpcd

ln -sf ../../openwrt_packages/net/net-tools net-tools`

`cd ..`

`cp ps3_petitboot_config .config`

`make menuconfig # If you want to change options`

`make kernel_menuconfig # Change kernel modules etc`

`make V=99 # Single thread only build`
