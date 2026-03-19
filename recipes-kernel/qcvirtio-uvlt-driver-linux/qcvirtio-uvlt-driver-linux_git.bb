SUMMARY = "Interface between VIRTIO device implementation and Linux device voltage regulators"
HOMEPAGE = "https://git.codelinaro.org"
LICENSE = "GPL-2.0-only"
LIC_FILES_CHKSUM = "file:///${QTI_LICENSE_DIR}/${LICENSE};md5=801f80980d171dd6425610833a22dbe6"

DEPENDS += "virtual/kernel"

require qcvirtio-uvlt-driver-linux.inc
inherit module qti-kernel-arch-clang

SRC_URI:append = " \
    file://coqos-uvlt.rules \
"
KERNEL_MODULE_AUTOLOAD = "coqos-uvlt"

# Avoid adding prefix, that will let us to include coqoshv.rules in the same package
KERNEL_MODULE_PACKAGE_SUFFIX = ""

EXTRA_OEMAKE += "KDIR=${STAGING_KERNEL_DIR}"
MODULES_INSTALL_TARGET = ""
MAKE_TARGETS = ""

do_install:append() {
    # Copy udev rule
    install -d ${D}${sysconfdir}/udev/rules.d/
    install -m 0644 ${WORKDIR}/coqos-uvlt.rules ${D}${sysconfdir}/udev/rules.d/

    install -d ${D}${nonarch_base_libdir}/modules/${KERNEL_VERSION}/extra/
    install -m 0644 ${S}/coqos-uvlt.ko ${D}${nonarch_base_libdir}/modules/${KERNEL_VERSION}/extra/
}

FILES:${PN} += "\
    ${nonarch_base_libdir}/modules/${KERNEL_VERSION}/extra/coqos-uvlt.ko \
    ${sysconfdir}/udev/rules.d/coqos-uvlt.rules \
"

RPROVIDES:${PN} = "kernel-module-coqos-uvlt-${KERNEL_VERSION}"
