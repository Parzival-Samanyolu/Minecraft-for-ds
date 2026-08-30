# Minecraft-for-ds

A cut-down Minecraft-style voxel sandbox for real Nintendo DS hardware
(devkitARM/libnds), scoped to the DS's actual budget: ARM9 @ 67MHz, 4MB RAM,
~656KB VRAM, fixed-function GPU. No redstone, no pathfinding, no
multiplayer.

Status: milestone 3 (player movement/collision, block break/place) --
streamed 16x32x16 chunks, integer value-noise heightmap terrain,
greedy-meshed rendering, a grounded player (gravity/jump/walk/collision)
and raycast block break/place on the top screen, position/target info on
the bottom screen. Inventory, crafting, day/night and mobs are not built
yet.

See [DEVELOPMENT.md](DEVELOPMENT.md) for build instructions and the note on
texture assets.
