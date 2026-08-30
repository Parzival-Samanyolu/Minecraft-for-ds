#ifndef HUD_LAYOUT_H
#define HUD_LAYOUT_H

#include "inventory.h"

/* bottom-screen touch HUD geometry, kept separate from the actual pixel
 * drawing (hud_nds.c) so the slot math can be checked without libnds --
 * 256x192 sub screen, slot indices match inventory.h's hotbar-then-grid
 * order. layout: a small target-block preview top-left, the 3x8 backing
 * grid above the always-visible 8-slot hotbar at the bottom. */
#define HUD_SLOT_SIZE 20
#define HUD_GRID_COLS 8
#define HUD_GRID_ROWS (INV_GRID_SLOTS / HUD_GRID_COLS)

#define HUD_TARGET_X 8
#define HUD_TARGET_Y 8

typedef struct {
	int x, y;
} HudRect;

/* slotIndex in [0, INV_TOTAL_SLOTS) -- see inventory.h for the hotbar/grid split */
HudRect hudSlotPosition(int slotIndex);

/* returns the slot index under (px,py), or -1 if the touch missed every slot */
int hudSlotAtPoint(int px, int py);

#endif
