/* host-side sanity/perf check for the platform-independent voxel core --
 * built with the system gcc (see tests/build.sh), nothing here touches
 * libnds so it can run right on the dev machine instead of only finding
 * out on real hardware whether the mesher is even correct */
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "chunk.h"
#include "block.h"
#include "noise.h"
#include "worldgen.h"
#include "mesh.h"
#include "world.h"
#include "render_nds.h"
#include "player.h"
#include "raycast.h"
#include "inventory.h"
#include "hud_layout.h"

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

	int fail = overflowedChunks > 0 || worstLocalCoord >= 7.9f;
	if (overflowedChunks > 0)
		printf("\nFAIL: mesh quad cap was too small for generated terrain\n");
	if (worstLocalCoord >= 7.9f)
		printf("\nFAIL: a vertex coordinate is too close to the v16 range limit\n");

	/* player/raycast/block-edit checks run against the chunk at (0,0),
	 * so they need to happen before the streaming walk below moves the
	 * loaded window away from it */
	printf("\n=== player physics: drop from above the world onto terrain ===\n");
	int dropH = worldgenColumnHeight(8, 8);
	Player pl;
	playerInit(&pl, 8.5f, 40.0f, 8.5f);
	int fellFrames = 0;
	for (; fellFrames < 600 && !pl.onGround; fellFrames++)
		playerStep(&pl, &world, 0.0f, 0.0f, 0, 1.0f / 60.0f);
	printf("terrain height at (8,8): %d\n", dropH);
	printf("fell for %d frames, landed at y=%.3f, onGround=%d\n", fellFrames, pl.y, pl.onGround);
	if (!pl.onGround) {
		printf("FAIL: player never landed (fell through terrain or got stuck airborne)\n");
		fail = 1;
	} else if (fabsf(pl.y - (dropH + 1)) > 0.001f) {
		printf("FAIL: landed at the wrong height (expected ~%d)\n", dropH + 1);
		fail = 1;
	}

	printf("\n=== raycast: look straight down from just above the landing spot ===\n");
	RaycastHit hit = raycastBlocks(&world, pl.x, pl.y + PLAYER_EYE_HEIGHT, pl.z, 0.0f, -1.0f, 0.0f, 8.0f);
	printf("hit=%d block=(%d,%d,%d) place=(%d,%d,%d)\n",
	       hit.hit, hit.bx, hit.by, hit.bz, hit.px, hit.py, hit.pz);
	if (!hit.hit || hit.bx != 8 || hit.by != dropH || hit.bz != 8) {
		printf("FAIL: raycast didn't hit the block the player is standing on\n");
		fail = 1;
	}

	printf("\n=== block break/place round-trip ===\n");
	BlockId before = worldGetBlock(&world, 8, dropH, 8);
	worldSetBlock(&world, 8, dropH, 8, BLOCK_AIR);
	BlockId afterBreak = worldGetBlock(&world, 8, dropH, 8);
	worldSetBlock(&world, 8, dropH, 8, BLOCK_STONE);
	BlockId afterPlace = worldGetBlock(&world, 8, dropH, 8);
	printf("before=%d afterBreak=%d afterPlace=%d\n", before, afterBreak, afterPlace);
	if (afterBreak != BLOCK_AIR || afterPlace != BLOCK_STONE) {
		printf("FAIL: worldSetBlock didn't stick\n");
		fail = 1;
	}

	printf("\n=== wall collision: walk straight into a stone wall ===\n");
	for (int y = dropH + 1; y < dropH + 4; y++)
		worldSetBlock(&world, 11, y, 8, BLOCK_STONE);
	for (int i = 0; i < 300; i++)
		playerStep(&pl, &world, PLAYER_WALK_SPEED / 60.0f, 0.0f, 0, 1.0f / 60.0f);
	printf("stopped at x=%.3f (wall front face is at x=11.0)\n", pl.x);
	if (pl.x >= 11.0f - PLAYER_HALF_WIDTH + 0.01f) {
		printf("FAIL: player tunneled into/through the wall\n");
		fail = 1;
	}

	printf("\n=== inventory: stacking, selection, swap ===\n");
	Inventory inv;
	inventoryInit(&inv);
	int leftover = inventoryAddItem(&inv, BLOCK_STONE, 70);
	int stoneCount = 0, stoneSlots = 0;
	for (int i = 0; i < INV_TOTAL_SLOTS; i++)
		if (inv.slots[i].id == BLOCK_STONE) { stoneCount += inv.slots[i].count; stoneSlots++; }
	printf("added 70 stone: leftover=%d, landed in %d slot(s) totalling %d\n", leftover, stoneSlots, stoneCount);
	if (leftover != 0 || stoneCount != 70 || stoneSlots != 2) {
		printf("FAIL: stacking across the 64-item cap didn't split/land as expected\n");
		fail = 1;
	}

	/* inventoryAddItem fills low slot indices first, so both stone stacks
	 * from above landed in the hotbar (0 and 1) -- place something
	 * deliberate in a grid slot instead of relying on that fill order */
	inv.slots[10].id = BLOCK_WOOD;
	inv.slots[10].count = 5;
	inv.selectedHotbar = 2; /* currently empty */
	inventorySwap(&inv, 2, 10);
	BlockId picked = inventorySelectedBlock(&inv);
	int countBefore = inv.slots[2].count;
	BlockId taken = inventoryTakeSelected(&inv);
	int countAfter = inv.slots[2].count;
	printf("swap grid[10] wood into hotbar[2]: picked=%d before=%d after=%d taken=%d, grid[10] now id=%d\n",
	       picked, countBefore, countAfter, taken, inv.slots[10].id);
	if (taken != BLOCK_WOOD || countAfter != countBefore - 1 || inv.slots[10].id != BLOCK_AIR) {
		printf("FAIL: swap+take didn't move/consume items as expected\n");
		fail = 1;
	}

	printf("\n=== HUD layout: slot hit-testing and non-overlap ===\n");
	int layoutOk = 1;
	for (int i = 0; i < INV_TOTAL_SLOTS; i++) {
		HudRect r = hudSlotPosition(i);
		if (r.x < 0 || r.y < 0 || r.x + HUD_SLOT_SIZE > 256 || r.y + HUD_SLOT_SIZE > 192)
			layoutOk = 0;
		if (hudSlotAtPoint(r.x + HUD_SLOT_SIZE / 2, r.y + HUD_SLOT_SIZE / 2) != i)
			layoutOk = 0;
	}
	for (int i = 0; i < INV_TOTAL_SLOTS && layoutOk; i++) {
		HudRect a = hudSlotPosition(i);
		for (int j = i + 1; j < INV_TOTAL_SLOTS; j++) {
			HudRect b = hudSlotPosition(j);
			int overlapX = a.x < b.x + HUD_SLOT_SIZE && b.x < a.x + HUD_SLOT_SIZE;
			int overlapY = a.y < b.y + HUD_SLOT_SIZE && b.y < a.y + HUD_SLOT_SIZE;
			if (overlapX && overlapY) {
				layoutOk = 0;
				break;
			}
		}
	}
	if (hudSlotAtPoint(HUD_TARGET_X + HUD_SLOT_SIZE / 2, HUD_TARGET_Y + HUD_SLOT_SIZE / 2) != -1)
		layoutOk = 0; /* target preview box should not double as a real inventory slot */
	printf("%d slots checked (on-screen, centers hit-test to themselves, no overlaps): %s\n",
	       INV_TOTAL_SLOTS, layoutOk ? "OK" : "FAIL");
	if (!layoutOk)
		fail = 1;

	printf("\n=== streaming as the camera walks across chunk borders ===\n");
	int32_t walk[][2] = { { 16, 0 }, { 32, 0 }, { 200, 0 }, { 200, 16 }, { 200, 200 } };
	for (size_t i = 0; i < sizeof(walk) / sizeof(walk[0]); i++) {
		int b = worldUpdate(&world, walk[i][0], walk[i][1]);
		printf("worldUpdate(%d,%d): %d chunks (re)built\n", walk[i][0], walk[i][1], b);
	}

	if (fail) {
		printf("\nFAIL\n");
		return 1;
	}
	printf("\nOK\n");
	return 0;
}
