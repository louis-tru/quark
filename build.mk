include out/config.mk

ARCH          ?= x64
SUFFIX        ?= $(ARCH)
OS            ?= `uname`
BUILDTYPE     ?= Release
V             ?= 0
CXX           ?= g++
LINK          ?= g++
SHARED        ?=
ENV           ?=
QMAKE         = ./libs/qmake
GYP           = $(QMAKE)/gyp/gyp
LIBS_DIR      = out/$(OS).$(SUFFIX).$(BUILDTYPE)$(if $(SHARED),.$(SHARED))
BUILD_STYLE   = make

#######################

STYLES		= make xcode msvs make-linux cmake-linux cmake
GYPFILES	= Makefile qgr.gyp tools/common.gypi out/config.gypi tools.gyp tools/tools.gypi
GYP_ARGS	= -Goutput_dir="out" \
-Iout/var.gypi -Iout/config.gypi -Itools/common.gypi -S.$(OS).$(SUFFIX) --depth=.

ifeq ($(V), 1)
V_ARG = "V=1"
endif

ifeq ($(OS), android)
BUILD_STYLE = make-linux
endif

gen_project=\
	echo "{'variables':{'project':'$(1)'}}" > out/var.gypi; \
	GYP_GENERATORS=$(1) $(GYP) -f $(1) $(2) --generator-output="out/$(1)" $(GYP_ARGS)

make_compile=\
	$(ENV) $(1) -C "out/$(BUILD_STYLE)" -f Makefile.$(OS).$(SUFFIX) \
	CXX="$(CXX)" LINK="$(LINK)" $(V_ARG) BUILDTYPE=$(BUILDTYPE) \
	builddir="$(shell pwd)/$(LIBS_DIR)"

.PHONY: $(STYLES) all build build-jsa test2 clean

.SECONDEXPANSION:

###################### Build ######################

all: build

# GYP file generation targets.
$(STYLES): $(GYPFILES)
	@$(call gen_project,$@,qgr.gyp)

build: $(BUILD_STYLE) # out/$(BUILD_STYLE)/Makefile.$(OS).$(SUFFIX)
	@$(call make_compile,$(MAKE))

build-jsa: $(GYPFILES)
	@$(call gen_project,$(BUILD_STYLE),tools.gyp)
	@$(call make_compile,$(MAKE))
	@mkdir -p $(QMAKE)/bin/$(OS)
	@cp $(LIBS_DIR)/jsa-shell $(QMAKE)/bin/$(OS)/jsa-shell

test2: $(GYPFILES)
	@#make -C test -f test2.mk
	@$(call gen_project,$(BUILD_STYLE),test2.gyp)
	@$(call make_compile,$(MAKE))

clean:
	@rm -rfv $(LIBS_DIR)
	@rm -rfv out/qmake/product/$(OS)