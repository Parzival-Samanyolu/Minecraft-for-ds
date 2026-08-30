#include "hud_layout.h"

#define HOTBAR_Y (192 - HUD_SLOT_SIZE - 4)
#define GRID_Y (HOTBAR_Y - 4 - HUD_GRID_ROWS * HUD_SLOT_SIZE)
#define ROW_X ((256 - HUD_GRID_COLS * HUD_SLOT_SIZE) / 2)

HudRect hudSlotPosition(int slotIndex)
{
	HudRect r = { 0, 0, HUD_SLOT_SIZE, HUD_SLOT_SIZE };
	if (slotIndex < INV_HOTBAR_SLOTS) {
		r.x = ROW_X + slotIndex * HUD_SLOT_SIZE;
		r.y = HOTBAR_Y;
		return r;
	}

	int gi = slotIndex - INV_HOTBAR_SLOTS;
	int row = gi / HUD_GRID_COLS;
	int col = gi % HUD_GRID_COLS;
	r.x = ROW_X + col * HUD_SLOT_SIZE;
	r.y = GRID_Y + row * HUD_SLOT_SIZE;
	return r;
}

HudRect hudCraftCellPosition(int cellIndex)
{
	HudRect r = { 0, 0, HUD_SLOT_SIZE, HUD_SLOT_SIZE };
	int row = cellIndex / HUD_CRAFT_COLS;
	int col = cellIndex % HUD_CRAFT_COLS;
	r.x = HUD_CRAFT_X + col * HUD_SLOT_SIZE;
	r.y = HUD_CRAFT_Y + row * HUD_SLOT_SIZE;
	return r;
}

HudRect hudCraftOutputPosition(void)
{
	HudRect r = { HUD_CRAFT_OUTPUT_X, HUD_CRAFT_OUTPUT_Y, HUD_SLOT_SIZE, HUD_SLOT_SIZE };
	return r;
}

HudRect hudCraftTogglePosition(void)
{
	HudRect r = { HUD_CRAFT_TOGGLE_X, HUD_CRAFT_TOGGLE_Y, HUD_SLOT_SIZE, HUD_SLOT_SIZE };
	return r;
}

HudRect hudHealthBarPosition(void)
{
	HudRect r = { HUD_HEALTH_BAR_X, HUD_HEALTH_BAR_Y, HUD_BAR_W, HUD_BAR_H };
	return r;
}

HudRect hudHungerBarPosition(void)
{
	HudRect r = { HUD_HEALTH_BAR_X, HUD_HUNGER_BAR_Y, HUD_BAR_W, HUD_BAR_H };
	return r;
}

static int inRect(int px, int py, HudRect r)
{
	return px >= r.x && px < r.x + r.w && py >= r.y && py < r.y + r.h;
}

HudHit hudHitTest(int px, int py)
{
	HudHit miss = { HUD_HIT_NONE, 0 };

	for (int i = 0; i < INV_TOTAL_SLOTS; i++)
		if (inRect(px, py, hudSlotPosition(i))) {
			HudHit h = { HUD_HIT_INV_SLOT, i };
			return h;
		}

	for (int i = 0; i < CRAFT_GRID_SIZE; i++)
		if (inRect(px, py, hudCraftCellPosition(i))) {
			HudHit h = { HUD_HIT_CRAFT_CELL, i };
			return h;
		}

	if (inRect(px, py, hudCraftOutputPosition())) {
		HudHit h = { HUD_HIT_CRAFT_OUTPUT, 0 };
		return h;
	}

	if (inRect(px, py, hudCraftTogglePosition())) {
		HudHit h = { HUD_HIT_CRAFT_TOGGLE, 0 };
		return h;
	}

	return miss;
}
