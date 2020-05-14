#ifndef ___VIDEO_h_
#define ___VIDEO_h_

#if VIDEO_MODE == 1
#define SURFACE_WIDTH 320
#define SURFACE_HEIGHT 240
#define UPSCALE_P upscale_256x224_to_320x240
#ifdef BILINEAR_SCALE
#define UPSCALE_P_BILINEAR upscale_256x224_to_320x240_bilinearish
#endif
#elif VIDEO_MODE == 2
#define SURFACE_WIDTH 400
#define SURFACE_HEIGHT 240
#define UPSCALE_P upscale_256x224_to_384x240_for_400x240
#elif VIDEO_MODE == 3
#define SURFACE_WIDTH 480
#define SURFACE_HEIGHT 272
#define UPSCALE_P upscale_256x224_to_384x272_for_480x272
#endif

#endif
