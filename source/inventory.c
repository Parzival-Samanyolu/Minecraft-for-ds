#include <string.h>
#include "inventory.h"

void inventoryInit(Inventory *inv)
{
	memset(inv, 0, sizeof(*inv));
	inv->selectedHotbar = 0;
}

int inventoryAddItem(Inventory *inv, BlockId id, int count)
{
	if (id == BLOCK_AIR || count <= 0)
		return count;

	for (int i = 0; i < INV_TOTAL_SLOTS && count > 0; i++) {
		ItemStack *s = &inv->slots[i];
		if (s->id != id || s->count >= ITEM_MAX_STACK)
			continue;
		int room = ITEM_MAX_STACK - s->count;
		int take = count < room ? count : room;
		s->count += (uint8_t)take;
		count -= take;
	}

	for (int i = 0; i < INV_TOTAL_SLOTS && count > 0; i++) {
		ItemStack *s = &inv->slots[i];
		if (s->count != 0)
			continue;
		int take = count < ITEM_MAX_STACK ? count : ITEM_MAX_STACK;
		s->id = id;
		s->count = (uint8_t)take;
		count -= take;
	}

	return count;
}

BlockId inventorySelectedBlock(const Inventory *inv)
{
	const ItemStack *s = &inv->slots[inv->selectedHotbar];
	return s->count > 0 ? s->id : BLOCK_AIR;
}

BlockId inventoryTakeSelected(Inventory *inv)
{
	ItemStack *s = &inv->slots[inv->selectedHotbar];
	if (s->count == 0)
		return BLOCK_AIR;

	BlockId id = s->id;
	s->count--;
	if (s->count == 0)
		s->id = BLOCK_AIR;
	return id;
}

void inventorySwap(Inventory *inv, int slotA, int slotB)
{
	if (slotA < 0 || slotA >= INV_TOTAL_SLOTS || slotB < 0 || slotB >= INV_TOTAL_SLOTS)
		return;
	if (slotA == slotB)
		return;

	ItemStack tmp = inv->slots[slotA];
	inv->slots[slotA] = inv->slots[slotB];
	inv->slots[slotB] = tmp;
}
