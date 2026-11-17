TARGET ?= linux
ROOT_DIR := $(CURDIR)/..
BUILD_DIR := $(ROOT_DIR)/build
RDK_HAL_DIR := $(ROOT_DIR)/rdk-halif-aidl
HALIF_LIB_DIR := $(RDK_HAL_DIR)/out/target/lib/rdk-halif-aidl
HALIF_INCLUDE_DIR := $(RDK_HAL_DIR)/out/build/include
PANEL_HEADERS := $(HALIF_INCLUDE_DIR)/panel/current/include
VIDEODECODER_HEADERS := $(HALIF_INCLUDE_DIR)/videodecoder/current/include
COMMON_HEADERS := $(HALIF_INCLUDE_DIR)/common/current/include

MOCK_SERVICE_BIN := $(BUILD_DIR)/bin/MockPanelService
SOURCES := src/PanelOutputService.cpp src/PanelOutput.cpp
OBJECTS := $(SOURCES:.cpp=.o)
CXXFLAGS := -Wall -Wextra -Werror -std=c++17 -ggdb \
	-isystem $(PANEL_HEADERS) -isystem $(VIDEODECODER_HEADERS) -isystem $(COMMON_HEADERS) \
	-isystem $(BUILD_DIR)/usr/include
LDFLAGS := -L$(HALIF_LIB_DIR) -Wl,-rpath-link=$(HALIF_LIB_DIR) \
	-L$(BUILD_DIR)/usr/lib -Wl,-rpath-link=$(BUILD_DIR)/usr/lib \
	-Wl,--copy-dt-needed-entries -Wl,-rpath,'$$ORIGIN' -Wl,--disable-new-dtags
LDLIBS := -lbinder -lutils -lpanel-cpp_vts -lvideodecoder-cpp_vts -lcommon-cpp_vts -lpthread

.PHONY: all clean check-headers check-libs
all: $(MOCK_SERVICE_BIN)

check-headers:
	@test -d "$(PANEL_HEADERS)" || { echo "ERROR: Panel headers are missing; run ./build.sh TARGET=$(TARGET) first"; exit 1; }
	@test -d "$(VIDEODECODER_HEADERS)" || { echo "ERROR: VideoDecoder headers are missing; run ./build.sh TARGET=$(TARGET) first"; exit 1; }
	@test -d "$(COMMON_HEADERS)" || { echo "ERROR: Common headers are missing; run ./build.sh TARGET=$(TARGET) first"; exit 1; }

check-libs:
	@test -e "$(HALIF_LIB_DIR)/libpanel-cpp_vts.so" || { echo "ERROR: Panel library is missing; run ./build.sh TARGET=$(TARGET) first"; exit 1; }
	@test -e "$(BUILD_DIR)/usr/lib/libbinder.so" || { echo "ERROR: Binder SDK is missing; run ./build.sh TARGET=$(TARGET) first"; exit 1; }

$(MOCK_SERVICE_BIN): check-headers check-libs $(OBJECTS)
	@mkdir -p $(dir $@)
	$(CXX) $(LDFLAGS) $(OBJECTS) -o $@ $(LDLIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -fPIC -c $< -o $@

clean:
	rm -f $(OBJECTS) $(MOCK_SERVICE_BIN)
