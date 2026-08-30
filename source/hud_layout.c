#include "hud_layout.h"

#define HOTBAR_Y (192 - HUD_SLOT_SIZE - 4)
#define GRID_Y (HOTBAR_Y - 4 - HUD_GRID_ROWS * HUD_SLOT_SIZE)
#define ROW_X ((256 - HUD_GRID_COLS * HUD_SLOT_SIZE) / 2)

HudRect hudSlotPosition(int slotIndex)
{
	HudRect r;
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

int hudSlotAtPoint(int px, int py)
{
	for (int i = 0; i < INV_TOTAL_SLOTS; i++) {
		HudRect r = hudSlotPosition(i);
		if (px >= r.x && px < r.x + HUD_SLOT_SIZE && py >= r.y && py < r.y + HUD_SLOT_SIZE)
			return i;
	}
	return -1;
}
