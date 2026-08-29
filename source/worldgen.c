#include "worldgen.h"
#include "noise.h"

static uint32_t s_seed = 1337;

void worldgenSetSeed(uint32_t seed)
{
	s_seed = seed;
}

int worldgenColumnHeight(int32_t worldX, int32_t worldZ)
{
	fx_t n = fbmNoise2D(worldX, worldZ, s_seed);
	int range = WORLDGEN_MAX_HEIGHT - WORLDGEN_MIN_HEIGHT;
	return WORLDGEN_MIN_HEIGHT + ((int)n * range) / FX_ONE;
}

void worldgenFillChunk(Chunk *c)
{
	int32_t baseX = c->cx * CHUNK_SX;
	int32_t baseZ = c->cz * CHUNK_SZ;

	for (int x = 0; x < CHUNK_SX; x++) {
		for (int z = 0; z < CHUNK_SZ; z++) {
			int32_t wx = baseX + x;
			int32_t wz = baseZ + z;
			int h = worldgenColumnHeight(wx, wz);
			int beach = h <= WORLDGEN_SEA_LEVEL + 1;

			for (int y = 0; y < CHUNK_SY; y++) {
				BlockId b;
				if (y > h) {
					b = (y <= WORLDGEN_SEA_LEVEL) ? BLOCK_WATER : BLOCK_AIR;
				} else if (y == h) {
					b = beach ? BLOCK_SAND : BLOCK_GRASS;
				} else if (y >= h - 3) {
					b = beach ? BLOCK_SAND : BLOCK_DIRT;
				} else {
					/* sparse coal sprinkle, ~1 in 24 stone blocks below the
					 * dirt layer -- no proper veins/clustering yet */
					b = (noiseHash3(wx, y, wz, s_seed * 7u + 13u) % 24 == 0)
					        ? BLOCK_COAL_ORE
					        : BLOCK_STONE;
				}
				chunkSet(c, x, y, z, b);
			}
		}
	}

	c->generated = 1;
	c->meshDirty = 1;
}
