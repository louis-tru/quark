-include .config.mk
HOST_OS        ?= $(shell uname|tr '[A-Z]' '[a-z]')
NODE           ?= node
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

FORWARD = make xcode msvs make-linux cmake-linux cmake build $(ANDROID_JAR) test2 clean

check_mac=\
	if [ "$(HOST_OS)" != "mac" ]; then \
		echo ;\
		echo target \"$(1)\" can only run on MAC system.;\
		echo ;\
		exit 1; \
	fi

.PHONY: $(FORWARD) ios android linux mac \
	product install install-qkmake \
	help web doc watch all _host_linux _host_mac sync

.SECONDEXPANSION:

# compile product quark and install
# It can only run in MAC system.
product:
	@$(MAKE) ios
	@$(MAKE) android
	@$(NODE) ./tools/cp_qkmake.js

install: product
	@$(MAKE) install-qkmake

install-qkmake:
	@$(NODE) ./tools/cp_qkmake.js
	@cd $(QKMAKE_OUT) && npm i -f
	@cd $(QKMAKE_OUT) && $(SUDO) npm i -g

$(FORWARD):
	@$(MAKE) -f build.mk $@

# build all ios platform and output to product dir
# It can only run in MAC system.
ios:
	@$(call check_mac,$@)
	@#./configure --os=ios --arch=arm  && $(MAKE) build # armv7 say goodbye
	@./configure --os=ios --arch=arm64 && $(MAKE) build
	@./configure --os=ios --arch=arm64 -em && $(MAKE) build # simulator for mac
	@./tools/gen_apple_frameworks.sh $(QKMAKE_OUT) ios

# build all android platform and output to product dir
android:
	@./configure --os=android --arch=x64   && $(MAKE) build
	@./configure --os=android --arch=arm   && $(MAKE) build
	@./configure --os=android --arch=arm64 && $(MAKE) build
	@$(MAKE) $(ANDROID_JAR)

linux:
	@./configure --os=linux   --arch=x64   && $(MAKE) build
	@./configure --os=linux   --arch=arm   && $(MAKE) build
	@./configure --os=linux   --arch=arm64 && $(MAKE) build

mac:
	@$(call check_mac,$@)
	@./configure --os=mac --arch=x64   && $(MAKE) build
	@./configure --os=mac --arch=arm64 && $(MAKE) build
	@./tools/gen_apple_frameworks.sh $(QKMAKE_OUT) mac

# build all from current system platform

all:
	@if [ "$(HOST_OS)" = "mac" ]; then \
		$(MAKE) _host_mac; \
	elif [ "$(HOST_OS)" = "linux" ]; then \
		$(MAKE) _host_linux; \
	else \
		echo Unsupported current System "$(HOST_OS)"; \
	fi

# build all on mac
_host_mac:
	@$(MAKE) android
	@$(MAKE) ios
	@$(MAKE) osx

# build all on linex os
_host_linux:
	@$(MAKE) android
	@$(MAKE) linux

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