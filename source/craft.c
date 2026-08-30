#include <string.h>
#include "craft.h"

const Recipe CRAFT_RECIPES[] = {
	{ BLOCK_WOOD, 1, BLOCK_PLANKS, 4 },
	{ BLOCK_PLANKS, 2, ITEM_STICK, 4 },
	{ BLOCK_PLANKS, 4, BLOCK_CRAFTING_TABLE, 1 },
};
const int CRAFT_RECIPE_COUNT = sizeof(CRAFT_RECIPES) / sizeof(CRAFT_RECIPES[0]);

void craftGridInit(CraftGrid *g)
{
	memset(g, 0, sizeof(*g));
}

int craftMatch(const CraftGrid *g)
{
	BlockId ingredient = BLOCK_AIR;
	int filled = 0;

	for (int i = 0; i < CRAFT_GRID_SIZE; i++) {
		if (g->cells[i].count == 0)
			continue;
		if (filled == 0)
			ingredient = g->cells[i].id;
		else if (g->cells[i].id != ingredient)
			return -1; /* mixed ingredients never match */
		filled++;
	}

	if (filled == 0)
		return -1;

	for (int r = 0; r < CRAFT_RECIPE_COUNT; r++)
		if (CRAFT_RECIPES[r].ingredient == ingredient && CRAFT_RECIPES[r].ingredientCount == filled)
			return r;

	return -1;
}

void craftPlaceFromInventory(CraftGrid *g, Inventory *inv, int invSlot)
{
	if (invSlot < 0 || invSlot >= INV_TOTAL_SLOTS)
		return;
	ItemStack *src = &inv->slots[invSlot];
	if (src->count == 0)
		return;

	for (int i = 0; i < CRAFT_GRID_SIZE; i++) {
		if (g->cells[i].count != 0)
			continue;
		g->cells[i].id = src->id;
		g->cells[i].count = 1;
		src->count--;
		if (src->count == 0)
			src->id = BLOCK_AIR;
		return;
	}
}

void craftReturnCell(CraftGrid *g, Inventory *inv, int cell)
{
	if (cell < 0 || cell >= CRAFT_GRID_SIZE || g->cells[cell].count == 0)
		return;
	inventoryAddItem(inv, g->cells[cell].id, g->cells[cell].count);
	g->cells[cell].id = BLOCK_AIR;
	g->cells[cell].count = 0;
}

int craftCollect(CraftGrid *g, Inventory *inv)
{
	int r = craftMatch(g);
	if (r < 0)
		return 0;

	inventoryAddItem(inv, CRAFT_RECIPES[r].output, CRAFT_RECIPES[r].outputCount);
	for (int i = 0; i < CRAFT_GRID_SIZE; i++) {
		g->cells[i].id = BLOCK_AIR;
		g->cells[i].count = 0;
	}
	return 1;
}
