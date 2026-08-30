#ifndef MOB_H
#define MOB_H

#include "world.h"
#include "player.h"
#include "daynight.h"

typedef enum {
	MOB_PASSIVE = 0, /* wanders, flees the player */
	MOB_HOSTILE,     /* wanders, chases and attacks the player */
} MobKind;

typedef enum {
	MOB_STATE_WANDER = 0,
	MOB_STATE_FLEE,
	MOB_STATE_CHASE,
} MobState;

#define MOB_MAX_COUNT 6
#define MOB_HALF_WIDTH 0.3f
#define MOB_HEIGHT 0.9f

#define MOB_WANDER_SPEED 1.2f   /* blocks/sec */
#define MOB_FLEE_SPEED 2.5f
#define MOB_CHASE_SPEED 2.0f
#define MOB_GRAVITY 22.0f
#define MOB_TERMINAL_VY -30.0f

#define MOB_FLEE_RADIUS 5.0f
#define MOB_AGGRO_RADIUS 8.0f
#define MOB_ATTACK_RANGE 1.0f
#define MOB_ATTACK_DAMAGE 2
#define MOB_ATTACK_COOLDOWN 1.0f  /* seconds between hits while in range */

#define MOB_WANDER_MIN_SECONDS 1.5f
#define MOB_WANDER_MAX_SECONDS 4.0f

#define MOB_SPAWN_INTERVAL_SECONDS 5.0f
#define MOB_MIN_SPAWN_DIST 3.0f

typedef struct {
	int active;
	MobKind kind;
	MobState state;
	float x, y, z;
	float vy;
	int onGround;
	float dirX, dirZ;      /* current horizontal heading, unit-ish vector */
	float wanderTimer;     /* seconds left before picking a new wander heading */
	float attackCooldown;
	uint32_t rngState;
} Mob;

typedef struct {
	Mob mobs[MOB_MAX_COUNT];
	uint32_t rngState;
	float spawnTimer;
} MobWorld;

void mobWorldInit(MobWorld *mw, uint32_t seed);

/* AI + physics for every active mob, one frame's worth */
void mobWorldStep(MobWorld *mw, World *w, Player *p, float dt);

/* attempts to spawn a passive mob during the day / hostile at night, at
 * most one per call, rate-limited internally -- no-ops if the pool is
 * full or no valid ground is found near the player within a few tries */
void mobWorldTrySpawn(MobWorld *mw, World *w, const DayNight *dn, const Player *p, float dt);

#endif
