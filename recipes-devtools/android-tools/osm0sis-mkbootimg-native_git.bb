SUMMARY = "Android mkbootimg and unpackbootimg tools"
DESCRIPTION = "Boot image creation tool from AOSP, a prerequisite for use \
with imageForge (coqoshv-tools) which requires offset parameters support when \
generating fastboot images with mkbootimg."

LICENSE = "BSD-3-Clause & Apache-2.0"
LIC_FILES_CHKSUM = " \
    file://NOTICE;md5=c19179f3430fd533888100ab6616e114 \
    file://mkbootimg.c;beginline=1;endline=15;md5=309583d146c9a413240584c19630c0bf \
"

inherit native

SRC_URI = " \
    git://git@github.com/osm0sis/mkbootimg.git;protocol=https;branch=master;rebaseable=1 \
"
SRCREV = "d4a2677828fe9b60117af8996dcf1dea85d6b431"

S = "${WORKDIR}/git"

do_install() {
    install -d -m 0755 ${D}${STAGING_DIR_NATIVE}/usr/share/coqos/${BPN}
    for file in mkbootimg unpackbootimg; do
        install -m 0755 ${S}/${file} ${D}${STAGING_DIR_NATIVE}/usr/share/coqos/${BPN}
    done
}

# Skip the unwanted steps
do_configure[noexec] = "1"
