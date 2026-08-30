#include <math.h>
#include "raycast.h"

#define RAYCAST_STEP 0.05f

RaycastHit raycastBlocks(World *w, float ox, float oy, float oz,
                          float dx, float dy, float dz, float maxDist)
{
	RaycastHit result = { 0 };

	int32_t lastBx = (int32_t)floorf(ox);
	int32_t lastBy = (int32_t)floorf(oy);
	int32_t lastBz = (int32_t)floorf(oz);

	int steps = (int)(maxDist / RAYCAST_STEP);
	for (int i = 1; i <= steps; i++) {
		float t = i * RAYCAST_STEP;
		float x = ox + dx * t;
		float y = oy + dy * t;
		float z = oz + dz * t;

		int32_t bx = (int32_t)floorf(x);
		int32_t by = (int32_t)floorf(y);
		int32_t bz = (int32_t)floorf(z);

		if (worldIsCollidable(w, bx, by, bz)) {
			result.hit = 1;
			result.bx = bx;
			result.by = by;
			result.bz = bz;
			result.px = lastBx;
			result.py = lastBy;
			result.pz = lastBz;
			return result;
		}

		lastBx = bx;
		lastBy = by;
		lastBz = bz;
	}

	return result;
}
