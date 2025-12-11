SUMMARY = "VirGL virtual OpenGL renderer"
DESCRIPTION = "Virgil is a research project to investigate the possibility of \
creating a virtual 3D GPU for use inside qemu virtual machines, that allows \
the guest operating system to use the capabilities of the host GPU to \
accelerate 3D rendering."
HOMEPAGE = "https://virgil3d.github.io/"

LICENSE = "MIT"
LIC_FILES_CHKSUM = " \
    file://COPYING;md5=c81c08eeefd9418fca8f88309a76db10 \
"

DEPENDS = "libdrm virtual/libgles2 virtual/libgbm libepoxy gbm gbm-headers glib-2.0 wayland"
DEPENDS_remove_class-native = "virtual/libgles2"
DEPENDS_append_class-native = " virtual/libgl"

SRCREV = "${AUTOREV}"
SRC_URI = " \
    ${PATH_TO_REPO}/external/virglrenderer/.git;protocol=${PROTO};destsuffix=external/virglrenderer \
"

S = "${WORKDIR}/external/virglrenderer"

inherit meson pkgconfig features_check

DEPENDS_remove = " meson-native"
DEPENDS_append = " meson0.63.3-native"

BBCLASSEXTEND = "native nativesdk"

REQUIRED_DISTRO_FEATURES = "opengl"
REQUIRED_DISTRO_FEATURES_class-native = ""
REQUIRED_DISTRO_FEATURES_class-nativesdk = ""

EXTRA_OEMESON:append = " \
            -Dplatforms=egl \
            "
