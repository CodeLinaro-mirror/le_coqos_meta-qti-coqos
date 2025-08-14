SUMMARY = "Circular buffer for the COQOS Trace read service logs using emlog"
DESCRIPTION = "A replacement of the default /var/log/cs-trace.log file with an emlog based circular buffer."
HOMEPAGE = "https://git.codelinaro.org/"

LICENSE = "BSD-3-Clause-Clear"
LIC_FILES_CHKSUM = "file://${QTI_LICENSE_DIR}/${LICENSE};md5=b796c0007db682166a1721da80267bb2"

SRC_URI = "file://trace-emlog.service"

inherit systemd

RDEPENDS:${PN} = "kernel-module-emlog emlog"
DEPENDS = "systemd"

SYSTEMD_SERVICE:${PN} = "trace-emlog.service"

do_install:append() {
    install -d ${D}${systemd_system_unitdir}
    install -m 0644 ${WORKDIR}/trace-emlog.service ${D}${systemd_system_unitdir}
}

FILES:${PN} += "${systemd_system_unitdir}/trace-emlog.service"
