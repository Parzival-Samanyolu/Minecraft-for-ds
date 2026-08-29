#!/usr/bin/env python3
# regenerates data/texture.bin -- a 16x16 RGB555 placeholder block texture.
# not real Minecraft art (see DEVELOPMENT.md for why), just a dev checker
# pattern so we can see the UVs are mapped right side up on the cube.
import struct

SIZE = 16

def rgb15(r, g, b):
    return (r & 0x1F) | ((g & 0x1F) << 5) | ((b & 0x1F) << 10)

DIRT = rgb15(14, 9, 4)
GRASS = rgb15(6, 18, 5)

def pixel(x, y):
    # 4px checker + a corner accent stripe so a flipped/rotated UV is obvious
    if y < 3:
        return GRASS
    checker = ((x // 4) + (y // 4)) % 2
    base = GRASS if checker else DIRT
    if x == 0 or y == SIZE - 1:
        return rgb15(31, 31, 0)
    return base

with open("data/texture.bin", "wb") as f:
    for y in range(SIZE):
        for x in range(SIZE):
            f.write(struct.pack("<H", pixel(x, y)))
