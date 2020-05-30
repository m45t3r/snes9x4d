#include <stdint.h>

void upscale_256x224_to_320x240(uint32_t *dst, uint32_t *src, int width);
#ifdef BILINEAR_SCALE
void upscale_256x224_to_320x240_bilinearish(uint32_t *dst, uint32_t *src, int width);
#endif
void upscale_256x224_to_384x240_for_400x240(uint32_t *dst, uint32_t *src, int width);
void upscale_256x224_to_384x272_for_480x272(uint32_t *dst, uint32_t *src, int width);

extern void (*upscale_p)(uint32_t *dst, uint32_t *src, int width);
