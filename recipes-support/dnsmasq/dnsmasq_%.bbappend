FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI += "\
    file://coqos.conf \
    file://dnsmasq.conf \
"

do_install:append() {
    install -d ${D}${systemd_system_unitdir}/dnsmasq.service.d
    install -m 0644 ${WORKDIR}/coqos.conf ${D}${systemd_system_unitdir}/dnsmasq.service.d
}

FILES:${PN} += "${systemd_system_unitdir}/dnsmasq.service.d/coqos.conf"
