#ifndef NOISE_H
#define NOISE_H

#include <stdint.h>

/* 24.8 fixed point everywhere -- ARM9 has no hardware FPU so this stays
 * pure integer math. picked value noise over Perlin/simplex for the same
 * reason: no per-corner gradient dot products, just a hash + lerp */
typedef int32_t fx_t;
#define FX_SHIFT 8
#define FX_ONE (1 << FX_SHIFT)

uint32_t noiseHash2(int32_t x, int32_t z, uint32_t seed);
uint32_t noiseHash3(int32_t x, int32_t y, int32_t z, uint32_t seed);

/* smoothed value noise on a lattice of the given cell size, result in [0, FX_ONE) */
fx_t valueNoise2D(int32_t x, int32_t z, int32_t cellSize, uint32_t seed);

/* 4-octave fractal sum of valueNoise2D, normalized back to [0, FX_ONE) */
fx_t fbmNoise2D(int32_t x, int32_t z, uint32_t seed);

#endif
