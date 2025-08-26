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
SRC_URI += "file://enable_cpufreq_default_gov_perf.cfg"
SRC_URI += "file://enable_qcom_scm_qcpe_hos.cfg"
SRC_URI += "file://enable_qti_tz_log.cfg"
SRC_URI += "file://enable_serial-8250.cfg"
SRC_URI += "file://enable_st_asm330lhh_modules.cfg"
SRC_URI += "file://enable_system_listener_qcpe_hos.cfg"
SRC_URI += "file://enable_vhost_vsock.cfg"
SRC_URI += "file://enable_vivid.cfg"
SRC_URI += "file://set_adreno_governor_performance.cfg"

#--------------------------------------------------------------------------------------------
# Workarounds for applying out-of-tree patches on repositories mapped by repo manifest
#--------------------------------------------------------------------------------------------

SRC_DIR = "${SRC_DIR_ROOT}/kernel/msm-5.4"

do_fixup_repo_copy() {
    local dir_path="${S}/${1}"
    local src_dir_path="${SRC_DIR}/${1}"
    if [ -z "${1}" ]; then
        dir_path="${S}"
        src_dir_path="${SRC_DIR}"
    fi
    if [ -d "${dir_path}" ]; then
        broken_symlinks=$(find "${dir_path}" -type l | while read link; do
            if [ ! -e "$(readlink -f "$link")" ]; then
                echo "$link"
            fi
        done)
        if [ -n "${broken_symlinks}" ]; then
            # Remove broken repository copy
            echo "Found broken symlinks in ${dir_path}:"
            echo "${broken_symlinks}"
            rm -rf "${dir_path}"
            # Re-create the repository from manifest checkout
            # Use --no-local to allow cloning a local repository with symlinks
            git clone --no-local "${src_dir_path}" "${dir_path}"
        fi
    fi
}

do_fixup_repos() {
    do_fixup_repo_copy ""
    do_fixup_repo_copy "techpack/display"
    do_fixup_repo_copy "techpack/ais"
    do_fixup_repo_copy "techpack/video"
}

addtask do_fixup_repos after do_symlink_kernsrc before do_validate_branches

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
