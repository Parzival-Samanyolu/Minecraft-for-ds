#ifndef RAYCAST_H
#define RAYCAST_H

#include <stdint.h>
#include "world.h"

typedef struct {
	int hit;
	int32_t bx, by, bz;   /* the block that was hit */
	int32_t px, py, pz;   /* empty cell just before it -- where a placed block would go */
} RaycastHit;

/* small-step march rather than a DDA grid walk -- reach is only a few
 * blocks so the extra steps cost nothing, and it can't misfire on the
 * axis-aligned edge cases a voxel DDA is prone to */
RaycastHit raycastBlocks(World *w, float ox, float oy, float oz,
                          float dx, float dy, float dz, float maxDist);

#endif
