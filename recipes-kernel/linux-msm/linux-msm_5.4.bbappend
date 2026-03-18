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
SRC_URI += "file://0012-qcom_scm-coqoshv-Initialize-SCM-before-entering-atom.patch"
SRC_URI += "file://0001-msm-geni-se-Add-state-checks-before-clock-operations.patch"

# v4l2loopback support for libcamera
SRC_URI += "file://0001-msm-ais-v4l2_v2-Add-media-subsystem-support.patch;patchdir=techpack/ais"
SRC_URI += "file://0002-msm-ais-v4l2_v2-Fix-v4l2-capabilities-query-ioctl.patch;patchdir=techpack/ais"
SRC_URI += "file://0003-msm-ais-v4l2_v2-Send-AIS_V4L2_OPEN_INPUT-command-onl.patch;patchdir=techpack/ais"
SRC_URI += "file://0004-msm-ais-v4l2_v2-Introduce-dmabuf-import-support.patch;patchdir=techpack/ais"

# set appropriate log level for pci msm driver
SRC_URI += "file://0001-pci-msm-replace-usage-of-pr_alert-macro-with-pr_info.patch"

#--------------------------------------------------------------------------------------------
# Put kernel config fragments here:
#--------------------------------------------------------------------------------------------
SRC_URI += "file://disable_hibernation.cfg"
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
SRC_URI += "file://disable_msm_console.cfg"
SRC_URI += "file://enable_gpio_mockup.cfg"

#--------------------------------------------------------------------------------------------
# Workarounds for applying out-of-tree patches on repositories mapped by repo manifest
#--------------------------------------------------------------------------------------------

do_fixup_repo_copy() {
    local src_dir_path="${1}"
    local dest_dir_path="${2}"

    if [ -L "${dest_dir_path}" ]; then
        # Follow the symlink
        dest_dir_path=$(readlink -f "${dest_dir_path}")
    fi

    if [ -d "${src_dir_path}" ]; then
        # Remove any existing repo
        rm -rf "${dest_dir_path}"
        # Re-create the repository from manifest checkout
        # Use --no-local to allow cloning a local repository with symlinks
        if ! git clone --depth 1 --no-local "${src_dir_path}" "${dest_dir_path}"; then
            bb.error "Error cloning repository from ${src_dir_path} to ${dest_dir_path}"
        fi
    else
        bb.error "Error: ${src_dir_path} is not a directory"
    fi
}

do_fixup_repos() {
    # recreate kernel repo itself
    do_fixup_repo_copy "${SRC_DIR_ROOT}/kernel/msm-5.4" "${S}"

    # recreate the in-kernel nested techpack repos
    for child_dir in $(ls -d "${SRC_DIR_ROOT}/kernel/msm-5.4/techpack"/*); do
        if [ -d "${child_dir}" ] && [ -d "${child_dir}/.git" ]; then
            child_dir_name=$(basename "${child_dir}")
            do_fixup_repo_copy "${SRC_DIR_ROOT}/kernel/msm-5.4/techpack/${child_dir_name}" "${S}/techpack/${child_dir_name}"
        fi
    done
}

DTS_SRC = "${WORKDIR}/vendor/qcom/proprietary/devicetree"
DTS_LINK = "${S}/arch/${ARCH}/boot/dts/vendor"

DTS_CAM_SRC = "${WORKDIR}/vendor/qcom/proprietary/camera-devicetree"
DTS_CAM_LINK = "${S}/arch/${ARCH}/boot/dts/vendor/qcom/camera"

DTS_DISP_SRC = "${WORKDIR}/vendor/qcom/proprietary/display-devicetree/display"
DTS_DISP_LINK = "${S}/arch/${ARCH}/boot/dts/vendor/qcom/display"

create_sym_link() {
    local src=$1
    local dest=$2
    if [ -L "${dest}" ]; then
        rm -f "${dest}"
    fi
    if [ -e "${dest}" ]; then
        rm -rf "${dest}"
    fi
    ln -s "${src}" "${dest}"
}

do_fixup_dts_symlinks() {
    # due to recreation of repos, symlinks need to be recreated as well
    create_sym_link "${DTS_SRC}" "${DTS_LINK}"
    create_sym_link "${DTS_CAM_SRC}" "${DTS_CAM_LINK}"
    create_sym_link "${DTS_DISP_SRC}" "${DTS_DISP_LINK}"
}

addtask do_fixup_repos after do_symlink_kernsrc before do_validate_branches
addtask do_fixup_dts_symlinks after do_fixup_repos before do_generate_gki_defconfig
addtask do_generate_gki_defconfig after do_fixup_dts_symlinks

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
do_patch:append() {
    cd ${S}/techpack/ais/
    patches="${@" ".join(find_techpack_patches(d))}"
    for s in ${patches}; do
        git am -s $s
    done
}
