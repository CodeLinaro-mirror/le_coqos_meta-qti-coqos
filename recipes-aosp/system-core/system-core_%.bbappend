FILESEXTRAPATHS_append := ":${THISDIR}/files"

SRC_URI += "file://0001-post_boot-Set-SA8155-GPU-clock-to-700MHz.patch"
SRC_URI += "file://0002-post_boot-Disable-CPU-isolation-for-core7.patch"
SRC_URI += "file://0003-post_boot-Switch-cpufreq-governor-to-performance.patch"
SRC_URI += "file://adbd"

do_install:append() {
    install -d ${D}/${sysconfdir}/default
    install -m 755 ${WORKDIR}/adbd ${D}/${sysconfdir}/default/
}

FILES:${PN}-adbd += "\
    ${sysconfdir}/default/adbd \
"
