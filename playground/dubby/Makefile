# Project Name
TARGET = DubbyPlayground

# Sources
CPP_SOURCES = / DubbyPlayground.cpp / ../../web-compiler/build_template/lib/DaisyDub/DspNode.cpp / ../../web-compiler/build_template/lib/DaisyDub/Dubby.cpp

# Library Locations
LIBDAISY_DIR = ../../web-compiler/build_template/lib/libDaisy
DAISYSP_DIR = ../../web-compiler/build_template/lib/DaisySP

# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile

# Print floats in PrintLine()
LDFLAGS += -u _printf_float