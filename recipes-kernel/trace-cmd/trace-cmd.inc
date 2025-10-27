SRCREV = "3852bbaa9c130cf20389688df6d000ed35a3eb1d"
PV = "2.9"

inherit pkgconfig

FILESEXTRAPATHS =. "${FILE_DIRNAME}/trace-cmd:"

SRC_URI = "git://git.kernel.org/pub/scm/linux/kernel/git/rostedt/trace-cmd.git \
"

S = "${WORKDIR}/git"
