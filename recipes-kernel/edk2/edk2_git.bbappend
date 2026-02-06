FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI:append = " \
    ${@bb.utils.contains('DISTRO_FEATURES', 'silent', 'file://0001-Reduce-number-of-bootloader-logs-in-silent-mode.patch', '', d)} \
"
