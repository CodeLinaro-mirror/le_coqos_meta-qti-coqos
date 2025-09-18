SUMMARY = "QTI package group for COQOS base packages"

inherit packagegroup

PACKAGES = "packagegroup-coqos-base"

# COQOS Trace and device-tree
RDEPENDS:${PN} += "\
    device-tree \
    trace-emlog \
    virglrenderer \
    "

# Linux adbd-relay
RDEPENDS:${PN} += "\
    adbd-relay \
    "

ALLOW_EMPTY:${PN} = "1"
