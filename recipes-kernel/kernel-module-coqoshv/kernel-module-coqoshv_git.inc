LICENSE = "GPL-2.0-only"
LIC_FILES_CHKSUM = "file://coqoshv.h;endline=4;md5=d6bc01964cf40db31a67cee7d80c6b7d"

COQOSHV_GUEST_DRIVER_LINUX_PATH = "coqoshv-guest-driver-linux"
SRC_URI = "${PATH_TO_REPO}/${COQOSHV_GUEST_DRIVER_LINUX_PATH}/.git;protocol=${PROTO};branch=coqoshv-guest-driver.lnx.1.15.0;destsuffix=${COQOSHV_GUEST_DRIVER_LINUX_PATH};usehead=1"

SRCREV = "${AUTOREV}"

SRC_DIR = "${SRC_DIR_ROOT}/${COQOSHV_GUEST_DRIVER_LINUX_PATH}"
S = "${WORKDIR}/${COQOSHV_GUEST_DRIVER_LINUX_PATH}"
