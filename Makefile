GIT_VERSION := "$(shell git describe --abbrev=7 --dirty --always --tags)"

UNZIP = 1
CHEATS = 1
ARM_ASM = 1
# SPC700_ASM = 1

FXOBJ = src/fxinst.o src/fxemu.o src/fxdbg.o

SOUNDOBJ = src/spc700.o src/soundux.o src/apu.o
SOUNDDEFINES = -DSPC700_C

CPUOBJ = src/cpuops.o src/cpuexec.o

C4OBJ = src/c4.o src/c4emu.o
C4DEFINES =

ifdef CHEATS
CHEAT = src/cheats.o src/cheats2.o
CHEATDEFINES = -DCHEATS
else
CHEAT =
CHEATDEFINES =
endif

ifdef ARM_ASM
ASM = src/arm/spc_decode.o
ASMDEFINES = -D__ARM__
ifdef SPC700_ASM
ASM += src/arm/spc700a.o
ASMDEFINES += -DSPC700_ASM
endif
else
ASM =
ASMDEFINES =
endif

OBJECTS = $(CPUOBJ) $(FXOBJ) $(C4OBJ) $(CHEAT) $(ASM) \
	src/cpu.o src/tile.o src/gfx.o src/clip.o \
	src/memmap.o src/ppu.o src/dma.o $(SOUNDOBJ) \
	src/sdd1.o src/sdd1emu.o src/dsp1.o src/sa1.o src/sa1cpu.o src/obc1.o \
	src/snes9x.o src/snapshot.o src/data.o src/globals.o \
	src/sdlmenu/sdlmenu.o src/sdlmenu/sdlmain.o src/sdlmenu/sdlaudio.o \
	src/sdlmenu/scaler.o src/sdlmenu/sdlvideo.o \

ifdef NETPLAY
OBJECTS += src/netplay.o #src/server.o
NETPLAYDEFINES = -DNETPLAY_SUPPORT
endif

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
GASM	= $(PREFIX)-g++

SYSROOT := $(shell $(CC) --print-sysroot)
SDL_CFLAGS := $(shell $(SYSROOT)/usr/bin/sdl-config --cflags)
SDL_LIBS := $(shell $(SYSROOT)/usr/bin/sdl-config --libs)

INCLUDE = -I. -Isrc/ -Isrc/unzip

LDLIBS  = -lSDL -lz -lm $(SDL_LIBS)

OFLAGS = -Ofast -march=armv5te -mtune=arm926ej-s \
	 -fomit-frame-pointer -fno-strict-aliasing -fno-stack-protector

ifeq ($(PGO), GENERATE)
  OFLAGS += -fprofile-generate -fprofile-dir=./profile
  LDLIBS += -lgcov
else ifeq ($(PGO), APPLY)
  OFLAGS += -fprofile-use -fprofile-dir=./profile -fbranch-probabilities
endif

CCFLAGS = $(OFLAGS) \
$(ASMDEFINES) \
$(C4DEFINES) \
$(CHEATDEFINES) \
$(INCLUDE) \
$(NETPLAYDEFINES) \
$(SDL_CFLAGS) \
$(SOUNDDEFINES) \
$(UNZIPDEFINES) \
-DBILINEAR_SCALE \
-DBUILD_VERSION=\"$(GIT_VERSION)\" \
-DCPU_SHUTDOWN \
-DMIYOO \
-DSPC700_SHUTDOWN \
-DVAR_CYCLES \
-DVIDEO_MODE=1 \
-DZLIB \
-D_FAST_GFX \
-D_ZAURUS \
-D__SDL__ \

CXXFLAGS = --std=gnu++03 \
	   -fno-exceptions -fno-rtti -fno-math-errno -fno-threadsafe-statics \
$(CCFLAGS)

CFLAGS = --std=gnu11 $(CCFLAGS)

.SUFFIXES: .o .cpp .c .cc .h .m .i .S .asm .obj
.PHONY: format

all: snes9x4d

format:
	clang-format -i **/*.{c,cpp,h}

snes9x4d: $(OBJECTS)
	$(CXX) -o $@ $(OBJECTS) $(LDLIBS)
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
