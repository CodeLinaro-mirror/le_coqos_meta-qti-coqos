do_install:append() {
    if ${@bb.utils.contains('DISTRO_FEATURES', 'early_init', 'true', 'false', d)}; then
        sed -i -e '/\[audio\]/{n;n;s/msleep=100/msleep=200/}' ${D}${sysconfdir}/early_init.conf
    fi
}
