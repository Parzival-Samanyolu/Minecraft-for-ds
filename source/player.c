#include <math.h>
#include "player.h"

void playerInit(Player *p, float x, float y, float z)
{
	p->x = x;
	p->y = y;
	p->z = z;
	p->vy = 0.0f;
	p->yaw = 0.0f;
	p->pitch = -15.0f;
	p->onGround = 0;
}

static int boxCollides(World *w, float x, float y, float z)
{
	int x0 = (int)floorf(x - PLAYER_HALF_WIDTH);
	int x1 = (int)floorf(x + PLAYER_HALF_WIDTH);
	int y0 = (int)floorf(y);
	int y1 = (int)floorf(y + PLAYER_HEIGHT - 0.01f);
	int z0 = (int)floorf(z - PLAYER_HALF_WIDTH);
	int z1 = (int)floorf(z + PLAYER_HALF_WIDTH);

	for (int by = y0; by <= y1; by++)
		for (int bz = z0; bz <= z1; bz++)
			for (int bx = x0; bx <= x1; bx++)
				if (worldIsCollidable(w, bx, by, bz))
					return 1;
	return 0;
}

/* each axis resolved against the position the previous axis already
 * settled on -- that's what makes sliding along a wall/floor fall out
 * for free instead of needing separate slide logic */
static void moveAxis(Player *p, World *w, float dx, float dy, float dz)
{
	if (dx != 0.0f) {
		float nx = p->x + dx;
		if (!boxCollides(w, nx, p->y, p->z))
			p->x = nx;
	}
	if (dz != 0.0f) {
		float nz = p->z + dz;
		if (!boxCollides(w, p->x, p->y, nz))
			p->z = nz;
	}
	if (dy != 0.0f) {
		float ny = p->y + dy;
		if (!boxCollides(w, p->x, ny, p->z)) {
			p->y = ny;
			p->onGround = 0;
		} else if (dy < 0.0f) {
			/* snap feet to the top of whatever's underfoot instead of
			 * leaving p->y wherever the last successful step happened to
			 * land -- otherwise landing height is off by up to a frame's
			 * worth of fall speed, which host_test.c caught immediately */
			p->y = floorf(ny) + 1.0f;
			p->onGround = 1;
			p->vy = 0.0f;
		} else {
			p->vy = 0.0f;
		}
	}
}

void playerStep(Player *p, World *w, float moveX, float moveZ, int jumpPressed, float dt)
{
	if (jumpPressed && p->onGround) {
		p->vy = PLAYER_JUMP_SPEED;
		p->onGround = 0;
	}

	p->vy -= PLAYER_GRAVITY * dt;
	if (p->vy < PLAYER_TERMINAL_VY)
		p->vy = PLAYER_TERMINAL_VY;

	moveAxis(p, w, moveX, p->vy * dt, moveZ);
}
