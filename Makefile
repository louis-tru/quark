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

FORWARD = build make xcode msvs make-linux cmake-linux cmake $(ANDROID_JAR) test2 clean

check_os=\
	if [ "$(HOST_OS)" != "$(1)" ]; then \
		echo ;\
		echo target \"$(2)\" can only run on $(1) system.;\
		echo ;\
		exit 1; \
	fi

.PHONY: $(FORWARD) ios android linux mac \
	install-only install help web doc watch all sync

.SECONDEXPANSION:

$(FORWARD):
	@$(MAKE) -f build.mk $@

# compile product quark and install
# It can only run in MAC system.
all:
	@$(MAKE) ios
	@$(MAKE) android
	@$(MAKE) mac
	@$(MAKE) linux
	@$(NODE) tools/cp_qkmake.js

install-only:
	@$(NODE) tools/cp_qkmake.js
	@cd $(QKMAKE_OUT) && npm i -f
	@cd $(QKMAKE_OUT) && $(SUDO) npm i -g

install: all
	@$(MAKE) install-only

# build all ios platform and output to product dir
# It can only run in MAC system.
ios:
	@$(call check_os,mac,$@)
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

mac:
	@$(call check_os,mac,$@)
	@./configure --os=mac --arch=x64   && $(MAKE) build
	@./configure --os=mac --arch=arm64 && $(MAKE) build
	@./tools/gen_apple_frameworks.sh $(QKMAKE_OUT) mac

linux:
	@$(call check_os,linux,$@)
	@./configure --os=linux   --arch=x64   && $(MAKE) build
	@./configure --os=linux   --arch=arm   && $(MAKE) build
	@./configure --os=linux   --arch=arm64 && $(MAKE) build

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