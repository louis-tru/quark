
OS    ?= $(shell uname)
ARCH  ?= $(shell arch)
OUT   := ../out/$(OS).$(ARCH)/test2
CXX   ?= g++
NAME  ?= test2

ifeq ($(OS),Linux)
	LINUX = $(OS)
endif

# -ffunction-sections -fdata-sections 
INCLUDES	= -I. $(if $(LINUX),-I../tools/linux/usr/include)
FLAGS 		= -Wall -g -O0 $(INCLUDES) '-DDEBUG' '-D_DEBUG'
CXXFLAGS 	= -std=c++0x -fexceptions -frtti
LINKFLAGS = -lGLESv2 -lEGL -lX11 -pthread -lasound

SOURCES = test2.cc \
					test2-xim.cc \
					test2-x11.cc \
					test2-thread.cc \
					test2-alsa.cc \
					test2-alsa2.cc \
					test2-xopen2.cc \
					test2-sys.cc \

OBJECTS = $(addsuffix .o,$(basename $(SOURCES)))

.SECONDEXPANSION:

.PHONY: all

flags = $(FLAGS) $(if $(shell sh -c "if [ -f $(basename $@).c ]; then echo ok; fi"),,$(CXXFLAGS))

all: $(OBJECTS)
	@$(CXX) -o $(OUT)/$(NAME) \
	-Wl,--whole-archive $(addprefix $(OUT)/,$(OBJECTS)) -Wl,--no-whole-archive $(LINKFLAGS)
	@echo LINK $(OUT)/$(NAME)

$(OBJECTS): $$(basename $$@).c*
	@mkdir -p $(dir $(OUT)/$@)
	@$(CXX) $(flags) -MMD -MF $(OUT)/$@.d.raw -c -o $(OUT)/$@ $(basename $@).c*
	@echo CXX $(OUT)/$@
