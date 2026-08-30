#include <math.h>
#include "entity.h"

int entityBoxCollides(World *w, float x, float y, float z, float halfWidth, float height)
{
	int x0 = (int)floorf(x - halfWidth);
	int x1 = (int)floorf(x + halfWidth);
	int y0 = (int)floorf(y);
	int y1 = (int)floorf(y + height - 0.01f);
	int z0 = (int)floorf(z - halfWidth);
	int z1 = (int)floorf(z + halfWidth);

	for (int by = y0; by <= y1; by++)
		for (int bz = z0; bz <= z1; bz++)
			for (int bx = x0; bx <= x1; bx++)
				if (worldIsCollidable(w, bx, by, bz))
					return 1;
	return 0;
}

void entityMoveAxis(World *w, float *x, float *y, float *z, float *vy, int *onGround,
                     float dx, float dy, float dz, float halfWidth, float height)
{
	if (dx != 0.0f) {
		float nx = *x + dx;
		if (!entityBoxCollides(w, nx, *y, *z, halfWidth, height))
			*x = nx;
	}
	if (dz != 0.0f) {
		float nz = *z + dz;
		if (!entityBoxCollides(w, *x, *y, nz, halfWidth, height))
			*z = nz;
	}
	if (dy != 0.0f) {
		float ny = *y + dy;
		if (!entityBoxCollides(w, *x, ny, *z, halfWidth, height)) {
			*y = ny;
			*onGround = 0;
		} else if (dy < 0.0f) {
			*y = floorf(ny) + 1.0f;
			*onGround = 1;
			*vy = 0.0f;
		} else {
			*vy = 0.0f;
		}
	}
}
