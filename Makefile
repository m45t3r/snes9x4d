GIT_VERSION := "$(shell git describe --abbrev=7 --dirty --always --tags)"

UNZIP = 1
C4_OLD = 1
# CHEATS = 1

FXOBJ = src/fxinst.o src/fxemu.o src/fxdbg.o

SOUNDOBJ = src/spc700.o src/soundux.o src/apu.o
SOUNDDEFINES = -DSPC700_C -DSPC700_SHUTDOWN

CPUOBJ = src/cpuops.o src/cpuexec.o

ifdef C4_OLD
C4OBJ = src/c4_old.o src/c4emu.o
C4DEFINES =
else
C4OBJ = src/c4.o src/c4emu.o
C4DEFINES =
endif

ifdef CHEATS
CHEAT = src/cheats.o src/cheats2.o
CHEATDEFINES = -DCHEATS
else
CHEAT =
CHEATDEFINES =
endif

OBJECTS = $(CPUOBJ) $(FXOBJ) $(C4OBJ) $(CHEAT) \
	src/cpu.o src/tile.o src/gfx.o src/clip.o \
	src/memmap.o src/ppu.o src/dma.o $(SOUNDOBJ) \
	src/sdd1.o src/sdd1emu.o src/dsp1.o src/sa1.o src/sa1cpu.o src/obc1.o \
	src/snes9x.o src/snapshot.o src/data.o src/globals.o \
	src/sdlmenu/sdlmenu.o src/sdlmenu/sdlmain.o src/sdlmenu/sdlaudio.o \
	src/sdlmenu/scaler.o src/sdlmenu/sdlvideo.o \

ifdef UNZIP
OBJECTS += src/loadzip.o src/unzip/unzip.o src/unzip/explode.o src/unzip/unreduce.o \
	   src/unzip/unshrink.o
UNZIPDEFINES = -DUNZIP_SUPPORT
endif

PREFIX  = arm-linux

CXX	= $(PREFIX)-g++
CC	= $(PREFIX)-gcc
STRIP	= $(PREFIX)-strip
AS	= $(PREFIX)-as

SYSROOT := $(shell $(CC) --print-sysroot)
SDL_CFLAGS := $(shell $(SYSROOT)/usr/bin/sdl-config --cflags)
SDL_LIBS := $(shell $(SYSROOT)/usr/bin/sdl-config --libs)

INCLUDE = -I. -Isrc/ -Isrc/unzip

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
  OFLAGS += -fprofile-use -fprofile-dir=./profile -fbranch-probabilities
else ifeq ($(PGO), FORCE-APPLY)
  OFLAGS += -fprofile-use -fprofile-dir=./profile -fbranch-probabilities -Wno-error=coverage-mismatch
else
  OFLAGS += -falign-functions=1 -falign-jumps=1 -falign-loops=1 \
	    -fno-unwind-tables -fno-asynchronous-unwind-tables -fno-unroll-loops \
	    -fmerge-all-constants
endif

CCFLAGS = $(OFLAGS) \
$(C4DEFINES) \
$(CHEATDEFINES) \
$(INCLUDE) \
$(SDL_CFLAGS) \
$(SOUNDDEFINES) \
$(UNZIPDEFINES) \
-DBILINEAR_SCALE \
-DBUILD_VERSION=\"$(GIT_VERSION)\" \
-DFAST_ALIGNED_LSB_WORD_ACCESS \
-DLAGFIX \
-DMIYOO \
-DVIDEO_MODE=1 \
-DZLIB \
-D_ZAURUS \

CXXFLAGS = --std=gnu++14 \
	   -fno-exceptions -fno-rtti -fno-threadsafe-statics \
$(CCFLAGS)

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
