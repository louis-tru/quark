include out/config.mk

ARCH					?= x64
SUFFIX				?= $(ARCH)
OS						?= `uname`
HO_STOS				?= `uname`
BUILDTYPE			?= Release
V							?= 0
CXX						?= g++
LINK					?= g++
SHARED				?=
ENV						?=
NODE					?= node
ANDROID_SDK		?= $(ANDROID_HOME)
ANDROID_JAR		?= $(ANDROID_SDK)/platforms/android-24/android.jar
JAVAC					?= javac
JAR						?= jar
QMAKE					= ./libs/qmake
GYP						= $(QMAKE)/gyp/gyp
LIBS_DIR 			= out/$(OS).$(SUFFIX).$(BUILDTYPE)$(if $(SHARED),.$(SHARED))
QMAKE_OUT			= out/qmake
BUILD_STYLE 	=	make
JSA_SHELL 		= $(QMAKE)/bin/${HO_STOS}/jsa-shell

ifneq ($(USER),root)
	SUDO = "sudo"
endif

#######################

DEPS			= libs/qkit libs/qmake/gyp.qgr depe/v8-link depe/FFmpeg.qgr depe/node.qgr
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

git_pull=sh -c "\
	if [ ! -f $(1)/.git/config ]; then \
		git clone $(3) $(1) && cd $(1) && git checkout $(2) && echo git clone $(3) ok; \
	else \
		cd $(1) && git checkout $(2) && git pull && echo git pull $(1) ok; \
	fi"

#https://github.com/louis-tru/qgr.git
#https://gitee.com/louis-tru/qgr.git

git_pull_deps=echo pull deps \
	$(foreach i,$(DEPS),&& \
		$(call git_pull,\
			$(call basename,$(i)),\
			$(if $(call suffix,$(i)),$(call subst,.,,$(call suffix,$(i))),master),\
			https://github.com/louis-tru/$(call subst,$(call suffix,$(i)),,$(call notdir,$(i))).git\
		) \
	)

.PHONY: $(STYLES) jsa-shell install install-qmake-link install-qmake \
	help all clean build web ios android osx doc test2 watch build-linux-all pull

.SECONDEXPANSION:

###################### Build ######################

all: build

# GYP file generation targets.
$(STYLES): $(GYPFILES)
	@$(call gen_project,$@,qgr.gyp)

build: $(BUILD_STYLE) # out/$(BUILD_STYLE)/Makefile.$(OS).$(SUFFIX)
	@$(call make_compile,$(MAKE))

jsa-shell: $(GYPFILES)
	@$(call gen_project,$(BUILD_STYLE),tools.gyp)
	@$(call make_compile,$(MAKE))
	@mkdir -p $(QMAKE)/bin/$(OS)
	@cp $(LIBS_DIR)/jsa-shell $(QMAKE)/bin/$(OS)/jsa-shell

test2: $(GYPFILES)
	@#make -C test -f test2.mk
	@$(call gen_project,$(BUILD_STYLE),test2.gyp)
	@$(call make_compile,$(MAKE))

