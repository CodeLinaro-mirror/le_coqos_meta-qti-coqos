QCVIRTIO_RTC_HOST_DRIVERS_REPO_PATH = "vendor/qcom/opensource/qcvirtio-host-drivers/qcvirtio-rtc-host-driver-linux"
SRC_URI = "${PATH_TO_REPO}/${QCVIRTIO_RTC_HOST_DRIVERS_REPO_PATH}/.git;protocol=${PROTO};destsuffix=${QCVIRTIO_RTC_HOST_DRIVERS_REPO_PATH};usehead=1"
SRCREV = "${AUTOREV}"
S = "${WORKDIR}/${QCVIRTIO_RTC_HOST_DRIVERS_REPO_PATH}"
