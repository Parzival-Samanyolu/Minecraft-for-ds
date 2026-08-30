#ifndef BLOCK_H
#define BLOCK_H

#include <stdint.h>

typedef uint8_t BlockId;

enum {
	BLOCK_AIR = 0,
	BLOCK_GRASS,
	BLOCK_DIRT,
	BLOCK_STONE,
	BLOCK_SAND,
	BLOCK_WATER,
	BLOCK_WOOD,
	BLOCK_LEAVES,
	BLOCK_COAL_ORE,
	BLOCK_PLANKS,
	BLOCK_CRAFTING_TABLE,
	BLOCK_COUNT,     /* ids below this are real, placeable voxel blocks */

	/* crafting-only items from here on -- never placed in the world, so
	 * they only need an inventory icon, not mesh faces. reusing BlockId
	 * as the one item-id space avoids a parallel enum/type just for this */
	ITEM_STICK = BLOCK_COUNT,
	ITEM_COUNT
};

static inline int blockIsPlaceable(BlockId b)
{
	return b > BLOCK_AIR && b < BLOCK_COUNT;
}

/* indices into the texture blob -- keep in sync with
 * tools/make_placeholder_texture.py and the real atlas whenever that
 * lands (see DEVELOPMENT.md) */
enum {
	TEX_GRASS_TOP = 0,
	TEX_GRASS_SIDE,
	TEX_DIRT,
	TEX_STONE,
	TEX_SAND,
	TEX_WATER,
	TEX_WOOD_SIDE,
	TEX_WOOD_TOP,
	TEX_LEAVES,
	TEX_COAL_ORE,
	TEX_PLANKS,
	TEX_CRAFTING_TABLE,
	TEX_STICK,       /* icon-only, never used as a mesh face */
	TEX_COUNT
};

static inline int blockIsOpaque(BlockId b)
{
	/* water is intentionally excluded here even though it's "solid" for
	 * meshing purposes -- two adjacent water blocks shouldn't get a face
	 * between them, and it'll want real alpha blending later */
	return b != BLOCK_AIR && b != BLOCK_WATER;
}

static inline int blockIsSolid(BlockId b)
{
	return b != BLOCK_AIR;
}

/* same rule as opacity (air and water don't block you either) -- named
 * separately so physics code doesn't read like it's making a meshing call */
static inline int blockIsCollidable(BlockId b)
{
	return blockIsOpaque(b);
}

/* dir: 0 = -axis face (bottom/-x/-z), 1 = +axis face (top/+x/+z)
 * axis: 0=x, 1=y, 2=z */
static inline uint8_t blockFaceTexture(BlockId b, int axis, int dir)
{
	switch (b) {
	case BLOCK_GRASS:
		if (axis == 1) return dir ? TEX_GRASS_TOP : TEX_DIRT;
		return TEX_GRASS_SIDE;
	case BLOCK_DIRT:
		return TEX_DIRT;
	case BLOCK_STONE:
		return TEX_STONE;
	case BLOCK_SAND:
		return TEX_SAND;
	case BLOCK_WATER:
		return TEX_WATER;
	case BLOCK_WOOD:
		if (axis == 1) return TEX_WOOD_TOP;
		return TEX_WOOD_SIDE;
	case BLOCK_LEAVES:
		return TEX_LEAVES;
	case BLOCK_COAL_ORE:
		return TEX_COAL_ORE;
	case BLOCK_PLANKS:
		return TEX_PLANKS;
	case BLOCK_CRAFTING_TABLE:
		return TEX_CRAFTING_TABLE;
	case ITEM_STICK:
		return TEX_STICK; /* not a real face, just what the inventory icon looks up */
	default:
		return TEX_STONE;
	}
}

#endif
