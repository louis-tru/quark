
include ../out/config.mk

SYSROOT    ?= /
OS         ?= $(shell uname) # target os platform
ARCH       ?= $(shell arch)  # target arch
BUILDTYPE  ?= Release
SUFFIX     ?= $(ARCH)
OUT        ?= ../out/$(OS).$(SUFFIX).$(BUILDTYPE)/test2/out
CXX        ?= g++
CC         ?= gcc
LINK       ?= g++
NAME       ?= test2
TARGET     ?= $(OUT)/$(NAME)
TEST       ?= sys

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

INCLUDES	 = -I. -I.. -I../out -I../deps/libtess2/Include
CFLAGS		 = -Wall -g -O0 '-DDEBUG' '-D_DEBUG'
# CXXFLAGS 	 = -std=c++0x -fexceptions -frtti
CXXFLAGS 	 = -std=c++14 -fexceptions -frtti
LINKFLAGS_START =
LINKFLAGS  =
CXX_SOURCES = \
	../out/test2.cc \
	test2-thread.cc \
	test2-sys.cc \
	test2-str.cc \
	test2-list.cc \
	test2-dict.cc \
	test2-math_dot.cc \
	test2-math_InvSqrt.cc \
	test2-render_path.cc \
	test2-arr.cc \
	test2-cls.cc \
	test2-try.cc \
	../src/util/log.cc \
	../src/util/array.cc \
	../src/util/string.cc \
	../src/util/object.cc \
	../src/util/hash.cc \
	../src/util/codec.cc \
	../src/util/error.cc \
	../src/util/dict.cc \
	../src/util/util.cc \
	../src/util/numbers.cc \
	../src/render/bezier.cc \
	../src/render/math.cc \
	../src/render/path.cc \
	../src/render/stroke.cc \
	../src/render/ft/ft_path.cc \

# deps/tess
C_SOURCES += \
	../deps/libtess2/Source/bucketalloc.c \
	../deps/libtess2/Source/dict.c \
	../deps/libtess2/Source/geom.c \
	../deps/libtess2/Source/mesh.c \
	../deps/libtess2/Source/priorityq.c \
	../deps/libtess2/Source/sweep.c \
	../deps/libtess2/Source/tess.c \
	../src/render/ft/ft_math.c \
	../src/render/ft/ft_stroke.c \

# ---------------------------- Platform ----------------------------

ifeq ($(LINUX),1)
	INCLUDES += -I../tools/linux/usr/include
	LINKFLAGS_START += -Wl,--whole-archive
	LINKFLAGS += -Wl,--no-whole-archive -lGLESv2 -lEGL -lX11 -pthread -lasound
	CXX_SOURCES +=	test2-alsa.cc \
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
	CXX_SOURCES += test2-opengl.mm
endif

# --------------------------------------------------------------------

C_OBJS   = $(foreach file,$(C_SOURCES),$(OUT)/$(file).o)
CXX_OBJS = $(foreach file,$(CXX_SOURCES),$(OUT)/$(file).o)

.SECONDEXPANSION:

.PHONY: all build cfg

all: build
	@echo
	@echo --------------------------------------------------------------------
	@$(TARGET)

build: cfg
	@make -f test2.mk $(TARGET)

cfg:
	@if [ ! -f ../out/test2_cfg.h ] || [ "`grep test2_$(TEST) ../out/test2_cfg.h`" == "" ]; then \
		echo '#define TEST_FUNC_NAME test2_$(TEST)' > ../out/test2_cfg.h; \
	fi

../out/test2.cc: test2.cc ../out/test2_cfg.h
	@echo '#include "test2_cfg.h"' > ../out/test2.cc
	@echo '#include "../test/test2.cc"' >> ../out/test2.cc

$(TARGET): $(CXX_OBJS) $(C_OBJS)
	$(LINK) -o $(TARGET) $(LINKFLAGS_START) $(CXX_OBJS) $(C_OBJS) $(LINKFLAGS)

$(CXX_OBJS): $$(subst $$(OUT)/,,$$(basename $$@)) test2.mk
	@mkdir -p $(dir $@)
	$(CXX) $(INCLUDES) $(CFLAGS) $(CXXFLAGS) -MMD -MF $@.d.raw -c -o $@ $(subst $(OUT)/,,$(basename $@))

$(C_OBJS): $$(subst $$(OUT)/,,$$(basename $$@)) test2.mk
	@mkdir -p $(dir $@)
	$(CC) $(INCLUDES) $(CFLAGS) -MMD -MF $@.d.raw -c -o $@ $(subst $(OUT)/,,$(basename $@))
