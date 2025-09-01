inherit qti-kernel-arch-clang

FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI += " \
    file://Makefile;subdir=git \
    file://0001-emlog.c-Add-noop-implementation-for-fsync-operation.patch \
    file://emlog.rules \
"

KERNEL_CC = "${CC} -fuse-ld=bfd"

EXTRA_OEMAKE += "KDIR='${STAGING_KERNEL_DIR}'"
MAKE_TARGETS = "all"

KERNEL_MODULE_AUTOLOAD += "emlog"
KERNEL_MODULE_PROBECONF += "emlog"
module_conf_emlog = "options emlog emlog_max_size=32768"

KERNEL_MODULE_PACKAGE_SUFFIX = ""

do_install:append() {
    # Copy udev rule
    install -d ${D}/etc/udev/rules.d/
    install -m 644 ${WORKDIR}/emlog.rules ${D}/etc/udev/rules.d/
}

FILES:${PN} += " \
    /lib/modules/${KERNEL_VERSION}/extra/emlog.ko \
    /etc/udev/rules.d/emlog.rules \
"
