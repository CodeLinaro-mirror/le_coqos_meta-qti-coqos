ARTIFACT_ID = "gcc-linaro-aarch64-none-elf"

TOOLCHAIN_LIBC = "newlib"
TOOLCHAIN_TARGET = "aarch64"
TOOLCHAIN_PLATFORM = "none"
TOOLCHAIN_ABI = "elf"
TOOLCHAIN_SRCDIR = "*aarch64*-elf*"
TOOLCHAIN_DESTDIR = "${ARTIFACT_ID}-${PV}"

GROUP_PV = "7.3-2018.05"

require gcc-linaro-prebuilt.inc
