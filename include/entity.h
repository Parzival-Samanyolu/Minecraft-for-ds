#ifndef ENTITY_H
#define ENTITY_H

#include "world.h"

/* AABB-vs-voxel collision shared by Player (player.c) and Mob (mob.c) --
 * pulled out in milestone 7 once mobs needed the exact same per-axis
 * resolve-and-slide logic player.c already had, parameterized by each
 * entity's own half-width/height instead of a fixed player size. */

int entityBoxCollides(World *w, float x, float y, float z, float halfWidth, float height);

/* each axis resolved against the position the previous axis already
 * settled on -- that's what makes sliding along a wall/floor fall out
 * for free instead of needing separate slide logic. on a downward
 * collision, y snaps to the top of whatever's underfoot rather than
 * wherever the last successful step happened to land. */
void entityMoveAxis(World *w, float *x, float *y, float *z, float *vy, int *onGround,
                     float dx, float dy, float dz, float halfWidth, float height);

#endif
