include out/config.mk

ARCH          ?= x64
SUFFIX        ?= $(ARCH)
NODE          ?= node
OS            ?= $(shell uname|tr '[A-Z]' '[a-z]')
BUILDTYPE     ?= Release
V             ?= 0
CXX           ?= g++
LINK          ?= g++
ANDROID_LIB   ?= $(ANDROID_SDK)/platforms/android-28/android.jar
ANDROID_JAR   ?= out/android.classs.quark.jar
JAVAC         ?= javac
JAR           ?= jar
QKMAKE        ?= libs/qkmake
QKMAKE_OUT    ?= out/qkmake
GYP           ?= $(QKMAKE)/gyp-next/gyp
OUTPUT        ?= $(OS).$(SUFFIX).$(BUILDTYPE)
LIBS_DIR      ?= out/$(OUTPUT)
STYLE         ?= make

#######################

STYLES    = make xcode msvs make-linux cmake-linux cmake
GYPFILES  = Makefile quark.gyp tools/common.gypi out/config.gypi trial/trial.gypi
GYP_ARGS  = -Goutput_dir="out" \
	-Iout/var.gypi -Iout/config.gypi -Itools/common.gypi -S.$(OS).$(SUFFIX) --depth=.

ifeq ($(V),1)
	V_ARG = "V=1"
endif

ifeq ($(OS), android)
	STYLE = make-linux
endif

.PHONY: $(STYLES) build test2 clean

.SECONDEXPANSION:

###################### Build ######################

#@$(call generator,$@,quark.gyp)

# GYP file generation targets.
$(STYLES): $(GYPFILES)
	@$(NODE) -e "require('fs').writeFileSync('out/var.gypi', '{\'variables\':{\'project\':\'$@\'}}')"
	@$(GYP) -f $@ quark.gyp --generator-output="out/$@" $(GYP_ARGS)
	@$(NODE) tools/touch.js

build: $(STYLE) # out/$(STYLE)/Makefile.$(OS).$(SUFFIX)
	$(MAKE) -C "out/$(STYLE)" -f Makefile.$(OS).$(SUFFIX) \
		$(V_ARG) BUILDTYPE=$(BUILDTYPE) builddir="$(shell pwd)/$(LIBS_DIR)"
	$(NODE) tools/cp_library.js

test2: $(GYPFILES)
	@make -C test/2 -f test2.mk

$(ANDROID_JAR): src/platforms/android/org/quark/*.java
	@mkdir -p out/android.classs
	@rm -rf out/android.classs/*
	$(JAVAC) -classpath $(ANDROID_LIB) -d out/android.classs src/platforms/android/org/quark/*.java
	@cd out/android.classs && $(JAR) cfv quark.jar .
	@mkdir -p $(QKMAKE_OUT)/product/android/libs
	@cp out/android.classs/quark.jar $(QKMAKE_OUT)/product/android/libs

clean:
	@rm -rfv $(LIBS_DIR)
	@rm -rfv out/noproj/product/$(OS)
