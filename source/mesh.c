#include <string.h>
#include "mesh.h"
#include "block.h"

static BlockId sampleClamped(const Chunk *c, int x, int y, int z)
{
	if (!chunkInBounds(x, y, z))
		return BLOCK_AIR;
	return chunkGet(c, x, y, z);
}

static void emitQuad(ChunkMesh *out, int axis, int dir, int origin[3], int w, int h, uint8_t tex)
{
	if (out->count >= MESH_MAX_QUADS) {
		out->overflowed = 1;
		return;
	}
	MeshQuad *q = &out->quads[out->count++];
	q->x = (uint8_t)origin[0];
	q->y = (uint8_t)origin[1];
	q->z = (uint8_t)origin[2];
	q->w = (uint8_t)w;
	q->h = (uint8_t)h;
	q->axis = (uint8_t)axis;
	q->dir = (uint8_t)dir;
	q->tex = tex;
}

void meshChunkBuild(const Chunk *c, ChunkMesh *out)
{
	out->count = 0;
	out->overflowed = 0;

	const int dims[3] = { CHUNK_SX, CHUNK_SY, CHUNK_SZ };
	static uint8_t mask[CHUNK_SY * CHUNK_SZ]; /* big enough for any dims[u]*dims[v] pair here */

	for (int d = 0; d < 3; d++) {
		int u = (d + 1) % 3;
		int v = (d + 2) % 3;
		int du = dims[u];
		int dv = dims[v];

		for (int dir = 0; dir < 2; dir++) {
			int nrm = dir ? 1 : -1;

			for (int layer = 0; layer < dims[d]; layer++) {
				memset(mask, 0, (size_t)(du * dv));

				int pos[3];
				pos[d] = layer;
				for (int iv = 0; iv < dv; iv++) {
					pos[v] = iv;
					for (int iu = 0; iu < du; iu++) {
						pos[u] = iu;
						BlockId here = sampleClamped(c, pos[0], pos[1], pos[2]);
						if (!blockIsOpaque(here))
							continue;

						int npos[3] = { pos[0], pos[1], pos[2] };
						npos[d] += nrm;
						BlockId neighbor = sampleClamped(c, npos[0], npos[1], npos[2]);
						if (blockIsOpaque(neighbor))
							continue;

						mask[iv * du + iu] = (uint8_t)(blockFaceTexture(here, d, dir) + 1);
					}
				}

				for (int iv = 0; iv < dv; iv++) {
					for (int iu = 0; iu < du; iu++) {
						uint8_t m = mask[iv * du + iu];
						if (!m)
							continue;

						int w = 1;
						while (iu + w < du && mask[iv * du + iu + w] == m)
							w++;

						int h = 1;
						int stop = 0;
						while (iv + h < dv && !stop) {
							for (int xx = 0; xx < w; xx++) {
								if (mask[(iv + h) * du + iu + xx] != m) {
									stop = 1;
									break;
								}
							}
							if (!stop)
								h++;
						}

						for (int yy = 0; yy < h; yy++)
							for (int xx = 0; xx < w; xx++)
								mask[(iv + yy) * du + iu + xx] = 0;

						int origin[3];
						origin[d] = layer + (dir ? 1 : 0);
						origin[u] = iu;
						origin[v] = iv;

						emitQuad(out, d, dir, origin, w, h, (uint8_t)(m - 1));
					}
				}
			}
		}
	}
}
