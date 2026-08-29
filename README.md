# Minecraft-for-ds

A cut-down Minecraft-style voxel sandbox for real Nintendo DS hardware
(devkitARM/libnds), scoped to the DS's actual budget: ARM9 @ 67MHz, 4MB RAM,
~656KB VRAM, fixed-function GPU. No redstone, no pathfinding, no
multiplayer.

Status: milestone 2 (chunked world + terrain + fly camera) -- streamed
16x32x16 chunks, integer value-noise heightmap terrain, greedy-meshed
rendering, D-pad fly camera on the top screen, streaming/debug info on the
bottom screen. Player movement/collision, inventory, crafting, day/night
and mobs are not built yet.

See [DEVELOPMENT.md](DEVELOPMENT.md) for build instructions and the note on
texture assets.
