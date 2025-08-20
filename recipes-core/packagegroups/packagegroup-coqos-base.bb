SUMMARY = "QTI package group for COQOS base packages"

inherit packagegroup

PACKAGES = "packagegroup-coqos-base"

# COQOS Trace and device-tree
RDEPENDS:${PN} += "\
    device-tree \
    trace-emlog \
    "

ALLOW_EMPTY:${PN} = "1"
