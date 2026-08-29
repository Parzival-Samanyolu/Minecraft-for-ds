#ifndef WORLDGEN_H
#define WORLDGEN_H

#include "chunk.h"

#define WORLDGEN_SEA_LEVEL 10
#define WORLDGEN_MIN_HEIGHT 6
#define WORLDGEN_MAX_HEIGHT 26

void worldgenSetSeed(uint32_t seed);
int worldgenColumnHeight(int32_t worldX, int32_t worldZ);
void worldgenFillChunk(Chunk *c);

#endif
