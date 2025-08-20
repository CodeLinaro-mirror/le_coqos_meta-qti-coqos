# Prefer systemd way of creating getty@.service symlinks using
# systemd-getty-generator (instead of the Yocto default systemd-serialgetty that
# creates everything in do_install).
# Baremetal platforms and COQOSHV might have different serial console devices,
# such as ttyS0, ttySC0, ttyAMA0, etc. Getty generator allows to not bother with
# static getty configuration.

PACKAGECONFIG_append = " serial-getty-generator"
