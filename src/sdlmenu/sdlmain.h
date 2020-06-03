#ifndef __MAIN_h_
#define __MAIN_h_

#define CFG_DIRECTORY ".snes9x4d-ng"

extern uint16 sfc_key[256];
extern short SaveSlotNum;
extern bool8_32 Scale;
#ifdef BILINEAR_SCALE
extern bool8_32 Bilinear;
#endif
extern uint32 MaxAutoFrameSkip;

const char *GetHomeDirectory();
void OutOfMemory();

// Auxiliary functions from Snes9x
extern void S9xDisplayFrameRate(uint8 *, uint32);
extern void S9xDisplayString(const char *string, uint8 *, uint32, int);

#endif // __MAIN_h_
