PROJECT = cgol
SRC = src/main.c src/cda.c src/utils.c
OBJ = $(SRC:src/%.c=$(BUILD_DIR)/%.o)

BUILD_DIR = target
DEPS_DIR = deps
EXECUTABLE = $(BUILD_DIR)/$(PROJECT)
RAYLIB_INSTALL = $(DEPS_DIR)/raylib_install
RAYLIB_STATIC_LIB = $(RAYLIB_INSTALL)/lib/libraylib.a

RAYLIB_TAG = 5.5
RAYLIB_LINUX_PREBUILT = github.com/raysan5/raylib/releases/download/$(RAYLIB_TAG)/raylib-$(RAYLIB_TAG)_linux_amd64.tar.gz

CC ?= gcc
CFLAGS ?= -O2 -Wall -I$(RAYLIB_INSTALL)/include
LDFLAGS ?= -L$(RAYLIB_INSTALL)/lib
LDLIBS ?= -l:libraylib.a -lm

.PHONY: all clean distclean fetch-raylib deps-check

all: deps-check $(BUILD_DIR) $(EXECUTABLE)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(EXECUTABLE): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS) $(LDLIBS)

$(BUILD_DIR)/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

deps-check:
ifeq ($(wildcard $(RAYLIB_STATIC_LIB)),)
	$(MAKE) fetch-raylib
endif

fetch-raylib:
	mkdir -p $(DEPS_DIR)
	@echo "Downloading prebuilt raylib $(RAYLIB_TAG) for Linux x86_64..."
	if [ ! -f "$(DEPS_DIR)/raylib-$(RAYLIB_TAG)_linux_amd64.tar.gz" ]; then \
	  curl -Ls -o $(DEPS_DIR)/raylib-$(RAYLIB_TAG)_linux_amd64.tar.gz $(RAYLIB_LINUX_PREBUILT); \
	fi
	@echo "Extracting to $(RAYLIB_INSTALL)..."
	mkdir -p $(RAYLIB_INSTALL)
	tar -xzf $(DEPS_DIR)/raylib-$(RAYLIB_TAG)_linux_amd64.tar.gz -C $(RAYLIB_INSTALL) --strip-components=1

clean:
	rm -rf $(BUILD_DIR) $(PROJECT)

distclean: clean
	rm -rf $(DEPS_DIR)
