#ifndef DAYNIGHT_H
#define DAYNIGHT_H

/* a real MC day is 20 real-world minutes -- way too long to ever see
 * change during a handheld play session or a testing pass, so this is
 * deliberately much shorter. tune freely; nothing else depends on the
 * exact number. */
#define DAY_LENGTH_SECONDS 240.0f

/* phase boundaries as a fraction of a full day, day starting at 0 */
#define DAYNIGHT_SUNSET_START 0.40f
#define DAYNIGHT_NIGHT_START 0.50f
#define DAYNIGHT_SUNRISE_START 0.90f

typedef struct {
	float t; /* seconds into the current day, [0, DAY_LENGTH_SECONDS) */
} DayNight;

void dayNightInit(DayNight *dn);
void dayNightStep(DayNight *dn, float dt);

/* fraction of the day elapsed, [0, 1) */
float dayNightFraction(const DayNight *dn);

/* mobs will care about this once they exist (milestone 7) */
int dayNightIsNight(const DayNight *dn);

/* 0..1 global brightness multiplier, applied on top of each mesh face's
 * own fixed shade (see render_nds.c) */
float dayNightLightLevel(const DayNight *dn);

/* sky/clear color for the current moment, each component 0..31 */
void dayNightSkyColor(const DayNight *dn, int *r, int *g, int *b);

#endif
