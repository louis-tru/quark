
HOST_OS        ?= $(shell uname|tr '[A-Z]' '[a-z]')
NODE           ?= node
ANDROID_JAR     = out/android.classs.quark.jar
FPROJ           = ./libs/noproj
FPROJ_OUT       = out/noproj
REMOTE_COMPILE_HOST ?= 192.168.0.115

ifneq ($(USER),root)
	SUDO := "sudo"
endif

ifeq ($(HOST_OS),darwin)
	HOST_OS := osx
endif

#######################

FORWARD = make xcode msvs make-linux cmake-linux cmake build $(ANDROID_JAR) test2 clean

check_osx=\
	if [ "$(HOST_OS)" != "osx" ]; then \
		echo ;\
		echo target \"$(1)\" can only run on MAC system.;\
		echo ;\
		exit 1; \
	fi

.PHONY: $(FORWARD) ios android linux osx \
	product install install-noproj \
	help web doc watch all all_on_linux all_on_osx sync sync_skia

.SECONDEXPANSION:

# compile product quark and install
# It can only run in MAC system.
product:
	@$(MAKE) ios
	@$(MAKE) android
	@$(NODE) ./tools/cp-noproj.js

install: product
	@$(MAKE) install-noproj

install-noproj:
	@$(NODE) ./tools/cp-noproj.js
	@cd $(FPROJ_OUT) && npm i -f
	@cd $(FPROJ_OUT) && $(SUDO) npm i -g

$(FORWARD):
	@$(MAKE) -f build.mk $@

# build all ios platform and output to product dir
# It can only run in MAC system.
ios:
	@$(call check_osx,$@)
	@#./configure --os=ios --arch=arm --library=shared && $(MAKE) build # armv7 say goodbye 
	@./configure --os=ios --arch=x64   --library=shared && $(MAKE) build # simulator
	@./configure --os=ios --arch=arm64 --library=shared && $(MAKE) build
	@./configure --os=ios --arch=arm64 --library=shared -v8 --suffix=arm64.v8 && $(MAKE) build # handy v8 debug
	@./tools/gen_apple_frameworks.sh $(FPROJ_OUT) ios

# build all android platform and output to product dir
android:
	@./configure --os=android --arch=x64   --library=shared && $(MAKE) build
	@./configure --os=android --arch=arm   --library=shared && $(MAKE) build
	@./configure --os=android --arch=arm64 --library=shared && $(MAKE) build
	@$(MAKE) $(ANDROID_JAR)

linux:
	@./configure --os=linux   --arch=x64   --library=shared && $(MAKE) build
	@./configure --os=linux   --arch=arm   --library=shared && $(MAKE) build
	@./configure --os=linux   --arch=x64                    && $(MAKE) build
	@./configure --os=linux   --arch=arm                    && $(MAKE) build

osx:
	@$(call check_osx,$@)
	@echo Unsupported

# build all from current system platform

all:
	@if [ "$(HOST_OS)" = "osx" ]; then \
		$(MAKE) all_on_osx; \
	elif [ "$(HOST_OS)" = "linux" ]; then \
		$(MAKE) all_on_linux; \
	else \
		echo Unsupported current System "$(HOST_OS)"; \
	fi

all_on_osx:
	@$(MAKE) android
	@$(MAKE) ios
	@./configure --os=ios     --arch=arm   --library=shared && $(MAKE) build
	@./configure --os=android --arch=x86   --library=shared && $(MAKE) build
	@./configure --os=android --arch=x86                    && $(MAKE) build
	@./configure --os=android --arch=x64                    && $(MAKE) build
	@./configure --os=android --arch=arm                    && $(MAKE) build
	@./configure --os=android --arch=arm64                  && $(MAKE) build

all_on_linux:
	@$(MAKE) android
	@$(MAKE) linux
	@./configure --os=android --arch=x86   --library=shared && $(MAKE) build
	@./configure --os=android --arch=x86                    && $(MAKE) build
	@./configure --os=android --arch=x64                    && $(MAKE) build
	@./configure --os=android --arch=arm                    && $(MAKE) build
	@./configure --os=android --arch=arm64                  && $(MAKE) build

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
	@./tools/sync_watch -h $(REMOTE_COMPILE_HOST)

sync_skia:
	@python deps/skia/tools/git-sync-deps

sync: # init git submodule
	@if [ ! -f test/android/app/app.iml ]; then \
		cp test/android/app/.app.iml test/android/app/app.iml; \
	fi
	@git pull
	@git submodule update --init --recursive
	@if [ ! -d deps/skia/third_party/externals ]; then $(MAKE) sync_skia; fi
