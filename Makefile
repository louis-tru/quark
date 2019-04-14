
HOST_OS       ?= $(shell uname|tr '[A-Z]' '[a-z]')
NODE          ?= node
ANDROID_JAR    = out/android.classs.qgr.jar
QMAKE          = ./libs/qmake
QMAKE_OUT      = out/qmake
GIT_repository := $(shell git remote -v|grep origin|tail -1|awk '{print $$2}'|cut -d "/" -f 1)

ifneq ($(USER),root)
	SUDO := "sudo"
endif

ifeq ($(HOST_OS),darwin)
	HOST_OS := osx
endif

ifeq ($(GIT_repository),)
	GIT_repository = https://github.com/louis-tru
endif

JSA_SHELL = $(QMAKE)/bin/${HOST_OS}/jsa-shell

#######################

DEPS = libs/qkit libs/qmake/gyp.qgr depe/v8-link \
	depe/FFmpeg.qgr depe/node.qgr depe/bplus
FORWARD = make xcode msvs make-linux cmake-linux cmake build build-jsa $(ANDROID_JAR) test2 clean

git_pull=sh -c "\
	if [ ! -f $(1)/.git/config ]; then \
		git clone $(3) $(1) && cd $(1) && git checkout $(2) && echo git clone $(3) ok; \
	else \
		cd $(1) && git checkout $(2) && git pull && echo git pull $(1) ok; \
	fi"

git_push=sh -c "cd $(1) && git push && echo git push $(1) ok"

git_pull_deps=echo $(1) deps \
	$(foreach i,$(DEPS),&& \
		$(call git_$(1),\
			$(call basename,$(i)),\
			$(if $(call suffix,$(i)),$(call subst,.,,$(call suffix,$(i))),master),\
			$(GIT_repository)/$(call subst,$(call suffix,$(i)),,$(call notdir,$(i))).git\
		) \
	)

gen_framework=\
	$(NODE) ./tools/gen_apple_framework.js ios $(1) "cut" "$(2)" \
	$(QMAKE_OUT)/product/ios/$(3)/Frameworks/$(4) \
	$(foreach i,$(5), out/ios.$(i).Release.shared/lib$(1).dylib)

check_osx=\
	if [ "$(HOST_OS)" != "osx" ]; then \
		echo ;\
		echo Error:;\
		echo target \"$(1)\" can only run on MAC system.;\
		echo ;\
		exit 1; \
	fi

.PHONY: all $(FORWARD) jsa ios android linux osx \
	install install-qmake-link install-qmake \
	help web doc watch build-linux-all build-osx-all pull push

.SECONDEXPANSION:

# install qgr
# It can only run in MAC system.
install: pull
	@$(MAKE) ios
	@$(MAKE) android
	@$(MAKE) install-qmake

install-qmake: $(JSA_SHELL)
	@$(NODE) ./tools/cp-qmake.js
	@cd $(QMAKE_OUT) && npm i -f
	@cd $(QMAKE_OUT) && $(SUDO) npm i -g

# debug install qgr
install-qmake-link: $(JSA_SHELL)
	@cd $(QMAKE_OUT) && $(SUDO) npm link -g

$(FORWARD):
	@$(MAKE) -f build.mk $@

$(JSA_SHELL): jsa

# build all ios platform and output to product dir
# It can only run in MAC system.
ios: $(JSA_SHELL)
	@$(call check_osx,$@)
	@#./configure --os=ios --arch=arm --library=shared && $(MAKE) build # armv7 say goodbye 
	@./configure --os=ios --arch=x64   --library=shared && $(MAKE) build
	@./configure --os=ios --arch=arm64 --library=shared && $(MAKE) build
	@./configure --os=ios --arch=arm64 --library=shared -v8 --suffix=arm64.v8 && $(MAKE) build # handy debug

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
android: $(JSA_SHELL) $(ANDROID_JAR)
	@./configure --os=android --arch=x64   --library=shared && $(MAKE) build
	@./configure --os=android --arch=arm64 --library=shared && $(MAKE) build

linux: $(JSA_SHELL)
	@./configure --os=linux   --arch=x64   --library=shared && $(MAKE) build
	@./configure --os=linux   --arch=arm   --library=shared && $(MAKE) build

osx:
	@echo Unsupported

all: pull
	@if [ "$(HOST_OS)" = "osx" ]; then \
		$(MAKE) build-osx-all; \
	elif [ "$(HOST_OS)" = "linux" ]; then \
		$(MAKE) build-linux-all; \
	else \
		echo Unsupported current System "$(HOST_OS)"; \
	fi

build-osx-all:
	@$(MAKE) android
	@$(MAKE) ios
	@./configure --os=ios     --arch=arm   --library=shared && $(MAKE) build
	@./configure --os=ios     --arch=x64   && $(MAKE) build
	@./configure --os=android --arch=arm   --library=shared && $(MAKE) build
	@./configure --os=android --arch=x86   --library=shared && $(MAKE) build
	@./configure --os=android --arch=x86   && $(MAKE) build
	@./configure --os=android --arch=x64   && $(MAKE) build
	@./configure --os=android --arch=arm   && $(MAKE) build
	@./configure --os=android --arch=arm64 && $(MAKE) build

build-linux-all:
	@$(MAKE) android
	@$(MAKE) linux
	@./configure --os=android --arch=arm   --library=shared && $(MAKE) build
	@./configure --os=android --arch=x86   --library=shared && $(MAKE) build
	@./configure --os=android --arch=x86   && $(MAKE) build
	@./configure --os=android --arch=x64   && $(MAKE) build
	@./configure --os=android --arch=arm   && $(MAKE) build
	@./configure --os=android --arch=arm64 && $(MAKE) build
	@./configure --os=linux   --arch=x64   && $(MAKE) build
	@./configure --os=linux   --arch=arm   && $(MAKE) build

jsa:
	@./configure --media=0
	@$(MAKE) build-jsa

doc:
	@$(NODE) tools/gen_html_doc.js doc out/doc

web:
	@$(NODE) --inspect=0.0.0.0:9229 tools/server.js

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
	@$(call git_pull_deps,pull)

push:
	@git push
	@$(call git_pull_deps,push)