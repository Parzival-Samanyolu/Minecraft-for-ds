#ifndef HUD_LAYOUT_H
#define HUD_LAYOUT_H

#include "inventory.h"
#include "craft.h"

/* bottom-screen touch HUD geometry, kept separate from the actual pixel
 * drawing (hud_nds.c) so the slot math can be checked without libnds --
 * 256x192 sub screen, slot indices match inventory.h's hotbar-then-grid
 * order. layout: a small target-block preview top-left, a crafting panel
 * (2x2 grid + output + mode toggle) top-right, and the 3x8 backing grid
 * above the always-visible 8-slot hotbar at the bottom. */
#define HUD_SLOT_SIZE 20
#define HUD_GRID_COLS 8
#define HUD_GRID_ROWS (INV_GRID_SLOTS / HUD_GRID_COLS)

#define HUD_TARGET_X 8
#define HUD_TARGET_Y 8

#define HUD_CRAFT_X 180
#define HUD_CRAFT_Y 40
#define HUD_CRAFT_COLS 2

#define HUD_CRAFT_OUTPUT_X (HUD_CRAFT_X + HUD_CRAFT_COLS * HUD_SLOT_SIZE + 4)
#define HUD_CRAFT_OUTPUT_Y (HUD_CRAFT_Y + HUD_SLOT_SIZE / 2)

#define HUD_CRAFT_TOGGLE_X 228
#define HUD_CRAFT_TOGGLE_Y 8

typedef struct {
	int x, y;
} HudRect;

typedef enum {
	HUD_HIT_NONE = 0,
	HUD_HIT_INV_SLOT,      /* index: 0..INV_TOTAL_SLOTS-1, see inventory.h */
	HUD_HIT_CRAFT_CELL,    /* index: 0..CRAFT_GRID_SIZE-1 */
	HUD_HIT_CRAFT_OUTPUT,  /* index: unused (0) */
	HUD_HIT_CRAFT_TOGGLE,  /* index: unused (0) */
} HudHitKind;

typedef struct {
	HudHitKind kind;
	int index;
} HudHit;

/* slotIndex in [0, INV_TOTAL_SLOTS) -- see inventory.h for the hotbar/grid split */
HudRect hudSlotPosition(int slotIndex);
HudRect hudCraftCellPosition(int cellIndex);
HudRect hudCraftOutputPosition(void);
HudRect hudCraftTogglePosition(void);

HudHit hudHitTest(int px, int py);

#endif
