#include <nds.h>
#include "render_nds.h"
#include "block.h"
#include "texture_bin.h"

#define TEX_TILE_BYTES (16 * 16 * 2)

static int s_texIds[TEX_COUNT];

void renderInit(void)
{
	vramSetBankA(VRAM_A_TEXTURE);

	glGenTextures(TEX_COUNT, s_texIds);
	for (int i = 0; i < TEX_COUNT; i++) {
		glBindTexture(0, s_texIds[i]);
		glTexImage2D(0, 0, GL_RGB, TEXTURE_SIZE_16, TEXTURE_SIZE_16, 0,
		             GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T,
		             (u8 *)texture_bin + i * TEX_TILE_BYTES);
	}
}

static float faceShade(int axis, int dir)
{
	if (axis == 1)
		return dir ? 1.0f : 0.5f;
	if (axis == 2)
		return 0.8f;
	return 0.6f;
}

static void emitQuadVerts(const MeshQuad *q, float lightMul)
{
	int axis = q->axis;
	int u = (axis + 1) % 3;
	int v = (axis + 2) % 3;
	int base[3] = { q->x, q->y, q->z };
	float shade = faceShade(axis, q->dir) * lightMul;

	glColor3f(shade, shade, shade);

	/* winding doesn't matter, POLY_CULL_NONE -- same corner order for
	 * every face keeps this one function generic across all 6 directions */
	int corners[4][2] = { { 0, 0 }, { q->w, 0 }, { q->w, q->h }, { 0, q->h } };
	for (int i = 0; i < 4; i++) {
		int local[3];
		local[axis] = base[axis];
		local[u] = base[u] + corners[i][0];
		local[v] = base[v] + corners[i][1];

		glTexCoord2f((float)corners[i][0], (float)corners[i][1]);
		glVertex3f(local[0] * BLOCK_UNIT, local[1] * BLOCK_UNIT, local[2] * BLOCK_UNIT);
	}
}

void renderWorld(World *w, float lightMul)
{
	for (int i = 0; i < WORLD_GRID_SLOTS; i++) {
		WorldSlot *slot = &w->slots[i];
		if (!slot->active)
			continue;

		glPushMatrix();
		glTranslatef32(floattof32(slot->chunk.cx * CHUNK_SX * BLOCK_UNIT), 0,
		               floattof32(slot->chunk.cz * CHUNK_SZ * BLOCK_UNIT));

		for (int t = 0; t < TEX_COUNT; t++) {
			int any = 0;
			for (int qi = 0; qi < slot->mesh.count; qi++) {
				if (slot->mesh.quads[qi].tex == t) {
					any = 1;
					break;
				}
			}
			if (!any)
				continue;

			glBindTexture(0, s_texIds[t]);
			glBegin(GL_QUADS);
			for (int qi = 0; qi < slot->mesh.count; qi++) {
				if (slot->mesh.quads[qi].tex == t)
					emitQuadVerts(&slot->mesh.quads[qi], lightMul);
			}
			glEnd();
		}

		glPopMatrix(1);
	}
}
