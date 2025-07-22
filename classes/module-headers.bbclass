# Helper class that only copies module header without compiling.
# Provides a 'native' virtual recipe.
#
# Recipes should not DEPEND directly on a module if they are not a module/kernel
# themselves. They would otherwise depend on the kernel checksum.
# Also, if a recipe RRECOMMENDS on any 'kernel-module', the build dependency
# to any recipe that 'inherit module' would be removed and then not correctly
# rebuild on module change.
# https://github.com/yoctoproject/poky/blob/dunfell/meta/lib/oe/sstatesig.py#L57-L66
#
# NOTE: Recipes inheriting this class don't provide packages and cannot be used
# to install header files to the rootfs.
#
# Required configuration
#
# do_install: Function installing headers.
#
# Example:
#
#     inherit module-headers
#
#     do_install() {
#        install -d ${D}/${includedir}/linux/
#        install -m 644 ${S}/include/uapi/linux/module_name.h ${D}/${includedir}/linux/
#     }

inherit nopackages

BBCLASSEXTEND += "native"
EXCLUDE_FROM_WORLD = "1"

# Ignore base tasks.
do_configure[noexec] = "1"
do_compile[noexec] = "1"

INHIBIT_DEFAULT_DEPS = "1"
