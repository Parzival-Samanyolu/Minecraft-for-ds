#ifndef PLAYER_H
#define PLAYER_H

#include "world.h"

#define PLAYER_HALF_WIDTH 0.3f
#define PLAYER_HEIGHT 1.7f
#define PLAYER_EYE_HEIGHT 1.55f

#define PLAYER_WALK_SPEED 4.0f    /* blocks/sec */
#define PLAYER_JUMP_SPEED 7.0f    /* blocks/sec, ~1.1 block hop */
#define PLAYER_GRAVITY 22.0f      /* blocks/sec^2 */
#define PLAYER_TERMINAL_VY -30.0f

typedef struct {
	float x, y, z;     /* feet position, block units */
	float vy;          /* only vertical velocity is tracked -- horizontal
	                     * movement is direct/instant, no accel or friction yet */
	float yaw, pitch;  /* degrees, look direction */
	int onGround;
} Player;

void playerInit(Player *p, float x, float y, float z);

/* moveX/moveZ are world-space (not view-relative) desired horizontal
 * motion for this step, already scaled by dt -- main.c derives them from
 * yaw + held keys. jumpPressed triggers a jump if currently grounded. */
void playerStep(Player *p, World *w, float moveX, float moveZ, int jumpPressed, float dt);

#endif
