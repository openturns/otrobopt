#
# Compile and create OpenTURNS installer for windows
#
# Rules dependencies make things done in the right order, however it does not
# detect if OpenTURNS sources has changed. This has to be specified manually:
# - to recompute a rules, remove the corresponding file
#   i.e. some OpenTURNS .cxx have changed:
#     rm mod-compile; make
# - to recompute every rules: make clean; make
# - to rebuild from scratch: make mrproper; make
#

###############################################################################
# adapt these following lines to your configuration :
#  OT_PREFIX  WINDEPS
# e.g. make OT_PREFIX=$PWD/ot/rc/mingw_build/install

# compiler prefix
ARCH=i686
TARGET = $(ARCH)-w64-mingw32

# OpenTurns paths
OT_PREFIX ?= $(HOME)/ot/mingw_build/install
OT_CONFIG_CMD = $(OT_PREFIX)/bin/openturns-config

OT_VERSION ?= $(shell $(OT_CONFIG_CMD) --version)

WINDEPS ?= $(OT_PREFIX)/../../distro/windows/openturns-developers-windeps

# Module paths
MOD_SRC    ?= $(PWD)/../..
MOD_BUILD  ?= $(MOD_SRC)/build-$(TARGET)
MOD_PREFIX ?= $(MOD_BUILD)/install
MODULE_VERSION = $(shell cat $(MOD_SRC)/VERSION)

# Wine
WINE_PREFIX = $(WINDEPS)/wine-$(ARCH)
export WINEPREFIX = $(WINE_PREFIX)
ifeq ($(ARCH), i686)
export WINEARCH = win32
else
export WINEARCH = win64
endif

MINGW_PREFIX = $(WINDEPS)/mingw-w64-x86_64/usr

# Python
PYBASEVER = 2.7
PYBASEVER_NODOT = $(shell echo $(PYBASEVER) | sed "s|\.||g")
PYTHON_EXECUTABLE=$(MINGW_PREFIX)/$(TARGET)/bin/python$(PYBASEVER_NODOT).exe
export PYTHONHOME := $(MINGW_PREFIX)/$(TARGET)
export PYTHONPATH := $(MINGW_PREFIX)/$(TARGET)/lib/python$(PYBASEVER_NODOT)


CMAKE=cmake

# launch checktests before creating the installer (y,n)
CHECK_OT = y

VERBOSE=ON

JOBS=$(shell getconf _NPROCESSORS_ONLN)
JOBS_PYTHON=$(JOBS)
JOBS_TEST=$(JOBS)
export PATH := $(MINGW_PREFIX)/bin:$(PATH)

DEBUG_OT=n

###############################################################################
# rules:

.PHONY: all clean-python clean mrproper


all: mod-installer


mod-check-compiler:
	@echo -n "Check OpenTURNS compilation dependencies: "
	@ls $(MOD_SRC) $(PYTHON_EXECUTABLE) > /dev/null
	@echo "ok"
	touch mod-check-compiler


mod-config: mod-check-compiler
	-mkdir -p $(MOD_BUILD)
	cp $(MINGW_PREFIX)/share/mingw/toolchain-$(TARGET).cmake $(MOD_BUILD)
	echo "set ( CMAKE_FIND_ROOT_PATH $(MINGW_PREFIX)/$(TARGET) )" >> $(MOD_BUILD)/toolchain-$(TARGET).cmake
	cp $(MINGW_PREFIX)/$(TARGET)/bin/python$(PYBASEVER_NODOT).dll $(MOD_BUILD)
	cd $(MOD_BUILD) && rm -f CMakeCache.txt && $(CMAKE) \
	  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
	  -DCMAKE_CXX_FLAGS_RELWITHDEBINFO="-O2 -g -pipe -Wall -Wp,-D_FORTIFY_SOURCE=2 -fexceptions --param=ssp-buffer-size=4" \
	  -DCMAKE_TOOLCHAIN_FILE=toolchain-$(TARGET).cmake \
	  -DCMAKE_VERBOSE_MAKEFILE=$(VERBOSE) \
	  -DOpenTURNS_DIR=$(OT_PREFIX)/lib/cmake/openturns \
	  -DPYTHON_INCLUDE_DIR=$(MINGW_PREFIX)/$(TARGET)/include/python$(PYBASEVER_NODOT) \
	  -DPYTHON_INCLUDE_DIR2=$(MINGW_PREFIX)/$(TARGET)/include/python$(PYBASEVER_NODOT) \
	  -DPYTHON_LIBRARY=$(MINGW_PREFIX)/$(TARGET)/lib/libpython$(PYBASEVER_NODOT).dll.a \
	  -DPYTHON_EXECUTABLE=$(PYTHON_EXECUTABLE) \
	  -DCMAKE_INSTALL_PREFIX=$(MOD_PREFIX) \
	$(MOD_SRC)
	touch mod-config

mod-compile: mod-config
	cd $(MOD_BUILD) && make -j$(JOBS)
	touch mod-compile

mod-install: mod-compile
	cd $(MOD_BUILD) && make install
ifeq ($(DEBUG_OT),n)
	$(TARGET)-strip --strip-unneeded $(MOD_PREFIX)/bin/*.dll
	$(TARGET)-strip -g $(MOD_PREFIX)/lib/*.a
	$(TARGET)-strip --strip-unneeded $(MOD_PREFIX)/Lib/site-packages/*/*.pyd
endif
	touch mod-install

mod-test: mod-install
	cd $(MOD_BUILD) && make tests -j$(JOBS_TEST)
ifeq ($(CHECK_OT),y)
	cp $(OT_PREFIX)/bin/*.dll $(MOD_BUILD)/lib/test
	cp $(MOD_PREFIX)/bin/*.dll $(MOD_BUILD)/lib/test
	cp $(OT_PREFIX)/bin/*.dll $(MOD_BUILD)/python/test
	cp $(MOD_PREFIX)/bin/*.dll $(MOD_BUILD)/python/test
	cp $(MINGW_PREFIX)/$(TARGET)/bin/python$(PYBASEVER_NODOT).dll $(MOD_BUILD)/python/test
	cd $(MOD_BUILD) && ctest -j$(JOBS) --output-on-failure --timeout 1000
	touch mod-test
endif

mod-check-installer: mod-test
	@echo -n "Check OpenTURNS prefix: "
	@ls $(MOD_PREFIX) > /dev/null
	@echo "ok"
	@echo -n "Check nsis compiler: "
	@makensis -VERSION > /dev/null 2>&1
	@echo "ok"
	@echo "OpenTURNS installer dependencies seems all right."
	touch mod-check-installer


mod-installer: mod-check-installer
	makensis -DMODULE_PREFIX=$(MOD_PREFIX) -DMODULE_VERSION=$(MODULE_VERSION) -DOPENTURNS_VERSION=$(OT_VERSION) -DPYBASEVER=$(PYBASEVER) -DPYBASEVER_NODOT=$(PYBASEVER_NODOT) -DARCH=$(ARCH) installer.nsi
	touch mod-installer


clean:
	-rm -f mod-*
	-cd $(MOD_BUILD) && make clean


mrproper:
	-rm -f mod-*
	-rm -rf $(MOD_PREFIX) $(MOD_BUILD)
	-rm -f *.exe


