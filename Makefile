# derived from "Makefile", 01 June 2010
# blame Skeezix

GIT_VERSION := "$(shell git describe --abbrev=7 --dirty --always --tags)"

UNZIP=1
CHEATS=1

FXOBJ = src/fxinst.o src/fxemu.o src/fxdbg.o
FXDEFINES = -DEXECUTE_SUPERFX_PER_LINE
FXNO_DEPENDS = zsnes_fx

SOUNDOBJ = src/spc700.o src/spc_decode.o src/soundux.o src/apu.o
SOUNDDEFINES = -DSPC700_C

CPUOBJ = src/cpuops.o src/cpuexec.o

C4OBJ = src/c4.o src/c4emu.o
C4DEFINES =
C4NO_DEPENDS = zsnes_c4

ifdef CHEATS
CHEAT = src/cheats.o src/cheats2.o
CHEATDEFINES = -DCHEATS
else
CHEAT =
CHEATDEFINES =
endif

OBJECTS = $(CPUOBJ) $(FXOBJ) $(C4OBJ) $(CHEAT) \
	src/cpu.o src/tile.o src/gfx.o src/clip.o \
	src/memmap.o src/ppu.o src/dma.o \
	src/sdlmenu/sdlmenu.o src/sdlmenu/sdlmain.o src/sdlmenu/sdlaudio.o src/sdlmenu/scaler.o \
	$(SOUNDOBJ) src/sdlmenu/sdlvideo.o \
	src/sdd1.o src/sdd1emu.o src/dsp1.o src/sa1.o src/sa1cpu.o src/obc1.o \
	src/snes9x.o src/snapshot.o src/data.o src/globals.o \

ifdef NETPLAY
OBJECTS += src/netplay.o #src/server.o
NETPLAYDEFINES = -DNETPLAY_SUPPORT
endif

ifdef UNZIP
OBJECTS += src/loadzip.o src/unzip/unzip.o src/unzip/explode.o src/unzip/unreduce.o \
	   src/unzip/unshrink.o
UNZIPDEFINES = -DUNZIP_SUPPORT
endif

TOOLCHAINDIR :=
BINPATH    :=

ARCH :=

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

OPTIMISE = -Ofast -march=armv5te -mtune=arm926ej-s \
			-ffast-math -fomit-frame-pointer -fno-strength-reduce \
			-falign-functions=2 -fno-stack-protector

ifeq ($(PGO), GENERATE)
  OPTIMISE += -fprofile-generate -fprofile-dir=./profile
  PROFILE += -lgcov
else ifeq ($(PGO), APPLY)
  OPTIMISE += -fprofile-use -fprofile-dir=./profile -fbranch-probabilities
endif

CXXFLAGS = $(OPTIMISE) $(PROFILE) $(INCLUDE) \
--std=gnu++98 \
-fno-exceptions -fno-rtti -fno-math-errno -fno-threadsafe-statics \
-D__ARM__ \
-D_ZAURUS \
-D_FAST_GFX \
-D__SDL__ \
-DMIYOO \
-DVIDEO_MODE=1 \
-DBILINEAR_SCALE \
-DZLIB \
-DVAR_CYCLES \
-DCPU_SHUTDOWN \
-DSPC700_SHUTDOWN \
-DBUILD_VERSION=\"$(GIT_VERSION)\" \
$(FXDEFINES) \
$(C4DEFINES) \
$(CPUDEFINES) \
$(SOUNDDEFINES) \
$(NETPLAYDEFINES) \
$(UNZIPDEFINES) \
$(GLIDEDEFINES) \
$(OPENGLDEFINES) \
$(GUIDEFINES) \
$(KREEDDEFINES) \
$(CHEATDEFINES) \
$(SDL_CFLAGS) \

CFLAGS = $(OPTIMISE) $(PROFILE) $(INCLUDE) \
--std=gnu89 \
-D__SDL__ \
-DMIYOO \
-DZLIB \
-DVAR_CYCLES \
-DCPU_SHUTDOWN \
-DSPC700_SHUTDOWN \
-DBUILD_VERSION=\"$(GIT_VERSION)\" \
$(FXDEFINES) \
$(C4DEFINES) \
$(CPUDEFINES) \
$(SOUNDDEFINES) \
$(NETPLAYDEFINES) \
$(UNZIPDEFINES) \
$(GLIDEDEFINES) \
$(OPENGLDEFINES) \
$(GUIDEFINES) \
$(KREEDDEFINES) \
$(CHEATDEFINES) \
$(SDL_CFLAGS) \

