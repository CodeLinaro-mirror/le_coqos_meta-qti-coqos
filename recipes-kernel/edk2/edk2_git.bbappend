FILESEXTRAPATHS_append := ":${THISDIR}/files"

SRC_URI += "file://0001-avb-bring-up-keymaster-for-LV.patch \
            file://0002-avb-send-dummy-ROT-and-boot-state-to-keymaster-from-.patch \
            file://0003-QcomModulePkg-Allow-lv-load-recovery-partition.patch \
            file://0004-BootLinux-use-fastboot-header-info-if-kernel_addr-0x.patch \
            file://0005-QcomModulePkg-KeyMaster-don-t-start-twice.patch \
            file://0006-Handle-outer-device-tree-case-in-boot-image.patch \
            file://0007-Skip-device-tree-memory-node-modification.patch \
            file://0008-Store-device-serial-number-in-dedicated-memory.patch \
            file://0009-Remove-keymaster-boot-parameters-setup.patch \
            file://0010-update-DevInfo-struct-to-share-it-with-guest-bootloader.patch \
            file://0011-provide-boot-tamper-state-to-tz.patch \
            file://0012-blow-milestone-fuse.patch \
            file://0013-Store-boot-recovery-wipe-command-for-GVM-in-case-of-.patch \
            "
