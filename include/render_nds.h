#ifndef RENDER_NDS_H
#define RENDER_NDS_H

#include "world.h"
#include "mob.h"

/* NDS 3D vertices are v16 (4.12 fixed): only about -8..+8 per component
 * is representable. a chunk is 32 blocks tall, so blocks can't be drawn
 * at 1 render-unit each -- everything (chunk translation, vertex data,
 * camera position) is scaled by this factor into render space instead.
 * 32 * 0.2 = 6.4, comfortably inside the +-8 limit. see DEVELOPMENT.md. */
#define BLOCK_UNIT 0.2f

void renderInit(void);
void renderWorld(World *w, float lightMul);
void renderMobs(const MobWorld *mw, float lightMul);

#endif
