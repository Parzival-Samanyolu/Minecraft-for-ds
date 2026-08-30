#!/usr/bin/env python3
# regenerates data/texture.bin -- one blob of 16x16 RGB555 placeholder
# block textures, back to back in TEX_* order (see include/block.h).
# not real Minecraft art (see DEVELOPMENT.md for why): these are just
# flat-ish speckled colors so each block type is visually distinct while
# testing the renderer.
import struct

SIZE = 16


def rgb15(r, g, b):
    return (r & 0x1F) | ((g & 0x1F) << 5) | ((b & 0x1F) << 10)


def lcg(seed):
    s = seed & 0xFFFFFFFF
    while True:
        s = (s * 1103515245 + 12345) & 0xFFFFFFFF
        yield s


def speckle(base, variance, seed, x, y):
    rnd = next(lcg(seed * 9781 + x * 131 + y * 733))
    d = (rnd % (variance * 2 + 1)) - variance
    return tuple(max(0, min(31, c + d)) for c in base)


def tile_speckle(base, variance, seed):
    return [[rgb15(*speckle(base, variance, seed, x, y)) for x in range(SIZE)] for y in range(SIZE)]


def tile_grass_top():
    return tile_speckle((6, 18, 5), 2, 1)


def tile_grass_side():
    dirt = tile_speckle((14, 9, 4), 2, 2)
    for y in range(4):
        for x in range(SIZE):
            dirt[y][x] = rgb15(*speckle((6, 18, 5), 2, 1, x, y))
    return dirt


def tile_dirt():
    return tile_speckle((14, 9, 4), 2, 3)


def tile_stone():
    return tile_speckle((16, 16, 16), 2, 4)


def tile_sand():
    return tile_speckle((28, 26, 18), 1, 5)


def tile_water():
    return tile_speckle((4, 10, 26), 1, 6)


def tile_wood_side():
    px = tile_speckle((13, 8, 4), 1, 7)
    for y in range(SIZE):
        for x in range(SIZE):
            if x % 4 == 0:
                r, g, b = speckle((9, 5, 2), 1, 7, x, y)
                px[y][x] = rgb15(r, g, b)
    return px


def tile_wood_top():
    px = tile_speckle((17, 12, 6), 1, 8)
    cx, cy = SIZE // 2, SIZE // 2
    for y in range(SIZE):
        for x in range(SIZE):
            ring = int(((x - cx) ** 2 + (y - cy) ** 2) ** 0.5) % 3
            if ring == 0:
                r, g, b = speckle((11, 7, 3), 1, 8, x, y)
                px[y][x] = rgb15(r, g, b)
    return px


def tile_leaves():
    return tile_speckle((3, 12, 3), 2, 9)


def tile_coal_ore():
    px = tile_speckle((15, 15, 15), 2, 10)
    for y in range(SIZE):
        for x in range(SIZE):
            if (x * 7 + y * 13) % 17 == 0:
                px[y][x] = rgb15(2, 2, 2)
    return px


def tile_planks():
    px = tile_speckle((22, 16, 9), 1, 11)
    for y in range(SIZE):
        if y % 4 == 0:
            for x in range(SIZE):
                r, g, b = speckle((15, 10, 5), 1, 11, x, y)
                px[y][x] = rgb15(r, g, b)
    return px


def tile_crafting_table():
    px = tile_speckle((18, 13, 7), 1, 12)
    for y in range(SIZE):
        for x in range(SIZE):
            if x == 0 or y == 0 or x == SIZE - 1 or y == SIZE - 1 or x == 8 or y == 8:
                r, g, b = speckle((10, 6, 3), 1, 12, x, y)
                px[y][x] = rgb15(r, g, b)
    return px


def tile_stick():
    px = [[rgb15(0, 0, 0) for _ in range(SIZE)] for _ in range(SIZE)]
    for y in range(SIZE):
        for x in range(SIZE):
            if abs((x - y)) <= 1 and 2 <= x + y <= 26:
                r, g, b = speckle((17, 12, 6), 1, 13, x, y)
                px[y][x] = rgb15(r, g, b)
    return px


TEXTURES = [
    tile_grass_top(),
    tile_grass_side(),
    tile_dirt(),
    tile_stone(),
    tile_sand(),
    tile_water(),
    tile_wood_side(),
    tile_wood_top(),
    tile_leaves(),
    tile_coal_ore(),
    tile_planks(),
    tile_crafting_table(),
    tile_stick(),
]

with open("data/texture.bin", "wb") as f:
    for tex in TEXTURES:
        for row in tex:
            for px in row:
                f.write(struct.pack("<H", px))

print(f"wrote {len(TEXTURES)} textures, {len(TEXTURES) * SIZE * SIZE * 2} bytes")
