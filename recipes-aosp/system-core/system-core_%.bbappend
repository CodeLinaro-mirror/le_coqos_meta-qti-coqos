FILESEXTRAPATHS_append := ":${THISDIR}/files"

SRC_URI += "file://0001-post_boot-Set-SA8155-GPU-clock-to-700MHz.patch"
SRC_URI += "file://0002-post_boot-Disable-CPU-isolation-for-core7.patch"
