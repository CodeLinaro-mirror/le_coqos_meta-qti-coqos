SUMMARY = "Clang compiler"

DESCRIPTION = "Prebuilt Clang cross-toolchain for Linux x86 host machines."

HOMEPAGE = "https://git.codelinaro.org"

LICENSE = "Apache-2.0-with-LLVM-exception"

LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/Apache-2.0-with-LLVM-exception;md5=0bcd48c3bdfef0c9d9fd17726e4b7dab"

inherit native

PV = "14.0.7"

CLANG_REPO = "linux-x86"
CLANG_DIR = "clang-r450784e"
CLANG_TAG = "ks-kernel.lnx.2.0.r31-rel"
CLANG_DOWNLOAD_PATH = "${CLANG_REPO}-${CLANG_TAG}.tar.bz2?path=${CLANG_DIR}"

SRC_URI = "https://git.codelinaro.org/clo/la/kernelplatform/prebuilts-master/clang/host/linux-x86/-/archive/${CLANG_TAG}/${CLANG_DOWNLOAD_PATH};name=clang"

SRC_URI[clang.sha256sum] = "47c0a2a38a8185d88899abaff21987458b8e8ac69a0128fa1dc5b7c97e207b65"

S = "${WORKDIR}"

do_install () {
    install -d ${D}/${STAGING_DIR_NATIVE}/usr/share/${PN}
    tar xpf ${WORKDIR}/${CLANG_DOWNLOAD_PATH} -C ${D}/${STAGING_DIR_NATIVE}/usr/share/${PN} --strip-components=2
}

# Do not create packages(*.ipk or *.rpm) for this recipe
PACKAGES = ""
inherit nopackages

# Do not strip binaries
INHIBIT_PACKAGE_DEBUG_SPLIT = "1"
INHIBIT_PACKAGE_STRIP = "1"
INHIBIT_SYSROOT_STRIP = "1"

# Skip the unwanted steps
do_patch[noexec] = "1"
do_compile[noexec] = "1"
do_configure[noexec] = "1"
