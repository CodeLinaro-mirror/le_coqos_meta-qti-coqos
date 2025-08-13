FILESEXTRAPATHS:append := ":${THISDIR}/files"

#--------------------------------------------------------------------------------------------
# Put kernel patches here:
#--------------------------------------------------------------------------------------------
SRC_URI += "file://0001-clk-Introduce-get-clock-by-unique-name-API-clk_get_b.patch"
