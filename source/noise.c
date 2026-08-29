#include "noise.h"

/* squirrel-eiserloh-style integer hash -- a few multiply/xorshift rounds,
 * cheap and good enough distribution for terrain, no float involved */
uint32_t noiseHash2(int32_t x, int32_t z, uint32_t seed)
{
	uint32_t h = (uint32_t)x * 374761393u + (uint32_t)z * 668265263u + seed * 2246822519u;
	h = (h ^ (h >> 13)) * 1274126177u;
	h ^= h >> 16;
	return h;
}

uint32_t noiseHash3(int32_t x, int32_t y, int32_t z, uint32_t seed)
{
	uint32_t h = (uint32_t)x * 374761393u + (uint32_t)y * 668265263u +
	             (uint32_t)z * 2147483647u + seed * 2246822519u;
	h = (h ^ (h >> 13)) * 1274126177u;
	h ^= h >> 16;
	return h;
}

static int32_t ifloordiv(int32_t a, int32_t b)
{
	int32_t q = a / b;
	if ((a % b != 0) && ((a < 0) != (b < 0)))
		q--;
	return q;
}

static int32_t imod(int32_t a, int32_t b)
{
	int32_t m = a % b;
	if (m < 0)
		m += b;
	return m;
}

static fx_t latticeFx(int32_t ix, int32_t iz, uint32_t seed)
{
	return (fx_t)((noiseHash2(ix, iz, seed) >> 24) & 0xFF);
}

static fx_t smooth(fx_t t)
{
	fx_t t2 = (t * t) >> FX_SHIFT;
	fx_t t3 = (t2 * t) >> FX_SHIFT;
	return 3 * t2 - 2 * t3;
}

static fx_t lerpFx(fx_t a, fx_t b, fx_t t)
{
	return a + (((b - a) * t) >> FX_SHIFT);
}

fx_t valueNoise2D(int32_t x, int32_t z, int32_t cellSize, uint32_t seed)
{
	int32_t ix0 = ifloordiv(x, cellSize);
	int32_t iz0 = ifloordiv(z, cellSize);
	int32_t ix1 = ix0 + 1;
	int32_t iz1 = iz0 + 1;

	fx_t fx = (imod(x, cellSize) << FX_SHIFT) / cellSize;
	fx_t fz = (imod(z, cellSize) << FX_SHIFT) / cellSize;

	fx_t v00 = latticeFx(ix0, iz0, seed);
	fx_t v10 = latticeFx(ix1, iz0, seed);
	fx_t v01 = latticeFx(ix0, iz1, seed);
	fx_t v11 = latticeFx(ix1, iz1, seed);

	fx_t sx = smooth(fx);
	fx_t sz = smooth(fz);

	fx_t top = lerpFx(v00, v10, sx);
	fx_t bot = lerpFx(v01, v11, sx);
	return lerpFx(top, bot, sz);
}

fx_t fbmNoise2D(int32_t x, int32_t z, uint32_t seed)
{
	fx_t total = 0;
	fx_t ampSum = 0;
	fx_t amp = FX_ONE;
	int32_t cell = 48;

	for (int o = 0; o < 4; o++) {
		total += (valueNoise2D(x, z, cell, seed + (uint32_t)o * 101u) * amp) >> FX_SHIFT;
		ampSum += amp;
		amp >>= 1;
		cell >>= 1;
		if (cell < 3)
			cell = 3;
	}

	return (total << FX_SHIFT) / ampSum;
}
