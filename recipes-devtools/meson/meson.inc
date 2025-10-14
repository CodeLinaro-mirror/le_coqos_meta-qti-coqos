HOMEPAGE = "http://mesonbuild.com"
SUMMARY = "A high performance build system"
DESCRIPTION = "Meson is a build system designed to increase programmer \
productivity. It does this by providing a fast, simple and easy to use \
interface for modern software development tools and practices."

LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://COPYING;md5=3b83ef96387f14655fc854ddc3c6bd57"

SRC_URI = "https://github.com/mesonbuild/meson/releases/download/0.63.3/meson-0.63.3.tar.gz \
           file://0001-python-module-do-not-manipulate-the-environment-when.patch \
           file://0001-is_debianlike-always-return-False.patch \
           file://0001-Check-for-clang-before-guessing-gcc-or-lcc.patch \
           "
SRC_URI[sha256sum] = "519c0932e1a8b208741f0fdce90aa5c0b528dd297cf337009bf63539846ac056"

S = "${WORKDIR}/meson-0.63.3"

SRC_URI_append_class-native = " \
    file://0001-Make-CPU-family-warnings-fatal.patch \
    file://0002-Support-building-allarch-recipes-again.patch \
"

UPSTREAM_CHECK_URI = "https://github.com/mesonbuild/meson/releases"
UPSTREAM_CHECK_REGEX = "meson-(?P<pver>\d+(\.\d+)+)\.tar"

inherit setuptools3

RDEPENDS_${PN} = "ninja python3-modules python3-pkg-resources"

FILES_${PN} += "${datadir}/polkit-1"
