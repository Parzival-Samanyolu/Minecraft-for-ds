#ifndef HUD_NDS_H
#define HUD_NDS_H

#include "inventory.h"
#include "craft.h"
#include "block.h"

void hudInit(void);
void hudRender(const Inventory *inv, const CraftGrid *grid, int craftMode,
                int hasTarget, BlockId targetBlock);

#endif
