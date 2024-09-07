
BASE       ?= ../..

include $(BASE)/out/config.mk

SYSROOT    ?= /
OS         ?= $(shell uname) # target os platform
ARCH       ?= $(shell arch)  # target arch
BUILDTYPE  ?= Release
SUFFIX     ?= $(ARCH)
OUT        ?= $(BASE)/out/$(OS).$(SUFFIX).$(BUILDTYPE)/test2/out
SRC        ?= $(BASE)/src
DEPS       ?= $(BASE)/deps
CXX        ?= g++
CC         ?= gcc
LINK       ?= g++
NAME       ?= test2
TARGET     ?= $(OUT)/$(NAME)
TEST       ?= sys

ifeq ($(OS),Darwin)
	OS = osx
endif

# --------------------------------------------------------------------

# android
# -ffunction-sections -fdata-sections 

INCLUDES	 = -I. -I$(BASE) -I$(BASE)/out -I$(DEPS)/libtess2/Include
CFLAGS		 = -Wall -g -O0 '-DDEBUG' '-D_DEBUG'
# CXXFLAGS 	 = -std=c++0x -fexceptions -frtti
CXXFLAGS 	 = -std=c++14 -fexceptions -frtti
LINKFLAGS_START =
LINKFLAGS  =
CXX_SOURCES = \
	$(BASE)/out/test2.cc \
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
	$(SRC)/util/log.cc \
	$(SRC)/util/array.cc \
	$(SRC)/util/string.cc \
	$(SRC)/util/object.cc \
	$(SRC)/util/hash.cc \
	$(SRC)/util/codec.cc \
	$(SRC)/util/error.cc \
	$(SRC)/util/dict.cc \
	$(SRC)/util/util.cc \
	$(SRC)/util/numbers.cc \
	$(SRC)/render/bezier.cc \
	$(SRC)/render/math.cc \
	$(SRC)/render/path.cc \
	$(SRC)/render/stroke.cc \
	$(SRC)/render/ft/ft_path.cc \

# deps/tess
C_SOURCES += \
	$(DEPS)/libtess2/Source/bucketalloc.c \
	$(DEPS)/libtess2/Source/dict.c \
	$(DEPS)/libtess2/Source/geom.c \
	$(DEPS)/libtess2/Source/mesh.c \
	$(DEPS)/libtess2/Source/priorityq.c \
	$(DEPS)/libtess2/Source/sweep.c \
	$(DEPS)/libtess2/Source/tess.c \
	$(SRC)/render/ft/ft_math.c \
	$(SRC)/render/ft/ft_stroke.c \

# ---------------------------- Platform ----------------------------

ifeq ($(OS),Linux)
	INCLUDES += -I$(BASE)/tools/linux/usr/include
	LINKFLAGS_START += -Wl,--whole-archive
	LINKFLAGS += -Wl,--no-whole-archive -lGLESv2 -lEGL -lX11 -pthread -lasound
	CXX_SOURCES +=	test2-alsa.cc \
									test2-alsa2.cc \
									test2-xim.cc \
									test2-x11.cc \
									test2-xopen.cc
endif

ifeq ($(OS),osx)
	SYSROOT = $(shell xcrun --show-sdk-path)
	CFLAGS += -arch $(ARCH) -isysroot $(SYSROOT)
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
	@if [ ! -f $(BASE)/out/test2_cfg.h ] || [ "`grep test2_$(TEST) $(BASE)/out/test2_cfg.h`" == "" ]; then \
		echo '#define TEST_FUNC_NAME test2_$(TEST)' > $(BASE)/out/test2_cfg.h; \
	fi

$(BASE)/out/test2.cc: test2.cc $(BASE)/out/test2_cfg.h
	@echo '#include "test2_cfg.h"' > $(BASE)/out/test2.cc
	@echo '#include "../test/2/test2.cc"' >> $(BASE)/out/test2.cc

$(TARGET): $(CXX_OBJS) $(C_OBJS)
	$(LINK) -o $(TARGET) $(LINKFLAGS_START) $(CXX_OBJS) $(C_OBJS) $(LINKFLAGS)

$(CXX_OBJS): $$(subst $$(OUT)/,,$$(basename $$@)) test2.mk
	@mkdir -p $(dir $@)
	$(CXX) $(INCLUDES) $(CFLAGS) $(CXXFLAGS) -MMD -MF $@.d.raw -c -o $@ $(subst $(OUT)/,,$(basename $@))

$(C_OBJS): $$(subst $$(OUT)/,,$$(basename $$@)) test2.mk
	@mkdir -p $(dir $@)
	$(CC) $(INCLUDES) $(CFLAGS) -MMD -MF $@.d.raw -c -o $@ $(subst $(OUT)/,,$(basename $@))
