SUMMARY = "QTI BSP Linux guest device tree with Coqos modifications."
DESCRIPTION = "QTI BSP Linux guest device tree with Coqos modifications."

LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/BSD-3-Clause;md5=550794465ba0ec5312d6919e203a55f9"

inherit devicetree

PROVIDES = "virtual/linux-dtb"

DT_INCLUDE += "${STAGING_KERNEL_DIR}/arch/${ARCH}/boot/dts/vendor/qcom/"

# Enforce 'phandle' generation compatible with YOCTO kernel build
DTC_FLAGS += " --phandle epapr --symbols"

COMPATIBLE_MACHINE_sa81x5 = ".*"

SRC_URI_${BASEMACHINE} = " \
    file://sa8155p-adp-air-initial.dts \
    file://sa8155p-minic-edk2.dtsi \
    "
