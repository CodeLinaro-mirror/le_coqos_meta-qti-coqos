COQOS_SEPOLICY_DIR := "${THISDIR}"
CONTRIB_MODULES += " coqos_virtio dnsmasq logging systemd pulseaudio"

POLICY_MODULES_EXCLUDE += "bluetooth"

do_copy_coqos_sepolicy_modules() {
    cp -rf ${COQOS_SEPOLICY_DIR}/coqos-sepolicy ${WORKDIR}/
}

do_patch:append() {
    install_device_policy(d, "coqos-sepolicy")
}

addtask do_copy_coqos_sepolicy_modules after do_unpack before do_patch
