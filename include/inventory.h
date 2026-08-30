#ifndef INVENTORY_H
#define INVENTORY_H

#include "item.h"

/* slot indices: 0..INV_HOTBAR_SLOTS-1 are the hotbar (also what's shown
 * as the bottom row of the touch HUD), the rest are the backing grid --
 * hud_layout.h maps these same indices to screen positions, so this is
 * the one place the 8/24 split is defined */
#define INV_HOTBAR_SLOTS 8
#define INV_GRID_SLOTS 24
#define INV_TOTAL_SLOTS (INV_HOTBAR_SLOTS + INV_GRID_SLOTS)

typedef struct {
	ItemStack slots[INV_TOTAL_SLOTS];
	int selectedHotbar;
} Inventory;

void inventoryInit(Inventory *inv);

/* stacks into a matching slot first, then the first empty slot, then
 * another empty slot if it still doesn't fit in one stack. returns
 * whatever didn't fit anywhere (there's no ground-item entity to drop it
 * as, so callers just lose it -- fine for now, see DEVELOPMENT.md) */
int inventoryAddItem(Inventory *inv, BlockId id, int count);

/* peeks the currently selected hotbar slot without consuming it */
BlockId inventorySelectedBlock(const Inventory *inv);

/* consumes one item from the selected hotbar slot; returns BLOCK_AIR
 * (and does nothing) if that slot is already empty */
BlockId inventoryTakeSelected(Inventory *inv);

void inventorySwap(Inventory *inv, int slotA, int slotB);

#endif
