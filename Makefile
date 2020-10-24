UNZIP = 1
C4_OLD = 1
SRTC = 0
CHEATS = 0
SDLMENU = 1
SETA_DSP = 0
SNAPSHOT_OLD = 0
SPC7110 = 0

include Makefile.common

PREFIX  = arm-linux

CXX	= $(PREFIX)-g++
CC	= $(PREFIX)-gcc
STRIP	= $(PREFIX)-strip
AS	= $(PREFIX)-as

SYSROOT := $(shell $(CC) --print-sysroot)
SDL_CFLAGS := $(shell $(SYSROOT)/usr/bin/sdl-config --cflags)
SDL_LIBS := $(shell $(SYSROOT)/usr/bin/sdl-config --libs)

LDFLAGS = -lz -lm $(SDL_LIBS) -Wl,--as-needed -Wl,--gc-sections -s

OFLAGS = -Ofast -march=armv5te -mtune=arm926ej-s -marm \
	 -flto=4 -fwhole-program -fuse-linker-plugin \
	 -fdata-sections -ffunction-sections \
	 -fno-stack-protector -fomit-frame-pointer \
	 -Wall

ifeq ($(PGO), GENERATE)
  OFLAGS += -fprofile-generate -fprofile-dir=./profile
  LDFLAGS += -lgcov
else ifeq ($(PGO), APPLY)
  OFLAGS += -fprofile-use -fprofile-dir=./profile -fbranch-probabilities -fno-unroll-loops
else ifeq ($(PGO), FORCE-APPLY)
  OFLAGS += -fprofile-use -fprofile-dir=./profile -fbranch-probabilities -Wno-error=coverage-mismatch
else
  OFLAGS += -falign-functions=1 -falign-jumps=1 -falign-loops=1 \
	    -fno-unwind-tables -fno-asynchronous-unwind-tables -fno-unroll-loops \
	    -fmerge-all-constants
endif

CCFLAGS += $(OFLAGS) \
	-DBILINEAR_SCALE \
	-DFAST_ALIGNED_LSB_WORD_ACCESS \
	-DFOREVER_16_BIT \
	-DFOREVER_16_BIT_SOUND \
	-DLAGFIX \
	-DMIYOO \
	-DSNESADVANCE_SPEEDHACKS \
	-DVIDEO_MODE=1 \
	-DZLIB \

CXXFLAGS = --std=gnu++14 $(CCFLAGS) \
	-fno-exceptions -fno-rtti -fno-threadsafe-statics

CFLAGS = --std=gnu11 $(CCFLAGS)

.SUFFIXES: .o .cpp .c .cc .h .m .i .S .asm .obj
.PHONY: format

all: snes9x4d

format:
	find . -regex '.*\.\(c\|h\|cpp\|hpp\|cc\|cxx\)' -exec clang-format -style=file -i {} \;

snes9x4d: $(OBJECTS)
	$(CXX) -o $@ $(OBJECTS) $(LDFLAGS)
	$(STRIP) $@

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $*.cpp -o $@

.c.o:
	$(CC) -c $(CFLAGS) $*.c -o $@

.cpp.S:
	$(CXX) -S $(CXXFLAGS) $*.cpp -o $@

.cpp.i:
	$(CXX) -E $(CXXFLAGS) $*.cpp -o $@

.S.o:
	$(CXX) -c $(CXXFLAGS) $*.S -o $@

.S.i:
	$(CXX) -c -E $(CXXFLAGS) $*.S -o $@

clean:
	rm -f $(OBJECTS)

release: clean all
