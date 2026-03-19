SUMMARY = "Interface between VIRTIO device implementation and Linux device reset controller headers"
HOMEPAGE = "https://git.codelinaro.org"
LICENSE = "GPLv2.0-with-linux-syscall-note"
LIC_FILES_CHKSUM = "file://${QTI_LICENSE_DIR}/${LICENSE};md5=8afb6abdac9a14cb18a0d6c9c151e9b4"

require qcvirtio-urst-driver-linux.inc
inherit module-headers

do_install() {
    install -d ${D}/${includedir}/linux/
    install -m 644 ${S}/include/uapi/linux/urst_ioctl.h ${D}/${includedir}/linux/
}
