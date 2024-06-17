SUMMARY = "QTI package group for COQOS base packages"

inherit packagegroup

PACKAGES = "packagegroup-coqos-base"

# COQOS Trace
RDEPENDS:${PN} += "\
    trace-emlog \
    virglrenderer \
    "

ALLOW_EMPTY:${PN} = "1"
