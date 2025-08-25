SUMMARY = "COQOS HV public interface headers (ABI)"

LICENSE = "GPL-2.0-only | BSD-3-Clause"

LIC_FILES_CHKSUM = " \
        file://${QTI_LICENSE_DIR}/GPL-2.0-only;md5=801f80980d171dd6425610833a22dbe6 \
"

SRC_URI = " \
    ${PATH_TO_REPO}/coqoshv-abi/.git;protocol=${PROTO};destsuffix=coqoshv-abi;branch=coqoshv-abi.qc.1.0 \
"

SRCREV = "${AUTOREV}"
SRC_DIR = "${SRC_DIR_ROOT}/coqoshv-abi"

S = "${WORKDIR}/coqoshv-abi"

inherit allarch

do_install() {
    mkdir -p ${D}${includedir}/coqoshv-abi-kernel-headers
    cp -r ${S}/* ${D}${includedir}/coqoshv-abi-kernel-headers
}
