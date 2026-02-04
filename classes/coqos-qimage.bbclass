inherit qimage

do_make_avb_image:append(){
    if ${@bb.utils.contains('DISTRO_FEATURES', 'qti-avb', 'true', 'false', d)}; then
        avbtool make_vbmeta_image \
            --include_descriptors_from_image ${DEPLOY_DIR_IMAGE}/image-qti-sa8155-coqos-linux-android.fastboot \
            --include_descriptors_from_image ${DEPLOY_DIR_IMAGE}/${PRODUCT}-dtbo.img \
            --include_descriptors_from_image ${DEPLOY_DIR_IMAGE}/${IMAGE_LINK_NAME}.ext4 \
            --setup_rootfs_from_kernel ${DEPLOY_DIR_IMAGE}/${IMAGE_LINK_NAME}.ext4 \
            --algorithm SHA256_RSA4096 \
            --key ${STAGING_DIR_NATIVE}${sysconfdir}/signing_tools/sigkeys/testkey_rsa4096.pem \
            --rollback_index 0 \
            --output ${DEPLOY_DIR_IMAGE}/${VBMETAIMAGE_TARGET}
    fi
}
