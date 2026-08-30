#ifndef HUD_NDS_H
#define HUD_NDS_H

#include "inventory.h"
#include "block.h"

void hudInit(void);
void hudRender(const Inventory *inv, int hasTarget, BlockId targetBlock);

#endif