out/android.classs.qgr.jar: android/org/qgr/*.java
	@mkdir -p out/android.classs
	@rm -rf out/android.classs/*
	@$(JAVAC) -bootclasspath $(ANDROID_JAR) -d out/android.classs android/org/qgr/*.java
	@cd out/android.classs; $(JAR) cfv qgr.jar .
	@mkdir -p $(QMAKE_OUT)/product/android/libs
	@cp out/android.classs/qgr.jar $(QMAKE_OUT)/product/android/libs

$(JSA_SHELL):
	@./configure --media=0
	@$(MAKE) jsa-shell

#################################################

gen_framework=\
	$(NODE) ./tools/gen_apple_framework.js ios $(1) "cut" "$(2)" \
	$(QMAKE_OUT)/product/ios/$(3)/Frameworks/$(4) \
	$(foreach i,$(5), out/ios.$(i).Release.shared/lib$(1).dylib)

# build all ios platform and output to product dir
ios: $(JSA_SHELL)
	#@./configure --os=ios --arch=arm --library=shared && $(MAKE) # armv7 say goodbye 
	@./configure --os=ios --arch=x64   --library=shared && $(MAKE)
	@./configure --os=ios --arch=arm64 --library=shared && $(MAKE)
	@./configure --os=ios --arch=arm64 --library=shared -v8 --suffix=arm64.v8 && $(MAKE)

	@$(call gen_framework,qgr,,iphonesimulator,,x64)
	@$(call gen_framework,qgr-media,no-inc,iphonesimulator,,x64)
	@$(call gen_framework,qgr-v8,depe/v8-link/include,iphonesimulator,,x64)
	@$(call gen_framework,qgr-js,no-inc,iphonesimulator,,x64)
	@$(call gen_framework,qgr-node,no-inc,iphonesimulator,,x64)

	@$(call gen_framework,qgr,,iphoneos,,arm64) # arm64 armv7
	@$(call gen_framework,qgr-media,no-inc,iphoneos,,arm64)
	@$(call gen_framework,qgr-v8,depe/v8-link/include,iphoneos,,arm64)
	@$(call gen_framework,qgr-js,no-inc,iphoneos,,arm64)
	@$(call gen_framework,qgr-node,no-inc,iphoneos,,arm64)

	@$(call gen_framework,qgr-v8,depe/v8-link/include,iphoneos,Debug,arm64.v8)
	@$(call gen_framework,qgr-js,no-inc,iphoneos,Debug,arm64.v8)
	@$(call gen_framework,qgr-node,no-inc,iphoneos,Debug,arm64.v8)

# build all android platform and output to product dir
android: $(JSA_SHELL) out/android.classs.qgr.jar
	@./configure --os=android --arch=x86   --library=shared && $(MAKE)
	@./configure --os=android --arch=arm64 --library=shared && $(MAKE)
	@./configure --os=android --arch=arm   --library=shared && $(MAKE)

install-qmake:
	@$(NODE) ./tools/cp-qmake.js
	@cd $(QMAKE_OUT) && npm i -f
	@cd $(QMAKE_OUT) && $(SUDO) npm i -g

# debug install qgr
install-qmake-link: $(JSA_SHELL)
	@cd $(QMAKE_OUT) && $(SUDO) npm link -g

# install qgr
install: install-qmake
	@$(MAKE) ios
	@$(MAKE) android

#################################################

build-linux-all: $(JSA_SHELL)
	@./configure --os=linux   --arch=x64   --library=shared && $(MAKE)
	@./configure --os=linux   --arch=x64                    && $(MAKE)
	@./configure --os=linux   --arch=arm   --library=shared && $(MAKE)
	@./configure --os=linux   --arch=arm                    && $(MAKE)
	@./configure --os=android --arch=x86                    && $(MAKE)
	@./configure --os=android --arch=x86   --library=shared && $(MAKE)
	@./configure --os=android --arch=x64                    && $(MAKE)
	@./configure --os=android --arch=x64   --library=shared && $(MAKE)
	@./configure --os=android --arch=arm                    && $(MAKE)
	@./configure --os=android --arch=arm   --library=shared && $(MAKE)
	@./configure --os=android --arch=arm64                  && $(MAKE)
	@./configure --os=android --arch=arm64 --library=shared && $(MAKE)

doc:
	@$(NODE) tools/gen_html_doc.js doc out/doc

web:
	@$(NODE) --inspect=0.0.0.0:9229 tools/server.js

clean:
	@rm -rfv $(LIBS_DIR)
	@rm -rfv $(QMAKE_OUT)/product/$(OS)

help:
	@echo
	@echo Run \"make\" start compile
	@echo Run \"make xcode\" output xcode project file
	@echo You must first call before calling make \"./configure\"
	@echo

watch:
	@./tools/sync_watch

pull:
	@git pull
	@$(call git_pull_deps)
