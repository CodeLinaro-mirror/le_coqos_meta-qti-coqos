QCVIRTIO_URST_REPO_PATH = "vendor/qcom/opensource/qcvirtio-host-drivers/qcvirtio-urst-driver-linux"
SRC_URI = "${PATH_TO_REPO}/${QCVIRTIO_URST_REPO_PATH}/.git;protocol=${PROTO};destsuffix=${QCVIRTIO_URST_REPO_PATH};usehead=1"
SRCREV = "${AUTOREV}"
S = "${WORKDIR}/${QCVIRTIO_URST_REPO_PATH}"
