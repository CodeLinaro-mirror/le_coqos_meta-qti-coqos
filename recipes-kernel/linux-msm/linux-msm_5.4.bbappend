FILESEXTRAPATHS:append := ":${THISDIR}/files"

#--------------------------------------------------------------------------------------------
# Put kernel patches here:
#--------------------------------------------------------------------------------------------
SRC_URI += "file://0001-clk-Introduce-get-clock-by-unique-name-API-clk_get_b.patch"
SRC_URI += "file://0002-msm_qmp.c-Support-a-pre-established-link.patch"
SRC_URI += "file://0003-kernel-sched-walt-walt.c-Fix-processing-CPU-cluster-.patch"
SRC_URI += "file://0004-pinctrl-msm-Don-t-call-irq_chip_eoi_parent-if-interr.patch"
SRC_URI += "file://0005-pinctrl-sm8150-Remove-GPIO-124-from-PDC-wakeirq-map.patch"
SRC_URI += "file://0006-st_asm330lhh-calculate-actual-odr-value-using-INTERN.patch"
