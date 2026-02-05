# We don't use mesa for vulkan loader, don't recommend it.
SRC_URI = "git://github.com/KhronosGroup/Vulkan-Tools.git;branch=main;protocol=https"
SRCREV = "8ad79c18ffb12cf903cad96a196c33d335f0cdb4"

PV = "1.3.240"

inherit cmake features_check pkgconfig