LDLIBS  = -lSDL -lz -lm $(SDL_LIBS)

.SUFFIXES: .o .cpp .c .cc .h .m .i .S .asm .obj

all: snes9x4d

snes9x4d: $(OBJECTS)
	$(CXX) -o $@ $(OBJECTS) $(EXTRALIBS) $(LDLIBS) $(PROFILE)
	$(STRIP) snes9x4d

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

.asm.o:
	$(NASM) -f elf $(FXDEFINES) -i . -i i386 -o $@ $*.asm

clean:
	rm -f $(OBJECTS)

release: clean all

src/ppu.o: $(FXDEPENDS)
src/cpu.o: $(FXDEPENDS)
src/memmap.o: $(FXDEPENDS)
src/globals.o: $(FXDEPENDS)
src/snapshot.o: $(FXDEPENDS)
src/snaporig.o: $(FXDEPENDS)

src/cpuexec.o: src/cpuexec.h src/cpuops.h src/snes9x.h src/snapshot.h src/gfx.h \
			src/memmap.h src/ppu.h src/debug.h src/port.h src/display.h src/apu.h src/spc700.h src/apu.h
src/debug.o: src/cpuops.h src/cpuexec.h src/snes9x.h \
			src/memmap.h src/ppu.h src/debug.h src/missing.h src/port.h src/display.h src/apu.h
src/ppu.o: src/snes9x.h src/memmap.h src/ppu.h src/missing.h src/port.h src/cpuexec.h \
			src/apu.h src/spc700.h src/fxemu.h src/fxinst.h src/sdd1.h
src/dsp1.o: src/snes9x.h src/port.h src/dsp1.h
src/sdd1.o: src/snes9x.h src/sdd1.h
src/sdd1emu.o: src/sdd1emu.h
src/sa1.o: src/sa1.h
src/snapshot.o: src/snapshot.h src/memmap.h src/snes9x.h src/65c816.h src/ppu.h \
				src/cpuexec.h src/display.h src/apu.h src/spc700.h src/soundux.h
src/snes96.o: src/port.h src/snes9x.h src/memmap.h
src/memmap.o: src/cpuexec.h src/snes9x.h src/memmap.h src/ppu.h src/port.h src/cheats.h src/getset.h src/apu.h \
			src/spc700.h
src/sdlmenu/sdlmain.o: src/cpuexec.h src/snes9x.h src/port.h src/snapshot.h src/display.h src/apu.h src/gfx.h src/cheats.h src/soundux.h
src/sdlmenu/sdlmenu.o: src/cpuexec.h src/snes9x.h src/port.h src/snapshot.h src/display.h src/apu.h src/gfx.h
src/sdlmenu/sdlvideo.o: src/display.h src/snes9x.h src/memmap.h src/debug.h src/ppu.h src/snapshot.h src/gfx.h src/soundux.h
src/gfx.o: src/memmap.h src/snes9x.h src/ppu.h src/gfx.h src/display.h src/port.h
src/tile.o: src/memmap.h src/snes9x.h src/ppu.h src/display.h src/gfx.h src/tile.h
src/spc700.o: src/spc700.h src/apu.h src/apumem.h src/snes9x.h src/memmap.h
src/apu.o: src/spc700.h src/apu.h src/apumem.h src/snes9x.h src/soundux.h
src/soundux.o: src/snes9x.h src/soundux.h src/apu.h
src/dma.o: src/ppu.h src/dma.h src/memmap.h src/getset.h src/snes9x.h src/port.h src/apu.h src/spc700.h src/sdd1.h src/sdd1emu.h
src/cheats.o: src/cheats.h src/snes9x.h src/port.h src/memmap.h
src/fxemu.o: src/fxemu.h src/fxinst.h
src/fxinst.o: src/fxemu.h src/fxinst.h
src/fxdbg.o: src/fxemu.h src/fxinst.h
src/offsets.o: src/port.h src/snes9x.h src/memmap.h src/ppu.h src/apu.h src/cpuexec.h src/65c816.h
src/globals.o: src/memmap.h src/spc700.h src/apu.h src/cpuexec.h src/ppu.h src/cheats.h src/snes9x.h src/gfx.h \
				src/missing.h src/dma.h src/dsp1.h src/soundux.h
src/xf86.o: src/display.h src/snes9x.h src/memmap.h src/debug.h src/ppu.h src/snapshot.h src/gfx.h
src/server.o: src/snes9x.h src/port.h src/memmap.h src/netplay.h
src/netplay.o: src/snes9x.h src/port.h src/memmap.h src/netplay.h
src/snaporig.o: src/cpuexec.h

