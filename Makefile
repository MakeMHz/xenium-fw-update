XBE_TITLE = Xenium\ Firmware\ Update
GEN_XISO = $(XBE_TITLE).iso
NXDK_SDL = y
NXDK_CXX = y
NXDK_NET = y

SRCS += \
	$(wildcard $(CURDIR)/src/*.cpp)
# Assets
SRCS += \
	$(wildcard $(CURDIR)/assets/*.cpp)
CFLAGS += \
	-I$(CURDIR)/include \
	-I$(CURDIR)/src \
	-O2 \
	-D_XBOX
CXXFLAGS += \
	-I$(CURDIR)/include \
	-I$(CURDIR)/src \
	-O2 \
	-D_XBOX

include $(NXDK_DIR)/Makefile
