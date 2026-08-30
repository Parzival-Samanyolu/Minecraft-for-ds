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

/* survival numbers on MC's familiar 0..20 scale, everything else tuned
 * by feel -- there's no food item to eat yet (see DEVELOPMENT.md), so
 * hunger only ever goes down; starvation stops at 1 HP same as vanilla
 * on normal difficulty, meaning the only way to actually die right now
 * is mob damage */
#define PLAYER_MAX_HEALTH 20
#define PLAYER_MAX_HUNGER 20
#define PLAYER_HUNGER_TICK_SECONDS 15.0f
#define PLAYER_REGEN_TICK_SECONDS 4.0f
#define PLAYER_REGEN_MIN_HUNGER 6
#define PLAYER_STARVE_TICK_SECONDS 8.0f
#define PLAYER_STARVE_FLOOR 1

typedef struct {
	float x, y, z;     /* feet position, block units */
	float vy;          /* only vertical velocity is tracked -- horizontal
	                     * movement is direct/instant, no accel or friction yet */
	float yaw, pitch;  /* degrees, look direction */
	int onGround;

	float spawnX, spawnY, spawnZ;
	int health, hunger;
	float hungerTimer, regenTimer, starveTimer;
} Player;

void playerInit(Player *p, float x, float y, float z);

/* moveX/moveZ are world-space (not view-relative) desired horizontal
 * motion for this step, already scaled by dt -- main.c derives them from
 * yaw + held keys. jumpPressed triggers a jump if currently grounded. */
void playerStep(Player *p, World *w, float moveX, float moveZ, int jumpPressed, float dt);

/* hunger drain, regen while well-fed, starvation damage once it hits 0 */
void playerSurvivalStep(Player *p, float dt);

/* applies damage; respawns at the stored spawn point with full stats if it drops to 0 or below */
void playerApplyDamage(Player *p, int amount);

#endif
