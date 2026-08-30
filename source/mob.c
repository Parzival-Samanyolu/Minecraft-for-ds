#include <math.h>
#include "mob.h"
#include "entity.h"

#define TWO_PI 6.28318530f

static uint32_t rngNext(uint32_t *state)
{
	*state = *state * 1103515245u + 12345u;
	return *state;
}

static float rngFloat01(uint32_t *state)
{
	return (float)((rngNext(state) >> 8) & 0xFFFF) / 65536.0f;
}

void mobWorldInit(MobWorld *mw, uint32_t seed)
{
	for (int i = 0; i < MOB_MAX_COUNT; i++)
		mw->mobs[i].active = 0;
	mw->rngState = seed;
	mw->spawnTimer = 0.0f;
}

static void pickNewWanderHeading(Mob *m)
{
	float angle = rngFloat01(&m->rngState) * TWO_PI;
	m->dirX = cosf(angle);
	m->dirZ = sinf(angle);
	m->wanderTimer = MOB_WANDER_MIN_SECONDS +
	                 rngFloat01(&m->rngState) * (MOB_WANDER_MAX_SECONDS - MOB_WANDER_MIN_SECONDS);
}

static void stepOneMob(Mob *m, World *w, Player *p, float dt)
{
	float dx = p->x - m->x;
	float dz = p->z - m->z;
	float dist = sqrtf(dx * dx + dz * dz);

	if (m->kind == MOB_PASSIVE)
		m->state = (dist < MOB_FLEE_RADIUS) ? MOB_STATE_FLEE : MOB_STATE_WANDER;
	else
		m->state = (dist < MOB_AGGRO_RADIUS) ? MOB_STATE_CHASE : MOB_STATE_WANDER;

	float speed;
	switch (m->state) {
	case MOB_STATE_FLEE:
		speed = MOB_FLEE_SPEED;
		if (dist > 0.001f) {
			m->dirX = -dx / dist;
			m->dirZ = -dz / dist;
		}
		break;
	case MOB_STATE_CHASE:
		speed = MOB_CHASE_SPEED;
		if (dist > 0.001f) {
			m->dirX = dx / dist;
			m->dirZ = dz / dist;
		}
		break;
	case MOB_STATE_WANDER:
	default:
		speed = MOB_WANDER_SPEED;
		m->wanderTimer -= dt;
		if (m->wanderTimer <= 0.0f)
			pickNewWanderHeading(m);
		break;
	}

	m->vy -= MOB_GRAVITY * dt;
	if (m->vy < MOB_TERMINAL_VY)
		m->vy = MOB_TERMINAL_VY;

	entityMoveAxis(w, &m->x, &m->y, &m->z, &m->vy, &m->onGround,
	               m->dirX * speed * dt, m->vy * dt, m->dirZ * speed * dt,
	               MOB_HALF_WIDTH, MOB_HEIGHT);

	if (m->attackCooldown > 0.0f)
		m->attackCooldown -= dt;

	if (m->kind == MOB_HOSTILE && m->state == MOB_STATE_CHASE &&
	    dist < MOB_ATTACK_RANGE && m->attackCooldown <= 0.0f) {
		playerApplyDamage(p, MOB_ATTACK_DAMAGE);
		m->attackCooldown = MOB_ATTACK_COOLDOWN;
	}
}

void mobWorldStep(MobWorld *mw, World *w, Player *p, float dt)
{
	for (int i = 0; i < MOB_MAX_COUNT; i++)
		if (mw->mobs[i].active)
			stepOneMob(&mw->mobs[i], w, p, dt);
}

/* scans a loaded chunk column top-down for the first solid block, i.e.
 * where a mob could stand. returns 1 and the spawn feet position (on top
 * of that block, with headroom) on success. */
static int findSurfaceSpawn(const Chunk *c, int lx, int lz, float *outY)
{
	for (int y = CHUNK_SY - 1; y >= 1; y--) {
		BlockId here = chunkGet(c, lx, y, lz);
		if (!blockIsCollidable(here) || here == BLOCK_WATER)
			continue;
		BlockId above1 = chunkGet(c, lx, y + 1, lz);
		BlockId above2 = chunkGet(c, lx, y + 2, lz);
		if (blockIsSolid(above1) || blockIsSolid(above2))
			continue; /* not enough headroom */
		*outY = (float)(y + 1);
		return 1;
	}
	return 0;
}

void mobWorldTrySpawn(MobWorld *mw, World *w, const DayNight *dn, const Player *p, float dt)
{
	mw->spawnTimer += dt;
	if (mw->spawnTimer < MOB_SPAWN_INTERVAL_SECONDS)
		return;
	mw->spawnTimer = 0.0f;

	int slot = -1;
	for (int i = 0; i < MOB_MAX_COUNT; i++)
		if (!mw->mobs[i].active) { slot = i; break; }
	if (slot < 0)
		return;

	for (int attempt = 0; attempt < 5; attempt++) {
		int chunkSlot = (int)(rngNext(&mw->rngState) % WORLD_GRID_SLOTS);
		const WorldSlot *ws = &w->slots[chunkSlot];
		if (!ws->active)
			continue;

		int lx = (int)(rngNext(&mw->rngState) % CHUNK_SX);
		int lz = (int)(rngNext(&mw->rngState) % CHUNK_SZ);
		float spawnY;
		if (!findSurfaceSpawn(&ws->chunk, lx, lz, &spawnY))
			continue;

		float spawnX = (float)(ws->chunk.cx * CHUNK_SX + lx) + 0.5f;
		float spawnZ = (float)(ws->chunk.cz * CHUNK_SZ + lz) + 0.5f;
		float pdx = spawnX - p->x, pdz = spawnZ - p->z;
		if (pdx * pdx + pdz * pdz < MOB_MIN_SPAWN_DIST * MOB_MIN_SPAWN_DIST)
			continue;

		Mob *m = &mw->mobs[slot];
		m->active = 1;
		m->kind = dayNightIsNight(dn) ? MOB_HOSTILE : MOB_PASSIVE;
		m->state = MOB_STATE_WANDER;
		m->x = spawnX;
		m->y = spawnY;
		m->z = spawnZ;
		m->vy = 0.0f;
		m->onGround = 1;
		m->attackCooldown = 0.0f;
		m->rngState = rngNext(&mw->rngState) ^ (uint32_t)slot;
		pickNewWanderHeading(m);
		return;
	}
}
