#ifndef WORLD_H
#define WORLD_H

#include <stdint.h>
#include "chunk.h"
#include "mesh.h"
#include "worldgen.h"

/* streaming radius in chunks. everything loaded here gets drawn every
 * frame too -- no frustum culling yet, so this doubles as the render
 * distance. host_test.c measured ~3800 tris for a 3x3 window (radius 1)
 * vs ~10400 for 5x5 (radius 2), and the DS's rasterizer budget is roughly
 * 120k tris/sec, i.e. ~4000 tris/frame at 30fps -- so radius 1 fits with
 * a little headroom and radius 2 already doesn't. see DEVELOPMENT.md. */
#define WORLD_RADIUS 1
#define WORLD_GRID_DIM (2 * WORLD_RADIUS + 1)
#define WORLD_GRID_SLOTS (WORLD_GRID_DIM * WORLD_GRID_DIM)

typedef struct {
	Chunk chunk;
	ChunkMesh mesh;
	int active;
} WorldSlot;

typedef struct {
	WorldSlot slots[WORLD_GRID_SLOTS];
	int32_t centerCx, centerCz;
} World;

void worldInit(World *w, uint32_t seed);

/* re-centers streaming on the chunk containing (worldX, worldZ) in block
 * units, (re)generating any slot whose toroidal identity changed. returns
 * how many chunks were (re)built this call, so callers can budget how much
 * worldgen+meshing work to spread across frames later on. */
int worldUpdate(World *w, int32_t worldX, int32_t worldZ);

Chunk *worldGetChunkAt(World *w, int32_t cx, int32_t cz);

/* block queries/edits in world (block) coordinates, not chunk-local.
 * a chunk outside the streamed window counts as collidable -- that's a
 * deliberate "world edge is a wall, not a cliff into the void" choice,
 * see DEVELOPMENT.md */
int worldIsCollidable(World *w, int32_t x, int32_t y, int32_t z);
BlockId worldGetBlock(World *w, int32_t x, int32_t y, int32_t z);

/* no-op if the target chunk isn't currently loaded; otherwise edits the
 * block and immediately rebuilds that chunk's mesh (not its neighbors'
 * -- see DEVELOPMENT.md for the seam-artifact tradeoff this implies) */
void worldSetBlock(World *w, int32_t x, int32_t y, int32_t z, BlockId b);

#endif
