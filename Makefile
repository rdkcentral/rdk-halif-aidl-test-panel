TARGET ?= linux
VARIANT := CPP

ROOT_DIR := $(CURDIR)
PANEL_TEST_DIR := $(ROOT_DIR)/src
BIN_DIR := $(ROOT_DIR)/build/bin
LIB_DIR := $(ROOT_DIR)/build/$(TARGET)/lib
BUILD_DIR := $(ROOT_DIR)/build/$(TARGET)/obj
TARGET_EXEC := vts_panel_hal_test

RDK_HALIF_AIDL_VERSION ?= main
PANEL_VERSION := $(shell git describe --tags --always 2>/dev/null || echo "unknown")
BUILD_TIMESTAMP := $(shell date +"%Y-%m-%d_%H:%M:%S")
CXXFLAGS := -std=c++17 -g3 -O0 -Wall -Wformat -Werror=format \
	-DRDK_HALIF_AIDL_VERSION=\"$(RDK_HALIF_AIDL_VERSION)\" \
	-DPANEL_VERSION=\"$(PANEL_VERSION)\" -DBUILD_TIMESTAMP=\"$(BUILD_TIMESTAMP)\"

SRC_DIRS := $(PANEL_TEST_DIR) $(ROOT_DIR)/test
HALIF_OUT_DIR := $(ROOT_DIR)/rdk-halif-aidl/out
HALIF_LIB_DIR := $(HALIF_OUT_DIR)/target/lib/rdk-halif-aidl
-include $(HALIF_OUT_DIR)/halif_generated.mk
INC_DIRS := $(SRC_DIRS)
CXXFLAGS += $(addprefix -isystem ,$(HALIF_INC_DIRS)) \
	-isystem $(ROOT_DIR)/build/usr/include
XLDFLAGS := -rdynamic -L$(ROOT_DIR)/build/usr/lib -Wl,-rpath-link=$(ROOT_DIR)/build/usr/lib \
	-L$(HALIF_LIB_DIR) -Wl,-rpath-link=$(HALIF_LIB_DIR) \
	-Wl,-rpath,'$$ORIGIN' -Wl,--disable-new-dtags \
	$(HALIF_MODULE_LIBS) -lbinder -lutils

UT_CORE_DIR := $(ROOT_DIR)/ut-core
export BIN_DIR LIB_DIR BUILD_DIR SRC_DIRS TARGET_EXEC INC_DIRS XLDFLAGS CXXFLAGS

.PHONY: all clean mockservice
all: $(TARGET_EXEC) mockservice
$(TARGET_EXEC):
	$(MAKE) -C $(UT_CORE_DIR) -j $(shell nproc) TARGET=$(TARGET) VARIANT=$(VARIANT)
mockservice:
	$(MAKE) -C $(ROOT_DIR)/mockservice TARGET=$(TARGET)
clean:
	$(MAKE) -C $(UT_CORE_DIR) -j TARGET=$(TARGET) VARIANT=$(VARIANT) clean
	$(MAKE) -C $(ROOT_DIR)/mockservice clean
	rm -rf $(PANEL_TEST_DIR)/*.o $(ROOT_DIR)/test/*.o
