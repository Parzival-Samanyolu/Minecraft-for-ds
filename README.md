# Minecraft-for-ds

A cut-down Minecraft-style voxel sandbox for real Nintendo DS hardware
(devkitARM/libnds), scoped to the DS's actual budget: ARM9 @ 67MHz, 4MB RAM,
~656KB VRAM, fixed-function GPU. No redstone, no pathfinding, no
multiplayer.

Status: milestone 1 (toolchain scaffold) -- libnds init, a single textured
rotating cube on the top screen, touch/console on the bottom screen. World
gen, player movement, inventory, crafting, day/night and mobs are not built
yet.

See [DEVELOPMENT.md](DEVELOPMENT.md) for build instructions and the note on
texture assets.
