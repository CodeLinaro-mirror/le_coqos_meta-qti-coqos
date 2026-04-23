SUMMARY = "Reboot driver"
HOMEPAGE = "https://git.codelinaro.org"
LICENSE = "GPL-2.0-only"
LIC_FILES_CHKSUM = "file:///${QTI_LICENSE_DIR}/${LICENSE};md5=801f80980d171dd6425610833a22dbe6"

DEPENDS += "virtual/kernel"

REBOOT_DRIVER_REPO_PATH = "vendor/qcom/opensource/bootloader-reboot-driver-linux"

SRC_URI = "\
    ${PATH_TO_REPO}/${REBOOT_DRIVER_REPO_PATH}/.git;protocol=${PROTO};destsuffix=${REBOOT_DRIVER_REPO_PATH};usehead=1 \
    file://99-coqos-vm-reboot.rules \
"

SRCREV = "${AUTOREV}"
S = "${WORKDIR}/${REBOOT_DRIVER_REPO_PATH}"

inherit module qti-kernel-arch-clang

MODULE_NAME = "coqos-vm-reboot"
KERNEL_MODULE_AUTOLOAD:append = " ${MODULE_NAME}"

EXTRA_OEMAKE += "KDIR=${STAGING_KERNEL_DIR}"
MODULES_INSTALL_TARGET = ""
MAKE_TARGETS = ""

do_install:append() {
    # Copy udev rule
    install -d ${D}${sysconfdir}/udev/rules.d
    install -m 0644 ${WORKDIR}/99-coqos-vm-reboot.rules ${D}${sysconfdir}/udev/rules.d

    install -d ${D}${nonarch_base_libdir}/modules/${KERNEL_VERSION}/extra
    install -m 0644 ${S}/${MODULE_NAME}.ko ${D}${nonarch_base_libdir}/modules/${KERNEL_VERSION}/extra
}

FILES:${PN} += "\
    ${nonarch_base_libdir}/modules/${KERNEL_VERSION}/extra/${MODULE_NAME}.ko \
    ${sysconfdir}/udev/rules.d/99-coqos-vm-reboot.rules \
"

RPROVIDES:${PN} = "kernel-module-${MODULE_NAME}-${KERNEL_VERSION}"
