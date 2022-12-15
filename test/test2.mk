
include ../out/config.mk

SYSROOT    ?= /
OS         ?= $(shell uname) # target os platform
ARCH       ?= $(shell arch)  # target arch
BUILDTYPE  ?= Release
SUFFIX     ?= $(ARCH)
OUT        ?= ../out/$(OS).$(SUFFIX).$(BUILDTYPE)/test2/out
CXX        ?= g++
LINK       ?= g++
NAME       ?= test2
TARGET     ?= $(OUT)/$(NAME)
FUNC       ?= sys

ifeq ($(BUILDTYPE),Release)
	DEBUG=1
endif

ifeq ($(OS),Darwin)
	OS = osx
endif

ifeq ($(OS),Linux)
	LINUX = 1
endif

# is mac osx
ifeq ($(OS),osx)
	OSX = 1
endif

# --------------------------------------------------------------------

# android
# -ffunction-sections -fdata-sections 

INCLUDES	 = -I. -I.. -I../out
CFLAGS		 = -Wall -g -O0 '-DDEBUG' '-D_DEBUG'
CXXFLAGS 	 = -std=c++0x -fexceptions -frtti
LINKFLAGS_START =
LINKFLAGS  =
SOURCES = ../out/test2.cc \
					test2-thread.cc \
					test2-sys.cc \
					test2-str.cc \
					test2-list.cc \
					test2-dict.cc \
					test2-math_dot.cc \
					test2-math_InvSqrt.cc \
					../src/util/log.cc \
					../src/util/array.cc \
					../src/util/string.cc \
					../src/util/object.cc \
					../src/util/hash.cc \
					../src/util/codec.cc \
					../src/util/error.cc \
					../src/util/dict.cc \

# ---------------------------- Platform ----------------------------

ifeq ($(LINUX),1)
	INCLUDES += -I../tools/linux/usr/include
	LINKFLAGS_START += -Wl,--whole-archive
	LINKFLAGS += -Wl,--no-whole-archive -lGLESv2 -lEGL -lX11 -pthread -lasound
	SOURCES +=	test2-alsa.cc \
							test2-alsa2.cc \
							test2-xim.cc \
							test2-x11.cc \
							test2-xopen.cc
endif

ifeq ($(OSX),1)
	LINKFLAGS += -arch $(ARCH) \
		-isysroot $(SYSROOT) \
		-all_load \
		-fobjc-arc \
		-fobjc-link-runtime \
		-stdlib=libc++ \
		-framework AppKit -framework OpenGL -framework CoreVideo
	SOURCES += test2-opengl.mm
endif

# --------------------------------------------------------------------

OBJS = $(foreach file,$(SOURCES),$(OUT)/$(file).o)

.SECONDEXPANSION:

.PHONY: all build cfg

all: build
	@$(TARGET)

build: cfg
	@make -f test2.mk $(TARGET)

cfg:
	@if [ ! -f ../out/test2_cfg.h ] || [ "`grep test2_$(FUNC) ../out/test2_cfg.h`" == "" ]; then \
		echo '#define TEST_FUNC_NAME test2_$(FUNC)' > ../out/test2_cfg.h; \
	fi

../out/test2.cc: test2.cc ../out/test2_cfg.h
	@echo '#include "test2_cfg.h"' > ../out/test2.cc
	@echo '#include "../test/test2.cc"' >> ../out/test2.cc

$(TARGET): $(OBJS)
	$(LINK) -o $(TARGET) $(LINKFLAGS_START) $(OBJS) $(LINKFLAGS)

$(OBJS): $$(subst $$(OUT)/,,$$(basename $$@)) test2.mk
	@mkdir -p $(dir $@)
	$(CXX) $(INCLUDES) $(CFLAGS) $(CXXFLAGS) -MMD -MF $@.d.raw -c -o $@ $(subst $(OUT)/,,$(basename $@))
