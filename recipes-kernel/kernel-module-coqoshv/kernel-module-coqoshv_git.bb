SUMMARY = "Interface between VIRTIO device implementation and COQOS HV"

require kernel-module-coqoshv_git.inc
inherit module qti-kernel-arch-clang

SRC_URI += " \
    file://Makefile;subdir=${COQOSHV_GUEST_DRIVER_LINUX_PATH} \
    file://coqoshv.conf \
    file://coqoshv.rules \
"

# Do not add a suffix to the kernel module package name,
# so that coqoshv.rules and the module are included in the same package
KERNEL_MODULE_PACKAGE_SUFFIX = ""

do_install:append() {
    # Create destination directories
    install -d ${D}/lib/modules/${KERNEL_VERSION}/extra/

    # Copy kernel module
    install -m 644 ${B}/coqoshv.ko ${D}/lib/modules/${KERNEL_VERSION}/extra/

    # Copy udev rule
    install -d ${D}/etc/udev/rules.d/
    install -m 644 ${WORKDIR}/coqoshv.rules ${D}/etc/udev/rules.d/

    # copy modules-load
    install -d ${D}/etc/modules-load.d/
    install -m 644 ${WORKDIR}/coqoshv.conf ${D}/etc/modules-load.d/
}

FILES:${PN}:append = " \
    /etc/udev/rules.d/coqoshv.rules \
    /etc/modules-load.d/coqoshv.conf \
    "
