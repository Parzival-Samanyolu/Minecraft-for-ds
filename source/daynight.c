#include "daynight.h"

#define DAY_R 14
#define DAY_G 22
#define DAY_B 31
#define NIGHT_R 1
#define NIGHT_G 1
#define NIGHT_B 5
#define DAY_LIGHT 1.0f
#define NIGHT_LIGHT 0.35f

void dayNightInit(DayNight *dn)
{
	dn->t = 0.0f;
}

void dayNightStep(DayNight *dn, float dt)
{
	dn->t += dt;
	while (dn->t >= DAY_LENGTH_SECONDS)
		dn->t -= DAY_LENGTH_SECONDS;
	while (dn->t < 0.0f)
		dn->t += DAY_LENGTH_SECONDS;
}

float dayNightFraction(const DayNight *dn)
{
	return dn->t / DAY_LENGTH_SECONDS;
}

int dayNightIsNight(const DayNight *dn)
{
	float f = dayNightFraction(dn);
	return f >= DAYNIGHT_NIGHT_START && f < DAYNIGHT_SUNRISE_START;
}

/* 0 = fully night, 1 = fully day -- shared by light level and sky color
 * so they always move together instead of two separate interpolations
 * drifting out of sync */
static float dayAmount(const DayNight *dn)
{
	float f = dayNightFraction(dn);

	if (f < DAYNIGHT_SUNSET_START)
		return 1.0f;
	if (f < DAYNIGHT_NIGHT_START)
		return 1.0f - (f - DAYNIGHT_SUNSET_START) / (DAYNIGHT_NIGHT_START - DAYNIGHT_SUNSET_START);
	if (f < DAYNIGHT_SUNRISE_START)
		return 0.0f;
	return (f - DAYNIGHT_SUNRISE_START) / (1.0f - DAYNIGHT_SUNRISE_START);
}

float dayNightLightLevel(const DayNight *dn)
{
	float d = dayAmount(dn);
	return NIGHT_LIGHT + (DAY_LIGHT - NIGHT_LIGHT) * d;
}

void dayNightSkyColor(const DayNight *dn, int *r, int *g, int *b)
{
	float d = dayAmount(dn);
	*r = NIGHT_R + (int)((DAY_R - NIGHT_R) * d + 0.5f);
	*g = NIGHT_G + (int)((DAY_G - NIGHT_G) * d + 0.5f);
	*b = NIGHT_B + (int)((DAY_B - NIGHT_B) * d + 0.5f);
}
