SUMMARY = "QTI package group for COQOS base packages"

inherit packagegroup

PACKAGES = "packagegroup-coqos-base"

RDEPENDS:${PN} += "\
    device-tree \
    dnsmasq \
    trace-emlog \
    trace-cmd \
    virglrenderer \
    "

ALLOW_EMPTY:${PN} = "1"
