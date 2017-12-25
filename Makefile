-include out/config.mk

ARCH					?= x64
SUFFIX				?= $(ARCH)
OS						?= `uname`
BUILDTYPE			?= Release
V							?= 0
CXX						?= g++
LINK					?= g++
ENV						?=
NODE					?= node
ANDROID_SDK		?= $(ANDROID_HOME)
ANDROID_JAR		?= $(ANDROID_SDK)/platforms/android-23/android.jar
JAVAC					?= javac
JAR						?= jar
TOOLS					= ./node_modules/ngui-tools
GYP						= $(TOOLS)/toolchain/gyp/gyp
LIBS_DIR 			= out/$(OS).$(SUFFIX).$(BUILDTYPE)
TOOLS_OUT			= out/ngui-tools
MAKE_STYLE		=	make

PTOJECTS = make xcode msvs make-linux cmake-linux cmake
GYPFILES = Makefile ngui.gyp tools/common.gypi \
					 out/config.gypi tools.gyp tools/tools.gypi
GYP_ARGS = -Goutput_dir="out" \
-Iout/var.gypi -Iout/config.gypi -Itools/common.gypi -S.$(OS).$(SUFFIX) --depth=.

ifeq ($(V), 1)
V_ARG = "V=1"
endif

ifeq ($(OS), android)
MAKE_STYLE = make-linux
endif

define make_compile
	$(ENV) $(MAKE) -C "out/$(MAKE_STYLE)" -f Makefile.$(OS).$(SUFFIX) \
CXX="$(CXX)" LINK="$(LINK)" $(V_ARG) BUILDTYPE=$(BUILDTYPE) \
builddir="$(shell pwd)/$(LIBS_DIR)"
endef

.PHONY: $(PTOJECTS) jsa-shell install install_inl \
	help all clear clear-all build server ios android doc tools

.SECONDEXPANSION:

###################### Build ######################

all: build

out/config.gypi: configure
	./configure

# GYP file generation targets.
$(PTOJECTS): $(GYPFILES)
	@echo "{'variables':{'project':'$@'}}" > out/var.gypi;
	@GYP_GENERATORS=$@ \
	$(GYP) -f $@ ngui.gyp --generator-output="out/$@" $(GYP_ARGS)

build: $(MAKE_STYLE) # out/$(MAKE_STYLE)/Makefile.$(OS).$(SUFFIX)
	@$(make_compile)

jsa-shell: $(GYPFILES)
	@echo "{'variables':{'project':'$(MAKE_STYLE)'}}" > out/var.gypi;
	$(GYP) -f $(MAKE_STYLE) tools.gyp --generator-output="out/$(MAKE_STYLE)" $(GYP_ARGS)
	@$(make_compile)
	@mkdir -p $(TOOLS)/bin/$(OS)
	@cp $(LIBS_DIR)/jsa-shell $(TOOLS)/bin/$(OS)/jsa-shell

#################################################

# build all ios platform and output to product dir
ios:
	#@./configure --os=ios --arch=arm --library=shared
	#@$(MAKE) build   # armv7 say goodbye 
	@./configure --os=ios --arch=x64 --library=shared
	@$(MAKE) build
	@./configure --os=ios --arch=arm64 --library=shared
	@$(MAKE) build
	@./configure --os=ios --arch=arm64 --library=shared \
		-v8 --suffix=arm64.v8 --without-embed-bitcode
	@$(MAKE) build
	@$(NODE) ./tools/gen_apple_framework.js ios \
					 $(TOOLS_OUT)/product/ios/iphonesimulator/Release/Frameworks \
					 ./out/ios.x64.Release/libngui.dylib 
	@$(NODE) ./tools/gen_apple_framework.js ios \
					 $(TOOLS_OUT)/product/ios/iphonesimulator/Debug/Frameworks \
					 ./out/ios.x64.Release/libngui.dylib 
	@$(NODE) ./tools/gen_apple_framework.js ios \
					 $(TOOLS_OUT)/product/ios/iphoneos/Release/Frameworks \
					 ./out/ios.arm64.Release/libngui.dylib # out/ios.armv7.Release/libngui.dylib
	@$(NODE) ./tools/gen_apple_framework.js ios \
					 $(TOOLS_OUT)/product/ios/iphoneos/Debug/Frameworks \
					 ./out/ios.arm64.v8.Release/libngui.dylib

# build all android platform and output to product dir
android:
	@./configure --os=android --arch=x86 --library=shared
	@$(MAKE) build
	@./configure --os=android --arch=arm64 --library=shared
	@$(MAKE) build
	@./configure --os=android --arch=arm --library=shared
	@$(MAKE) build
	@$(MAKE) out/android.classs.ngui.jar

out/android.classs.ngui.jar: android/org/ngui/*.java
	@mkdir -p out/android.classs
	@rm -rf out/android.classs/*
	@$(JAVAC) -bootclasspath $(ANDROID_JAR) -d out/android.classs android/org/ngui/*.java
	@cd out/android.classs; $(JAR) cfv ngui.jar .
	@mkdir -p $(TOOLS_OUT)/product/android/libs
	@cp out/android.classs/ngui.jar $(TOOLS_OUT)/product/android/libs

# install ngui command
install:
	@$(MAKE) ios
	@$(MAKE) android
	@./configure --media=0
	@$(MAKE) jsa-shell
	@$(MAKE) tools

# debug install ngui command
install_inl:
	@./configure --media=0
	@$(MAKE) jsa-shell
	@./$(TOOLS)/install link

tools:
	@sudo rm -rf ./out/ngui-tools/bin/shell.js
	@$(NODE) ./tools/cp-ngui-tools.js
	@$(TOOLS_OUT)/install

doc:
	@$(NODE) tools/gen_html_doc.js doc out/doc

server:
	@$(NODE) tools/server.js

clear:
	@rm -rfv out/$(OS).*
	@rm -rfv out/product/ngui/product/$(OS)

clear-all:
	@rm -rfv out

help:
	@echo
	@echo exec \"make\" or \"make build\" start compile
	@echo exec \"make xcode\" output xcode project file
	@echo You must first call before calling make \"./configure\"
	@echo
