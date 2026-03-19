PACKAGECONFIG_append = " systemd"

FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

SRC_URI += "\
    file://weston.service \
    file://weston_early.service \
    file://weston.ini \
    file://qcom_background.png \
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

    # Install background image
    install -d ${D}/${datadir}/weston
    install -m 0644 ${WORKDIR}/qcom_background.png ${D}/${datadir}/weston/qcom_background.png
}
