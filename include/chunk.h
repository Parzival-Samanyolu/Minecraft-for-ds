#ifndef CHUNK_H
#define CHUNK_H

#include <stdint.h>
#include "block.h"

/* full world height lives in one chunk -- no vertical stacking (yet).
 * 16x32x16 = 8192 bytes/chunk, see DEVELOPMENT.md for the budget math */
#define CHUNK_SX 16
#define CHUNK_SY 32
#define CHUNK_SZ 16
#define CHUNK_BLOCKS (CHUNK_SX * CHUNK_SY * CHUNK_SZ)

typedef struct {
	int32_t cx, cz;          /* chunk-grid coordinates, not block coordinates */
	BlockId blocks[CHUNK_BLOCKS];
	uint8_t generated;
	uint8_t meshDirty;
} Chunk;

static inline int chunkInBounds(int x, int y, int z)
{
	return x >= 0 && x < CHUNK_SX && y >= 0 && y < CHUNK_SY && z >= 0 && z < CHUNK_SZ;
}

static inline int chunkIndex(int x, int y, int z)
{
	return (y * CHUNK_SZ + z) * CHUNK_SX + x;
}

static inline BlockId chunkGet(const Chunk *c, int x, int y, int z)
{
	if (!chunkInBounds(x, y, z))
		return BLOCK_AIR;
	return c->blocks[chunkIndex(x, y, z)];
}

static inline void chunkSet(Chunk *c, int x, int y, int z, BlockId b)
{
	if (!chunkInBounds(x, y, z))
		return;
	c->blocks[chunkIndex(x, y, z)] = b;
	c->meshDirty = 1;
}

#endif
