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
SRC_URI += "file://0007-qcom_scm-Add-support-for-QCPE-3.0.0-on-HOS.patch"
SRC_URI += "file://0008-qcom_scm-coqoshv-Implement-COQOSHV-QCPE-system-liste.patch"
SRC_URI += "file://0009-Add-kernel-patch-to-measure-blocking-time-for-atomic.patch"
SRC_URI += "file://0010-boot_stats-Add-State-Manager-KPI-markers.patch"
SRC_URI += "file://0011-tz_log-Register-hyp-related-nodes-only-if-related-co.patch"

# v4l2loopback support for libcamera
SRC_URI += "file://0001-msm-ais-v4l2_v2-Add-media-subsystem-support.patch;patchdir=techpack/ais"
SRC_URI += "file://0002-msm-ais-v4l2_v2-Fix-v4l2-capabilities-query-ioctl.patch;patchdir=techpack/ais"
SRC_URI += "file://0003-msm-ais-v4l2_v2-Send-AIS_V4L2_OPEN_INPUT-command-onl.patch;patchdir=techpack/ais"
SRC_URI += "file://0004-msm-ais-v4l2_v2-Introduce-dmabuf-import-support.patch;patchdir=techpack/ais"

#--------------------------------------------------------------------------------------------
# Put kernel config fragments here:
#--------------------------------------------------------------------------------------------
SRC_URI += "file://disable_kernel_unmap.cfg"
SRC_URI += "file://disable_module_sig.cfg"
SRC_URI += "file://enable_coresight-tmc.cfg"
SRC_URI += "file://enable_qcom_scm_qcpe_hos.cfg"
SRC_URI += "file://enable_qti_tz_log.cfg"
SRC_URI += "file://enable_st_asm330lhh_modules.cfg"
SRC_URI += "file://enable_system_listener_qcpe_hos.cfg"
SRC_URI += "file://enable_vivid.cfg"

#--------------------------------------------------------------------------------------------
# Below are workarounds needed for out-of-tree patches for V4L2loopback to be applied
#--------------------------------------------------------------------------------------------

SRC_DIR = "${SRC_DIR_ROOT}/kernel/msm-5.4"

# Dirty hack to make patching of "techpack" stuff work.
# This reinforces the fix for broken .git symlinks present in COQOS SDK layers.
do_fixup_dotgit_repo_n() {
    if [ -L "${S}/${1}/.git" ]; then
        if [ ! -e "${S}/${1}/.git" ]; then
           echo "Symlink ~.git is not valid!. Creating a .git repo copy."
           rm "${S}/${1}/.git"
           cp -r -L "${SRC_DIR}/${1}/.git" "${S}/${1}/.git"
        fi
    fi
}

do_fixup_dotgit_repo() {
    if [ -L "${S}/.git" ]; then
        if [ ! -e "${S}/.git" ]; then
            echo "Symlink ~.git is not valid!. Creating a .git repo copy."
            rm "${S}/.git"
            cp -r -L "${SRC_DIR}/.git" "${S}/.git"
        fi
    fi

    do_fixup_dotgit_repo_n techpack/display
    do_fixup_dotgit_repo_n techpack/ais
    do_fixup_dotgit_repo_n techpack/video
}

addtask do_fixup_dotgit_repo after do_symlink_kernsrc before do_validate_branches

# find_patches override for kernel-yocto.bbclass to ignore patches with patchdir=techpack completely.
# They will be handled separately
def find_patches(d,subdir):
    patches = src_patches(d)
    patch_list=[]
    for p in patches:
        _, _, local, _, _, parm = bb.fetch.decodeurl(p)
        patchdir = ''
        if "patchdir" in parm:
            patchdir = parm["patchdir"]
        if not patchdir.startswith("techpack"):
            if subdir:
                if subdir == patchdir:
                    patch_list.append(local)
            else:
                patch_list.append(local)

    return patch_list

def find_techpack_patches(d):
    patches = src_patches(d)
    patch_list=[]
    for p in patches:
        _, _, local, _, _, parm = bb.fetch.decodeurl(p)
        patchdir = ''
        if "patchdir" in parm:
            patchdir = parm["patchdir"]
        if patchdir.startswith("techpack"):
            patch_list.append(local)

    return patch_list

# Another hack to be able to apply patches on "techpack" repo.
# The right solution would be to teach kernel-yocto.bbclass to work
# correctly with multiple git repos in SRC_URI.
do_patch_append() {
    cd ${S}/techpack/ais/
    patches="${@" ".join(find_techpack_patches(d))}"
    for s in ${patches}; do
        git am -s $s
    done
}
