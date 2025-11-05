FILESEXTRAPATHS_append := ":${THISDIR}/files"

SRC_URI_append = " file://0001-pulseaudio.service-Run-PulseAudio-as-system-wide-dae.patch"
SRC_URI_append = " file://0002-pulseaudio.service-Remove-dependency-to-pulseaudio.s.patch"
SRC_URI_append = " file://0003-system.pa-Apply-auth-anonymous-1-modification-direct.patch"
SRC_URI_append = " file://0004-system.pa-Fix-missing-loading-of-agl-audio-plugin-mo.patch"
SRC_URI_append = " file://0001-Add-acdb-and-codec-control-modules-loading-to-fix-mi.patch"
SRC_URI_append = " file://0001-Start-pulseaudio-after-servicemanager-initializes-de.patch"
SRC_URI_append = " file://0001-pulseaudio.service-Enable-all-Audio-TDM-paths-before.patch"

do_install_append() {
    install -d ${D}${systemd_system_unitdir}/
    install -d ${D}${systemd_system_unitdir}/multi-user.target.wants

    mv ${D}${systemd_user_unitdir}/pulseaudio.service ${D}${systemd_system_unitdir}
    ln -sf ${systemd_system_unitdir}/pulseaudio.service ${D}${systemd_system_unitdir}/multi-user.target.wants/pulseaudio.service

    rm ${D}${systemd_user_unitdir}/default.target.wants/pulseaudio.service
    rm ${D}${systemd_user_unitdir}/sockets.target.wants/pulseaudio.socket
}

FILES_${PN}-server_append = " ${systemd_system_unitdir}/*"
