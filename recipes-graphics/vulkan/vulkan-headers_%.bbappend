SRC_URI = "git://github.com/KhronosGroup/Vulkan-Headers.git;branch=main;protocol=https"

SRCREV = "9f93cbe76abe9f6cb4a36df10b08fa3b78ae0027"
RDEPENDS:${PN} += "python3-core"

PV = "1.3.240"
