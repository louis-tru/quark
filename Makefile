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
	if [ "$(1)" != "$(2)" ]; then \
		echo ;\
		echo target \"$(3)\" can only build on $(2) system.;\
		echo ;\
		exit 1; \
	fi

remote_build=\
	ssh $(REMOTE_COMPILE_HOST) 'bash -l -s' < tools/remote_build.sh $(1) $(V) && \
	scp $(REMOTE_COMPILE_HOST):~/quark/out/remote_build.tgz out && \
	cd out && \
	tar xfvz remote_build.tgz

.PHONY: $(FORWARD) ios android linux mac \
	install-only install help web doc watch all sync r_android r_linux

.SECONDEXPANSION:

$(FORWARD):
	@$(MAKE) -f build.mk $@

all:
	@$(MAKE) ios
	@$(MAKE) mac
	@if [ "$(HOST_ARCH)" = "x64" ]; then $(MAKE) android; else $(MAKE) r_android; fi
	@if [ "$(HOST_OS)" = "linux" ]; then $(MAKE) linux; else $(MAKE) r_linux; fi
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
	@$(call check,$(HOST_OS),mac,$@)
	@./configure --os=ios --arch=arm64 && $(MAKE) build
	@./configure --os=ios --arch=arm64 -em && $(MAKE) build # simulator for mac
	@./configure --os=ios --arch=x64 -em && $(MAKE) build
	@./tools/gen_apple_frameworks.sh $(QKMAKE_OUT) ios

mac:
	@$(call check,$(HOST_OS),mac,$@)
	@./configure --os=mac --arch=arm64 -v8 && $(MAKE) build
	@./configure --os=mac --arch=x64       && $(MAKE) build
	@./tools/gen_apple_frameworks.sh $(QKMAKE_OUT) mac

# build all android platform and output to product dir
android:
	@$(call check,$(HOST_ARCH),x64,$@)
	@./configure --os=android --arch=arm64 && $(MAKE) build
	@./configure --os=android --arch=x64   && $(MAKE) build
	@$(MAKE) $(ANDROID_JAR)

linux:
	@$(call check,$(HOST_OS),linux,$@)
	@./configure --os=linux   --arch=arm64 && $(MAKE) build
	@#./configure --os=linux  --arch=arm && $(MAKE) build
	@./configure --os=linux   --arch=x64   && $(MAKE) build

r_android:
	$(call remote_build,android)

r_linux:
	$(call remote_build,linux)

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