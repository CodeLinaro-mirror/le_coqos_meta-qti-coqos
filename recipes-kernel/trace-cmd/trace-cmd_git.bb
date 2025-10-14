SUMMARY = "User interface to Ftrace"
HOMEPAGE = "http://git.kernel.org/"

LICENSE = "GPLv2 & LGPLv2.1"
LIC_FILES_CHKSUM = " \
    file://COPYING;md5=873f48a813bded3de6ebc54e6880c4ac\
    file://COPYING.LIB;md5=edb195fe538e4552c1f6ca0fd7bf4f0a\
    file://tracecmd/trace-cmd.c;beginline=3;endline=3;md5=aeb8e904cabe48223d58bc309297aa3c \
    file://lib/trace-cmd/trace-input.c;beginline=3;endline=3;md5=0049fd9293753da7669278d5a3e4cb2b \
"

require trace-cmd.inc

PACKAGECONFIG ??= ""
PACKAGECONFIG[audit] = ',NO_AUDIT=1,audit'
EXTRA_OEMAKE = "\
    'prefix=${prefix}' \
    'bindir=${bindir}' \
    'man_dir=${mandir}' \
    'html_install=${datadir}/kernelshark/html' \
    'img_install=${datadir}/kernelshark/html/images' \
    \
    'bindir_relative=${@oe.path.relative(prefix, bindir)}' \
    'libdir=${libdir}' \
    \
    NO_PYTHON=1 \
    ${PACKAGECONFIG_CONFARGS} \
"

do_compile_prepend() {
    # Make sure the recompile is OK
    rm -f ${B}/.*.d
}

do_install() {
        oe_runmake DESTDIR="${D}" install
}

do_patch[noexec] = "1"
