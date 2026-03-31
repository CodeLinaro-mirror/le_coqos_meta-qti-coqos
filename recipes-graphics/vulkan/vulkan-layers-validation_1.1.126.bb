SUMMARY = "Vulkan Ecosystem Components - Validation Layers"
DESCRIPTION = "Vulkan is an Explicit API, enabling direct control over \
               how GPUs actually work. By design, minimal error checking \
               is done inside a Vulkan driver. Applications have full \
               control and responsibility for correct operation. Any \
               errors in how Vulkan is used can result in a crash. \
               This project provides Vulkan validation layers that can be \
               enabled to assist development by enabling developers to \
               verify their applications correct use of the Vulkan API"
SECTION = "graphics"
HOMEPAGE = "https://www.khronos.org/vulkan"
DEPENDS = "bison-native glslang glslang-native spirv-tools vulkan-loader"

inherit cmake python3native

REQUIRED_DISTRO_FEATURES = "wayland"

LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://LICENSE.txt;md5=7dbefed23242760aa3475ee42801c5ac"

S = "${WORKDIR}/git"

SRCREV = "5db6e0af00441eec792a2f87ca8626a29d17b8b4"
SRC_URI = "git://github.com/KhronosGroup/Vulkan-ValidationLayers.git;branch=sdk-${PV}"

SRC_URI[sha256sum] = "e1cfbfbf5f7b80419ed026b2e7c8e55c8967bb118dab05e0f5c7c755b7d86f94"

EXTRA_OECMAKE = " \
    -DGLSLANG_INSTALL_DIR=${STAGING_DIR_HOST}/usr \
    -DGLSLANG_SPIRV_INCLUDE_DIR=${STAGING_DIR_HOST}/usr/include/glslang/ \
    -DCUSTOM_SPIRV_TOOLS_BIN_ROOT=1 \
    -DSPIRV_TOOLS_BINARY_ROOT=${STAGING_DIR_HOST}/usr \
    -DBUILD_TESTS=0 \
    -DBUILD_WSI_MIR_SUPPORT=0 \
    -DBUILD_WSI_WAYLAND_SUPPORT=1 \
    -DBUILD_WSI_XCB_SUPPORT=0 \
    -DBUILD_WSI_XLIB_SUPPORT=0 \
    -DBUILD_DEMOS=0 \
"

INHIBIT_PACKAGE_DEBUG_SPLIT = '1'
INHIBIT_PACKAGE_STRIP = '1'

PACKAGES = "${PN}"
FILES_${PN} += "${datadir} \
               ${libdir}"

do_install_append() {
     rm -rf ${D}${libdir}/libVkLayer_utils.a
}
