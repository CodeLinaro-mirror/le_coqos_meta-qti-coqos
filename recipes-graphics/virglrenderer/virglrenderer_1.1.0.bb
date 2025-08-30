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

# Upstream repository; commit which carries the 1.1.0 tag
SRCREV = "1aeaf5e10a9c89096e96d09599aa419d5c50712f"
SRC_URI = " \
    git://git@gitlab.freedesktop.org/virgl/virglrenderer.git;protocol=ssh;nobranch=1;rebaseable=1 \
    file://0001-include-missing-glgeterror-in-format-check.patch \
    file://0002-vrend-define-missing-gbm-formats.patch \
    file://0003-workaround-for-ucompare-shader-compiler-bug.patch \
    file://0004-workaround-disable-dual-src-blend.patch \
    file://0005-vrend_renderer-Emulate-OpenGL-Transform-Feedback-adv.patch \
    file://0006-Revert-523796808ae9de6f8fc778a2f5599a8314989189-vren.patch \
    file://0007-shader-Avoid-requiring-GL_EXT_texture_shadow_lod-if-.patch \
    file://0008-vrend-renderer-disable-feat_egl_image_storage.patch \
    file://0009-Unconditionally-use-glClearBufferXXX-instead-of-glCl-1.patch \
    file://0012-virgl-Implement-API-to-attach-dmabuf-backing-for-cla.patch \
    file://0013-Add-rgba-resources-backed-by-EGL-image-as-not-suppor.patch \
"

S = "${WORKDIR}/git"

inherit meson pkgconfig features_check

DEPENDS_remove = " meson-native"
DEPENDS_append = " meson0.57.1-native"

BBCLASSEXTEND = "native nativesdk"

REQUIRED_DISTRO_FEATURES = "opengl"
REQUIRED_DISTRO_FEATURES_class-native = ""
REQUIRED_DISTRO_FEATURES_class-nativesdk = ""

EXTRA_OEMESON:append = " \
            -Dplatforms=egl \
            "
