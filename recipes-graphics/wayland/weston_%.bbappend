PACKAGECONFIG_append = " systemd"

FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

SRC_URI += "\
    file://weston.service \
    file://weston_early.service \
    file://weston.ini \
"

# Enable ivi-shell
do_install_append() {
    if ${@bb.utils.contains('DISTRO_FEATURES', 'early_init', 'true', 'false', d)}; then
        install -m 644 -p -D ${WORKDIR}/weston_early.service ${D}${systemd_system_unitdir}/weston.service
    else
        install -m 644 -p -D ${WORKDIR}/weston.service ${D}${systemd_system_unitdir}/weston.service
    fi

    WESTON_INI_CONFIG=${sysconfdir}/xdg/weston
    install -d ${D}${WESTON_INI_CONFIG}
    install -m 0644 ${WORKDIR}/weston.ini ${D}${WESTON_INI_CONFIG}/weston.ini
}
