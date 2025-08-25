FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI += " \
    file://wlan.network \
"

do_install:append() {
    install -d ${D}${sysconfdir}/systemd/network
    install -m 0644 ${WORKDIR}/wlan.network ${D}${sysconfdir}/systemd/network
    rm -f ${D}${sysconfdir}/resolv.conf
    ln -s ${sysconfdir}/resolv-conf.systemd ${D}${sysconfdir}/resolv.conf
}

FILES:${PN} += " \
    ${sysconfdir}/systemd/network/wlan.network \
    ${sysconfdir}/resolv.conf \
"
