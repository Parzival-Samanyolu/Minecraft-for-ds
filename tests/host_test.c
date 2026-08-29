/* host-side sanity/perf check for the platform-independent voxel core --
 * built with the system gcc (see tests/build.sh), nothing here touches
 * libnds so it can run right on the dev machine instead of only finding
 * out on real hardware whether the mesher is even correct */
#include <stdio.h>
#include <string.h>
#include "chunk.h"
#include "block.h"
#include "noise.h"
#include "worldgen.h"
#include "mesh.h"
#include "world.h"
#include "render_nds.h"

/* mirrors emitQuadVerts()'s corner math in source/render_nds.c -- checked
 * here too since v16 (the NDS's vertex fixed-point format) only holds
 * about +-8.0 per axis and there's no way to compile-check that against
 * real libnds headers from this sandbox */
static float worstCaseLocalCoord(const ChunkMesh *m)
{
	float worst = 0.0f;
	for (int i = 0; i < m->count; i++) {
		const MeshQuad *q = &m->quads[i];
		int axis = q->axis;
		int u = (axis + 1) % 3;
		int v = (axis + 2) % 3;
		int base[3] = { q->x, q->y, q->z };
		int corners[4][2] = { { 0, 0 }, { q->w, 0 }, { q->w, q->h }, { 0, q->h } };
		for (int c = 0; c < 4; c++) {
			int local[3];
			local[axis] = base[axis];
			local[u] = base[u] + corners[c][0];
			local[v] = base[v] + corners[c][1];
			for (int k = 0; k < 3; k++) {
				float scaled = local[k] * BLOCK_UNIT;
				if (scaled > worst)
					worst = scaled;
			}
		}
	}
	return worst;
}

static int countExposedFacesNaive(const Chunk *c)
{
	int faces = 0;
	for (int x = 0; x < CHUNK_SX; x++) {
		for (int y = 0; y < CHUNK_SY; y++) {
			for (int z = 0; z < CHUNK_SZ; z++) {
				BlockId here = chunkGet(c, x, y, z);
				if (!blockIsOpaque(here))
					continue;
				static const int off[6][3] = {
					{ 1, 0, 0 }, { -1, 0, 0 }, { 0, 1, 0 },
					{ 0, -1, 0 }, { 0, 0, 1 }, { 0, 0, -1 },
				};
				for (int f = 0; f < 6; f++) {
					BlockId n = chunkGet(c, x + off[f][0], y + off[f][1], z + off[f][2]);
					if (!blockIsOpaque(n))
						faces++;
				}
			}
		}
	}
	return faces;
}

int main(void)
{
	printf("=== struct sizes ===\n");
	printf("Chunk        %zu bytes\n", sizeof(Chunk));
	printf("ChunkMesh    %zu bytes (cap %d quads)\n", sizeof(ChunkMesh), MESH_MAX_QUADS);
	printf("WorldSlot    %zu bytes\n", sizeof(WorldSlot));
	printf("World        %zu bytes total (%d slots, radius %d)\n",
	       sizeof(World), WORLD_GRID_SLOTS, WORLD_RADIUS);

	World world;
	worldInit(&world, 1337);

	printf("\n=== initial load at (0,0) ===\n");
	int built = worldUpdate(&world, 0, 0);
	printf("chunks built: %d (expected %d)\n", built, WORLD_GRID_SLOTS);

	long totalQuads = 0, totalNaiveFaces = 0;
	int maxQuads = 0, overflowedChunks = 0;
	int minHeight = 999, maxHeightSeen = -999;
	float worstLocalCoord = 0.0f;

	for (int i = 0; i < WORLD_GRID_SLOTS; i++) {
		WorldSlot *s = &world.slots[i];
		if (!s->active)
			continue;
		totalQuads += s->mesh.count;
		if (s->mesh.count > maxQuads)
			maxQuads = s->mesh.count;
		if (s->mesh.overflowed)
			overflowedChunks++;
		totalNaiveFaces += countExposedFacesNaive(&s->chunk);

		float w = worstCaseLocalCoord(&s->mesh);
		if (w > worstLocalCoord)
			worstLocalCoord = w;

		for (int x = 0; x < CHUNK_SX; x += 4) {
			for (int z = 0; z < CHUNK_SZ; z += 4) {
				int h = worldgenColumnHeight(s->chunk.cx * CHUNK_SX + x, s->chunk.cz * CHUNK_SZ + z);
				if (h < minHeight) minHeight = h;
				if (h > maxHeightSeen) maxHeightSeen = h;
			}
		}
	}

	printf("\n=== mesh stats across %d loaded chunks ===\n", WORLD_GRID_SLOTS);
	printf("terrain height range sampled: %d..%d (world %d..%d, sea level %d)\n",
	       minHeight, maxHeightSeen, WORLDGEN_MIN_HEIGHT, WORLDGEN_MAX_HEIGHT, WORLDGEN_SEA_LEVEL);
	printf("naive exposed faces (no merge): %ld  (%ld tris)\n", totalNaiveFaces, totalNaiveFaces * 2);
	printf("greedy-merged quads:            %ld  (%ld tris)\n", totalQuads, totalQuads * 2);
	printf("merge ratio: %.1fx fewer quads than naive faces\n",
	       totalNaiveFaces > 0 ? (double)totalNaiveFaces / (double)totalQuads : 0.0);
	printf("max quads in a single chunk: %d / %d cap\n", maxQuads, MESH_MAX_QUADS);
	printf("chunks that overflowed the quad cap: %d\n", overflowedChunks);
	printf("worst-case |local vertex coord| * BLOCK_UNIT: %.3f (v16 limit is ~7.9997)\n", worstLocalCoord);

	long worldMemBytes = (long)sizeof(World);
	printf("\nresident world memory: %ld bytes (%.1f KB) for a %dx%d chunk window\n",
	       worldMemBytes, worldMemBytes / 1024.0, WORLD_GRID_DIM, WORLD_GRID_DIM);

	printf("\n=== streaming as the camera walks across chunk borders ===\n");
	int32_t walk[][2] = { { 16, 0 }, { 32, 0 }, { 200, 0 }, { 200, 16 }, { 200, 200 } };
	for (size_t i = 0; i < sizeof(walk) / sizeof(walk[0]); i++) {
		int b = worldUpdate(&world, walk[i][0], walk[i][1]);
		printf("worldUpdate(%d,%d): %d chunks (re)built\n", walk[i][0], walk[i][1], b);
	}

	if (overflowedChunks > 0) {
		printf("\nFAIL: mesh quad cap was too small for generated terrain\n");
		return 1;
	}
	if (worstLocalCoord >= 7.9f) {
		printf("\nFAIL: a vertex coordinate is too close to the v16 range limit\n");
		return 1;
	}
	printf("\nOK\n");
	return 0;
}
