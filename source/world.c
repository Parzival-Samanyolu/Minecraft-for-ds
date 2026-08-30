#include <string.h>
#include "world.h"
#include "block.h"

static int32_t imod(int32_t a, int32_t b)
{
	int32_t m = a % b;
	if (m < 0)
		m += b;
	return m;
}

static int32_t ifloordiv(int32_t a, int32_t b)
{
	int32_t q = a / b;
	if ((a % b != 0) && ((a < 0) != (b < 0)))
		q--;
	return q;
}

static int slotIndex(int32_t cx, int32_t cz)
{
	int sx = imod(cx, WORLD_GRID_DIM);
	int sz = imod(cz, WORLD_GRID_DIM);
	return sz * WORLD_GRID_DIM + sx;
}

void worldInit(World *w, uint32_t seed)
{
	memset(w, 0, sizeof(*w));
	worldgenSetSeed(seed);
	/* force every slot to regenerate on the first worldUpdate call */
	w->centerCx = 0x7fffffff;
	w->centerCz = 0x7fffffff;
}

int worldUpdate(World *w, int32_t worldX, int32_t worldZ)
{
	int32_t newCx = ifloordiv(worldX, CHUNK_SX);
	int32_t newCz = ifloordiv(worldZ, CHUNK_SZ);

	if (newCx == w->centerCx && newCz == w->centerCz)
		return 0;

	int built = 0;
	for (int gz = -WORLD_RADIUS; gz <= WORLD_RADIUS; gz++) {
		for (int gx = -WORLD_RADIUS; gx <= WORLD_RADIUS; gx++) {
			int32_t cx = newCx + gx;
			int32_t cz = newCz + gz;
			WorldSlot *slot = &w->slots[slotIndex(cx, cz)];

			if (slot->active && slot->chunk.cx == cx && slot->chunk.cz == cz)
				continue;

			slot->chunk.cx = cx;
			slot->chunk.cz = cz;
			worldgenFillChunk(&slot->chunk);
			meshChunkBuild(&slot->chunk, &slot->mesh);
			slot->active = 1;
			built++;
		}
	}

	w->centerCx = newCx;
	w->centerCz = newCz;
	return built;
}

Chunk *worldGetChunkAt(World *w, int32_t cx, int32_t cz)
{
	WorldSlot *slot = &w->slots[slotIndex(cx, cz)];
	if (slot->active && slot->chunk.cx == cx && slot->chunk.cz == cz)
		return &slot->chunk;
	return NULL;
}

int worldIsCollidable(World *w, int32_t x, int32_t y, int32_t z)
{
	if (y < 0 || y >= CHUNK_SY)
		return y < 0; /* solid floor below the world, open sky above it */

	Chunk *c = worldGetChunkAt(w, ifloordiv(x, CHUNK_SX), ifloordiv(z, CHUNK_SZ));
	if (!c)
		return 1;

	BlockId b = chunkGet(c, imod(x, CHUNK_SX), y, imod(z, CHUNK_SZ));
	return blockIsCollidable(b);
}

BlockId worldGetBlock(World *w, int32_t x, int32_t y, int32_t z)
{
	if (y < 0 || y >= CHUNK_SY)
		return BLOCK_AIR;

	Chunk *c = worldGetChunkAt(w, ifloordiv(x, CHUNK_SX), ifloordiv(z, CHUNK_SZ));
	if (!c)
		return BLOCK_AIR;

	return chunkGet(c, imod(x, CHUNK_SX), y, imod(z, CHUNK_SZ));
}

void worldSetBlock(World *w, int32_t x, int32_t y, int32_t z, BlockId b)
{
	if (y < 0 || y >= CHUNK_SY)
		return;

	int32_t cx = ifloordiv(x, CHUNK_SX);
	int32_t cz = ifloordiv(z, CHUNK_SZ);
	WorldSlot *slot = &w->slots[slotIndex(cx, cz)];
	if (!slot->active || slot->chunk.cx != cx || slot->chunk.cz != cz)
		return;

	chunkSet(&slot->chunk, imod(x, CHUNK_SX), y, imod(z, CHUNK_SZ), b);
	meshChunkBuild(&slot->chunk, &slot->mesh);
}
