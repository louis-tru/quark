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
ANDROID_LIB   ?= $(ANDROID_SDK)/platforms/android-28/android.jar
ANDROID_JAR    = out/android.classs.ftr.jar
JAVAC         ?= javac
JAR            = jar
ENV           ?=
FTRP           = ./libs/ftrp
FTRP_OUT       = out/ftrp
GYP            = $(FTRP)/gyp/gyp
OUTPUT        ?= $(OS).$(SUFFIX).$(BUILDTYPE)
LIBS_DIR       = out/$(OUTPUT)
BUILD_STYLE   ?= make
BUILD_FILE    ?= ftr

#######################

STYLES		= make xcode msvs make-linux cmake-linux cmake
GYPFILES	= Makefile ftr.gyp tools/common.gypi out/config.gypi trial/trial.gypi
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

.PHONY: $(STYLES) all build test2 clean

.SECONDEXPANSION:

###################### Build ######################

all: build # compile

# GYP file generation targets.
$(STYLES): $(GYPFILES)
	@$(call gen_project,$@,$(BUILD_FILE).gyp)

build: $(BUILD_STYLE) # out/$(BUILD_STYLE)/Makefile.$(OS).$(SUFFIX)
	@$(call make_compile,$(MAKE))

test2: $(GYPFILES)
	@#make -C test -f test2.mk
	@$(call gen_project,$(BUILD_STYLE),test2.gyp)
	@$(call make_compile,$(MAKE))

$(ANDROID_JAR): android/org/ftr/*.java
	@mkdir -p out/android.classs
	@rm -rf out/android.classs/*
	$(JAVAC) -classpath $(ANDROID_LIB) -d out/android.classs android/org/ftr/*.java
	@cd out/android.classs; $(JAR) cfv ftr.jar .
	@mkdir -p $(FTRP_OUT)/product/android/libs
	@cp out/android.classs/ftr.jar $(FTRP_OUT)/product/android/libs

clean:
	@rm -rfv $(LIBS_DIR)
	@rm -rfv out/ftrp/product/$(OS)
