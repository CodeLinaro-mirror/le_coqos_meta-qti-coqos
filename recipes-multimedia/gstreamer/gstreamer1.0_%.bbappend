FILESEXTRAPATHS_append := ":${THISDIR}/${PN}"

SRC_URI_append = " file://0001-allocator-Avoid-integer-overflow-when-allocating-sys.patch"
SRC_URI_append = " file://0001-capsfilter-Return-correct-error-when-flushing-during.patch"
