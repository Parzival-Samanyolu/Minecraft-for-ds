#ifndef ITEM_H
#define ITEM_H

#include "block.h"

#define ITEM_MAX_STACK 64

typedef struct {
	BlockId id;     /* BLOCK_AIR means the slot is empty */
	uint8_t count;
} ItemStack;

#endif
