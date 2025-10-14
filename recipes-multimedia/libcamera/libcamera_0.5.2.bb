SUMMARY = "Linux libcamera framework"
SECTION = "libs"

LICENSE = "GPL-2.0-or-later & LGPL-2.1-or-later"

LIC_FILES_CHKSUM = "\
    file://LICENSES/GPL-2.0-or-later.txt;md5=fed54355545ffd980b814dab4a3b312c \
    file://LICENSES/LGPL-2.1-or-later.txt;md5=2a4f4fd2128ea2f65047ee63fbca9f68 \
"

SRC_URI = " \
    git://git.libcamera.org/libcamera/libcamera.git;protocol=https;nobranch=1;rebaseable=1 \
    file://0001-libcamera-process-Fix-compilation-for-some-aarch64-B.patch \
    file://0002-libcamera-pipeline-Introduce-v4l2-loopback-pipeline-.patch \
    file://0003-Backport-vivid-pipeline-support-into-v0.5.2.patch \
    file://0004-libcamera-pipeline-vivid-Add-support-of-multiple-cap.patch \
    file://0005-libcamera-pipeline-vivid-Fix-format-config-validatio.patch \
"

SRCREV = "aa0a91c48ddb38c302390d5c4899cb9e093ddd24"

PE = "1"

S = "${WORKDIR}/git"

DEPENDS = "python3-pyyaml-native python3-jinja2-native python3-ply-native python3-jinja2-native udev openssl chrpath-native libevent libyaml"
DEPENDS += "${@bb.utils.contains('DISTRO_FEATURES', 'qt', 'qtbase qtbase-native', '', d)}"

PACKAGES =+ "${PN}-gst"

PACKAGECONFIG ??= ""
PACKAGECONFIG[gst] = "-Dgstreamer=enabled,-Dgstreamer=disabled,gstreamer1.0 gstreamer1.0-plugins-base"

EXTRA_OEMESON = " \
    -Dpipelines=uvcvideo,v4l2-loopback,vivid \
    -Dv4l2=disabled \
    -Dcam=enabled \
    -Dlc-compliance=disabled \
    -Dtest=false \
    -Ddocumentation=disabled \
"

RDEPENDS_${PN} = "${@bb.utils.contains('DISTRO_FEATURES', 'wayland qt', 'qtwayland', '', d)}"

inherit meson pkgconfig python3native
DEPENDS_remove = " meson-native"
DEPENDS_append = " meson0.63.3-native"

do_configure_prepend() {
    sed -i -e 's|py_compile=True,||' ${S}/utils/codegen/ipc/mojo/public/tools/mojom/mojom/generate/template_expander.py
}

do_install_append() {
    chrpath -d ${D}${libdir}/libcamera.so
}

addtask do_recalculate_ipa_signatures_package after do_package before do_packagedata
do_recalculate_ipa_signatures_package() {
    local modules
    for module in $(find ${PKGD}/usr/lib/libcamera -name "*.so.sign"); do
        module="${module%.sign}"
        if [ -f "${module}" ] ; then
            modules="${modules} ${module}"
        fi
    done

    ${S}/src/ipa/ipa-sign-install.sh ${B}/src/ipa-priv-key.pem "${modules}"
}

# Fix for error:
# ../git/meson.build:247:7: ERROR: <ExternalProgram 'python3' is not a valid python or it is missing setuptools
unset _PYTHON_SYSCONFIGDATA_NAME

FILES_${PN}-gst = "${libdir}/gstreamer-1.0"
