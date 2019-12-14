include out/config.mk

ARCH          ?= x64
SUFFIX        ?= $(ARCH)
HOST_OS       ?= $(shell uname|tr '[A-Z]' '[a-z]')
BRAND         ?=
SHARED        ?=
OS            ?= $(HOST_OS)
BUILDTYPE     ?= Release
V             ?= 0
CXX           ?= g++
LINK          ?= g++
ANDROID_SDK   ?= $(ANDROID_HOME)
ANDROID_LIB   ?= $(ANDROID_SDK)/platforms/android-24/android.jar
ANDROID_JAR    = out/android.classs.ngui.jar
JAVAC         ?= javac
JAR            = jar
ENV           ?=
NXMAKE          = ./libs/nxmake
NXMAKE_OUT      = out/nxmake
GYP            = $(NXMAKE)/gyp/gyp
OUTPUT        ?= $(OS).$(SUFFIX).$(BUILDTYPE)
LIBS_DIR       = out/$(OUTPUT)
BUILD_STYLE    = make

#######################

STYLES		= make xcode msvs make-linux cmake-linux cmake
GYPFILES	= Makefile ngui.gyp tools/common.gypi out/config.gypi tools.gyp tools/tools.gypi
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

.PHONY: $(STYLES) all compile tools test2 clean

.SECONDEXPANSION:

###################### Build ######################

all: compile

# GYP file generation targets.
$(STYLES): $(GYPFILES)
	@$(call gen_project,$@,ngui.gyp)

compile: $(BUILD_STYLE) # out/$(BUILD_STYLE)/Makefile.$(OS).$(SUFFIX)
	@$(call make_compile,$(MAKE))

tools: $(GYPFILES)
	@$(call gen_project,$(BUILD_STYLE),tools.gyp)
	@$(call make_compile,$(MAKE))
	@cp $(LIBS_DIR)/jsa-shell $(NXMAKE)/bin/$(OS)-jsa-shell

test2: $(GYPFILES)
	@#make -C test -f test2.mk
	@$(call gen_project,$(BUILD_STYLE),test2.gyp)
	@$(call make_compile,$(MAKE))

$(ANDROID_JAR): android/org/ngui/*.java
	@mkdir -p out/android.classs
	@rm -rf out/android.classs/*
	@$(JAVAC) -bootclasspath $(ANDROID_LIB) -d out/android.classs android/org/ngui/*.java
	@cd out/android.classs; $(JAR) cfv ngui.jar .
	@mkdir -p $(NXMAKE_OUT)/product/android/libs
	@cp out/android.classs/ngui.jar $(NXMAKE_OUT)/product/android/libs

clean:
	@rm -rfv $(LIBS_DIR)
	@rm -rfv out/nxmake/product/$(OS)
