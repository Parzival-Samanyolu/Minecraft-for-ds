#ifndef CRAFT_H
#define CRAFT_H

#include "inventory.h"

#define CRAFT_GRID_SIZE 4  /* a 2x2 grid -- arrangement doesn't matter, only counts do */

typedef struct {
	BlockId ingredient;
	int ingredientCount;  /* how many of the 4 cells must hold it -- exact, no leftover */
	BlockId output;
	int outputCount;
} Recipe;

extern const Recipe CRAFT_RECIPES[];
extern const int CRAFT_RECIPE_COUNT;

typedef struct {
	ItemStack cells[CRAFT_GRID_SIZE];
} CraftGrid;

void craftGridInit(CraftGrid *g);

/* index into CRAFT_RECIPES that matches the grid's current contents, or -1.
 * shapeless and strict: every filled cell must hold the same ingredient,
 * and the filled count must exactly equal the recipe's requirement. */
int craftMatch(const CraftGrid *g);

/* moves 1 item from inventory slot invSlot into the first empty grid cell.
 * no-op if that inventory slot is empty or the grid has no room. */
void craftPlaceFromInventory(CraftGrid *g, Inventory *inv, int invSlot);

/* returns the item in grid cell `cell` back to the inventory, clearing the cell. */
void craftReturnCell(CraftGrid *g, Inventory *inv, int cell);

/* if craftMatch() succeeds: adds the recipe's output to the inventory and
 * clears every cell that was consumed. returns 1 if a craft happened. */
int craftCollect(CraftGrid *g, Inventory *inv);

#endif
