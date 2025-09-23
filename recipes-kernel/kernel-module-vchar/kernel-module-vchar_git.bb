SUMMARY = "VCHAR"

LICENSE = "GPL-2.0-only & GPL-2.0-only-WITH-Linux-syscall-note"

LIC_FILES_CHKSUM = " \
    file://${QTI_LICENSE_DIR}/GPL-2.0-only;md5=801f80980d171dd6425610833a22dbe6 \
    file://${QTI_LICENSE_DIR}/GPL-2.0-only-WITH-Linux-syscall-note;md5=7725c035f5c9707decb7cf301273aecc \
"

inherit module
inherit qti-kernel-arch-clang

KERNEL_CC = "${CC} -fuse-ld=bfd"
DEPENDS = "coqoshv-abi"

SRC_URI = " \
    ${PATH_TO_REPO}/vchar-km/.git;protocol=${PROTO};destsuffix=vchar-km;branch=vchar-kernel-module.lnx.1.0 \
"

SRCREV = "${AUTOREV}"

S = "${WORKDIR}/vchar-km"

EXTRA_OEMAKE += "-C linux COQOSHV_ABI_HEADERS=${STAGING_DIR_HOST}/usr/include/coqoshv-abi-kernel-headers"

MODULES_MODULE_SYMVERS_LOCATION := "linux"

do_install:append() {
    # Copy kernel module
    install -d ${D}/lib/modules/${KERNEL_VERSION}/extra/
    install -m 644 ${B}/linux/ixcf-vchar.ko ${D}/lib/modules/${KERNEL_VERSION}/extra/

    # Create destination directory for headers
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
