#include "player.h"
#include "entity.h"

void playerInit(Player *p, float x, float y, float z)
{
	p->x = x;
	p->y = y;
	p->z = z;
	p->vy = 0.0f;
	p->yaw = 0.0f;
	p->pitch = -15.0f;
	p->onGround = 0;

	p->spawnX = x;
	p->spawnY = y;
	p->spawnZ = z;
	p->health = PLAYER_MAX_HEALTH;
	p->hunger = PLAYER_MAX_HUNGER;
	p->hungerTimer = 0.0f;
	p->regenTimer = 0.0f;
	p->starveTimer = 0.0f;
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

	entityMoveAxis(w, &p->x, &p->y, &p->z, &p->vy, &p->onGround,
	               moveX, p->vy * dt, moveZ, PLAYER_HALF_WIDTH, PLAYER_HEIGHT);
}

void playerSurvivalStep(Player *p, float dt)
{
	p->hungerTimer += dt;
	while (p->hungerTimer >= PLAYER_HUNGER_TICK_SECONDS) {
		p->hungerTimer -= PLAYER_HUNGER_TICK_SECONDS;
		if (p->hunger > 0)
			p->hunger--;
	}

	if (p->hunger > PLAYER_REGEN_MIN_HUNGER && p->health < PLAYER_MAX_HEALTH) {
		p->regenTimer += dt;
		while (p->regenTimer >= PLAYER_REGEN_TICK_SECONDS) {
			p->regenTimer -= PLAYER_REGEN_TICK_SECONDS;
			if (p->health < PLAYER_MAX_HEALTH)
				p->health++;
		}
	} else {
		p->regenTimer = 0.0f;
	}

	if (p->hunger == 0 && p->health > PLAYER_STARVE_FLOOR) {
		p->starveTimer += dt;
		while (p->starveTimer >= PLAYER_STARVE_TICK_SECONDS) {
			p->starveTimer -= PLAYER_STARVE_TICK_SECONDS;
			if (p->health > PLAYER_STARVE_FLOOR)
				p->health--;
		}
	} else {
		p->starveTimer = 0.0f;
	}
}

void playerApplyDamage(Player *p, int amount)
{
	p->health -= amount;
	if (p->health <= 0) {
		p->x = p->spawnX;
		p->y = p->spawnY;
		p->z = p->spawnZ;
		p->vy = 0.0f;
		p->onGround = 0;
		p->health = PLAYER_MAX_HEALTH;
		p->hunger = PLAYER_MAX_HUNGER;
		p->hungerTimer = p->regenTimer = p->starveTimer = 0.0f;
	}
}
