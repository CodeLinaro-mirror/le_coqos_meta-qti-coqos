SUMMARY = "Interface between VIRTIO device implementation and COQOS HV headers."

require kernel-module-coqoshv_git.inc
inherit module-headers

do_install() {
    install -d ${D}/${includedir}/linux/
    install -m 644 ${S}/include/uapi/linux/coqoshv.h ${D}/${includedir}/linux/
}
