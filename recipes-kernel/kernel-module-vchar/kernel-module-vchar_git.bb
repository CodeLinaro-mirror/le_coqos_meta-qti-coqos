SUMMARY = "VCHAR"

LICENSE = "GPL-2.0-only & GPL-2.0-only-WITH-Linux-syscall-note"

LIC_FILES_CHKSUM = " \
        file://${QTI_LICENSE_DIR}/GPL-2.0-only;md5=801f80980d171dd6425610833a22dbe6 \
"

inherit module
inherit qti-kernel-arch-clang

KERNEL_CC = "${CC} -fuse-ld=bfd"
DEPENDS = "coqoshv-abi"

SRC_URI = " \
    file://include;subdir=kernel-module-vchar \
    file://linux;subdir=kernel-module-vchar \
    file://common;subdir=kernel-module-vchar \
"

SRCREV = "${AUTOREV}"

S = "${WORKDIR}/kernel-module-vchar"

EXTRA_OEMAKE += "-C linux COQOSHV_ABI_HEADERS=${STAGING_DIR_HOST}/usr/include/coqoshv-abi-kernel-headers"

# disable debug_info to avoid error from QTI internal
# binary-symbols-checker.bbclass
EXTRA_OEMAKE += "KBUILD_CFLAGS_MODULE+=-g0"

do_install_append() {
    # Create destination directories
    install -d ${D}/${includedir}/vchar-kernel-headers

    # Install kernel headers
    install -m 644 ${S}/include/vchar.h ${D}/${includedir}/vchar-kernel-headers
    install -m 644 ${S}/include/linux/vchar.h ${D}/${includedir}/vchar-kernel-headers/linux
    install -m 644 ${S}/include/uapi/vchar.h ${D}/${includedir}/vchar-kernel-headers/uapi
    install -m 644 ${S}/include/vring.h ${D}/${includedir}/vchar-kernel-headers
    install -m 644 ${S}/include/linux/vring.h ${D}/${includedir}/vchar-kernel-headers/linux
    install -m 644 ${S}/include/linux/ixcf.h ${D}/${includedir}/vchar-kernel-headers/linux
    install -m 644 ${S}/include/linux/arch/arm64/ixcf-cache.h ${D}/${includedir}/vchar-kernel-headers

    # install userspace header
    install -m 644 ${S}/include/uapi/vchar.h ${D}/${includedir}
}
