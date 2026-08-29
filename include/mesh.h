#ifndef MESH_H
#define MESH_H

#include <stdint.h>
#include "chunk.h"

/* generated terrain merges heavily (large flat runs of the same block+face);
 * tests/host_test.c measured a worst case of ~300 quads/chunk over a decent
 * sample of seeds, so 384 leaves headroom without wasting RAM on 768. will
 * need revisiting once block break/place can carve less mergeable shapes */
#define MESH_MAX_QUADS 384

typedef struct {
	uint8_t x, y, z;   /* origin corner, in block units within the chunk */
	uint8_t w, h;      /* extents along the face's two in-plane axes (u, then v) */
	uint8_t axis;      /* 0=x, 1=y, 2=z -- the axis the face is perpendicular to */
	uint8_t dir;       /* 0 = facing -axis, 1 = facing +axis */
	uint8_t tex;       /* TEX_* id from block.h */
} MeshQuad;

typedef struct {
	MeshQuad quads[MESH_MAX_QUADS];
	int count;
	int overflowed;    /* set if generated geometry exceeded MESH_MAX_QUADS */
} ChunkMesh;

/* builds greedy-merged faces for a chunk in isolation -- faces on the
 * chunk's own boundary are always treated as exposed (neighbor chunks
 * aren't consulted), which draws a few extra quads at chunk seams but
 * never leaves a hole. see DEVELOPMENT.md for why that tradeoff was made. */
void meshChunkBuild(const Chunk *c, ChunkMesh *out);

#endif
