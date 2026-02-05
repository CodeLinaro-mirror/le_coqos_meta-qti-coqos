# We don't use mesa for vulkan loader, don't recommend it.
SRC_URI = "git://github.com/KhronosGroup/Vulkan-Loader.git;branch=main;protocol=https"
SRCREV = "62fd1a35f841e623dccb7262bb8e9ad8544d005a"

PV = "1.3.240"

RRECOMMENDS:${PN} = ""
