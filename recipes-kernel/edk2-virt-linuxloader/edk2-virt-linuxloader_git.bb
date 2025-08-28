SUMMARY = "EDK2 based implementation of bootloader for Android GVM"

DESCRIPTION = "EDK2 UEFI LinuxLoader application to be run on top of Thin-UEFI \
               to boot Android GVM"

HOMEPAGE = "https://git.codelinaro.org"

LICENSE = "BSD-2-Clause & BSD-3-Clause & Apache-2.0"

LIC_FILES_CHKSUM = "\
    file://${COMMON_LICENSE_DIR}/BSD-2-Clause;md5=cb641bc04cda31daea161b1bc15da69f \
    file://${COMMON_LICENSE_DIR}/BSD-3-Clause;md5=550794465ba0ec5312d6919e203a55f9 \
"

REPO_NAME= "coqos-edk2"
SRC_URI = "${PATH_TO_REPO}/${REPO_NAME}/.git;protocol=${PROTO};destsuffix=${REPO_NAME};usehead=1"
SRCREV = "${AUTOREV}"

S = "${WORKDIR}/${REPO_NAME}"

TOOLCHAIN = "clang"
LINUX_LOADER_ARTIFACT = "LinuxLoader.efi"

EXTRA_OEMAKE = "'BOOTLOADER_OUT=${S}/out' \
                'PREBUILT_HOST_TOOLS=BUILD_CC=clang' \
                'BUILD_CXX=clang++' \
                'LDPATH=-fuse-ld=lld' \
                'BUILD_AR=llvm-ar' \
                'BOARD_BOOTLOADER_PRODUCT_NAME=gull' \
                'TARGET_ARCHITECTURE=AARCH64' \
                'VERIFIED_BOOT_LE=0' \
                'VERIFIED_BOOT_ENABLED=1' \
                'AB_RETRYCOUNT_DISABLE=0' \
                'TARGET_BOARD_TYPE_AUTO=1' \
                'USER_BUILD_VARIANT=0' \
                'DISABLE_PARALLEL_DOWNLOAD_FLASH=1' \
                'DISABLE_KERNEL_PROTOCOL=1' \
                'BUILD_USES_RECOVERY_AS_BOOT=1' \
                'BASE_ADDRESS=0x80000000' \
                'HOS_VIRT=1' \
                'HOS_VIRT_DTB_OVERLAY_ENABLED=1' \
                'CLANG_BIN=${STAGING_BINDIR_NATIVE}/' "

do_compile () {
    export CC=${BUILD_CC}
    export CXX=${BUILD_CXX}
    export LD=${BUILD_LD}
    export AR=${BUILD_AR}
    oe_runmake -f makefile all
}

do_install() {
    install -m 0644 ${S}/out/Build/DEBUG_CLANG35/AARCH64/${LINUX_LOADER_ARTIFACT} ${D}/
}

PACKAGE_ARCH = "${MACHINE_ARCH}"

FILES:${PN} += "${LINUX_LOADER_ARTIFACT}"

SYSROOT_DIRS += "/"

# Do not create packages(*.ipk or *.rpm) for this recipe
PACKAGES = ""
inherit nopackages

do_configure[noexec] = "1"
