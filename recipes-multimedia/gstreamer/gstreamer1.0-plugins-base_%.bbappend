FILESEXTRAPATHS_append := ":${THISDIR}/${PN}"

SRC_URI_append = " file://0001-wayland-fix-build-break-in-yocto.patch"
SRC_URI_append = " file://0001-appsrc-clear-eos-flag-on-flush-stop-event.patch"
