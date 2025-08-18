SUMMARY = "QTI package group for COQOS base packages"

inherit packagegroup

PACKAGES = "packagegroup-coqos-base"

# COQOS Trace
RDEPENDS:${PN} += "\
    trace-emlog \
    "

ALLOW_EMPTY:${PN} = "1"
