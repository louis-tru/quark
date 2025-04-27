-include .config.mk
NODE           ?= node
HOST_OS        ?= $(shell $(NODE) -e 'console.log(process.platform)')
HOST_ARCH      ?= $(shell $(NODE) -e 'console.log(process.arch)')
ANDROID_JAR    := out/android.classs.quark.jar
QKMAKE         := ./libs/qkmake
QKMAKE_OUT     := out/qkmake
REMOTE_COMPILE_HOST ?= 192.168.2.202

ifneq ($(USER),root)
	SUDO := "sudo"
endif

ifeq ($(HOST_OS),darwin)
	HOST_OS := mac
endif

#######################

FORWARD = build make xcode msvs make-linux cmake-linux cmake $(ANDROID_JAR) test2 clean

check=\
	if [ "$(filter $(1),$(2))" = "" ]; then \
		echo ;\
		echo target \"$(3)\" can only build on $(1) system.;\
		echo ;\
		exit 1; \
	fi

local_build=\
	$(if $(1),./configure $(1)) && $(MAKE) $(2)

remote_build=\
	./tools/remote_build.sh $(REMOTE_COMPILE_HOST) "$(1)" $(2) $(3)

maybe_remote_build=\
	$(call $(if $(1),local_build,remote_build),$(2),$(3),$(4))

.PHONY: $(FORWARD) ios android linux mac \
	install-only install help web doc watch all sync try_android try_linux

.SECONDEXPANSION:

$(FORWARD):
	@$(MAKE) -f build.mk $@

all:
	@$(MAKE) ios
	@$(MAKE) mac
	@$(MAKE) try_android
	@$(MAKE) try_linux

install-only:
	@$(NODE) tools/cp_qkmake.js
	@cd $(QKMAKE_OUT) && npm i -f --ignore-scripts
	@cd $(QKMAKE_OUT) && $(SUDO) npm i -g --ignore-scripts

install: all
	@$(MAKE) install-only

# build all ios platform and output to product dir
# It can only run in MAC system.
ios:
	@$(call check,mac,$(HOST_OS),$@)
	@./configure --os=ios --arch=arm64 && $(MAKE) build
	@./configure --os=ios --arch=arm64 -v8 && $(MAKE) build
	@./configure --os=ios --arch=arm64 -em -v8 && $(MAKE) build # simulator for mac
	@./configure --os=ios --arch=x64 -em && $(MAKE) build
	@./tools/gen_apple_frameworks.sh $(QKMAKE_OUT) ios

mac:
	@$(call check,mac,$(HOST_OS),$@)
	@./configure --os=mac --arch=arm64 -v8 && $(MAKE) build
	@./configure --os=mac --arch=x64       && $(MAKE) build
	@./tools/gen_apple_frameworks.sh $(QKMAKE_OUT) mac

# build all android platform and output to product dir
android:
	@$(call check,x64 arm64,$(HOST_ARCH),$@)
	@./configure --os=android --arch=arm64 && $(MAKE) build
	@$(call check,x64,$(HOST_ARCH),$@)
	@./configure --os=android --arch=x64   && $(MAKE) build
	@$(MAKE) $(ANDROID_JAR)

linux:
	@$(call check,linux,$(HOST_OS),$@)
	@./configure --os=linux   --arch=x64   && $(MAKE) build
	@./configure --os=linux   --arch=arm64 && $(MAKE) build

# try local and remote build
try_android:
	$(call maybe_remote_build,$(filter x64 arm64,$(HOST_ARCH)),\
			--os=android --arch=arm64,build,android/jniLibs/arm64-v8a)
	$(call maybe_remote_build,$(filter x64,$(HOST_ARCH)),\
			--os=android --arch=x64,build,android/jniLibs/x86_64/libquark.so)
	@$(MAKE) $(ANDROID_JAR)

try_linux:
	$(call maybe_remote_build,$(filter linux,$(HOST_OS)),"",linux,linux)

doc:
	@$(NODE) tools/gen_html_doc.js doc out/doc

web:
	@$(NODE) --inspect=0.0.0.0:9228 tools/server.js

help:
	@echo
	@echo Run \"make\" start build product
	@echo Run \"make xcode\" output xcode project file
	@echo You must first call before calling make \"./configure\"
	@echo

watch:
	@./tools/sync_watch -h $(REMOTE_COMPILE_HOST) -i .tmp

sync: # init git submodule
	@if [ ! -f test/android/app/app.iml ]; then \
		cp test/android/app/.app.iml test/android/app/app.iml; \
	fi
	@git pull
	@git submodule update --init --recursive