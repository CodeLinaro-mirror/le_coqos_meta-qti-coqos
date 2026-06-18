FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

SRC_URI += "file://iio.rules"

do_install:append() {
    if ${@bb.utils.contains('DISTRO_FEATURES', 'systemd', 'true', 'false', d)}; then
        install -d ${D}${sysconfdir}/udev/rules.d/
        install -m 0444 ${WORKDIR}/iio.rules ${D}${sysconfdir}/udev/rules.d/iio.rules
    fi
}
