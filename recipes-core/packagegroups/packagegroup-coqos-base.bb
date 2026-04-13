SUMMARY = "QTI package group for COQOS base packages"

inherit packagegroup

PACKAGES = "packagegroup-coqos-base"

RDEPENDS:${PN} += "\
    device-tree \
    dnsmasq \
    trace-emlog \
    trace-cmd \
    virglrenderer \
    vulkan-layers-validation \
    vulkan-tools \
    "

# Linux adbd-relay
RDEPENDS:${PN} += "\
    adbd-relay \
    "

# Test tools
RDEPENDS:${PN} += "\
    i2c-tools \
    spitools \
    "

ALLOW_EMPTY:${PN} = "1"
