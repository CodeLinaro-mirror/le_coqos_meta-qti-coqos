SUMMARY = "Vulkan Header files and API registry"
DESCRIPTION = "Vulkan is a 3D graphics and compute API providing cross-platform access \
to modern GPUs with low overhead and targeting realtime graphics applications such as \
games and interactive media. This package contains the development headers \
for packages wanting to make use of Vulkan."
HOMEPAGE = "https://www.khronos.org/vulkan/"
BUGTRACKER = "https://github.com/KhronosGroup/Vulkan-Headers"
SECTION = "libs"

LICENSE = "Apache-2.0 & MIT"
LIC_FILES_CHKSUM = "file://LICENSE.md;md5=1bc355d8c4196f774c8b87ed1a8dd625"

SRC_URI = "git://github.com/KhronosGroup/Vulkan-Headers.git;branch=vulkan-sdk-1.3.268;protocol=https"
SRCREV = "7b3466a1f47a9251ac1113efbe022ff016e2f95b"

S = "${WORKDIR}/git"

do_install_append() {
    # install a userspace header
    install -d ${D}/${includedir}/vk_video
    install -m 644 ${S}/include/vk_video/vulkan_video_codec_h264std_decode.h ${D}/${includedir}/vk_video/
    install -m 644 ${S}/include/vk_video/vulkan_video_codec_h264std_encode.h ${D}/${includedir}/vk_video/
    install -m 644 ${S}/include/vk_video/vulkan_video_codec_h264std.h ${D}/${includedir}/vk_video/
    install -m 644 ${S}/include/vk_video/vulkan_video_codec_h265std_decode.h ${D}/${includedir}/vk_video/
    install -m 644 ${S}/include/vk_video/vulkan_video_codec_h265std_encode.h ${D}/${includedir}/vk_video/
    install -m 644 ${S}/include/vk_video/vulkan_video_codec_h265std.h ${D}/${includedir}/vk_video/
    install -m 644 ${S}/include/vk_video/vulkan_video_codecs_common.h ${D}/${includedir}/vk_video/
}

FILES_${PN} += "${includedir}/vk_video"

UPSTREAM_CHECK_GITTAGREGEX = "sdk-(?P<pver>\d+(\.\d+)+)"


