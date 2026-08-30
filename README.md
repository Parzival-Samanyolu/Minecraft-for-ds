# Minecraft-for-ds

A cut-down Minecraft-style voxel sandbox for real Nintendo DS hardware
(devkitARM/libnds), scoped to the DS's actual budget: ARM9 @ 67MHz, 4MB RAM,
~656KB VRAM, fixed-function GPU. No redstone, no pathfinding, no
multiplayer.

Status: milestone 6 (day/night cycle + lighting) -- streamed 16x32x16
chunks, integer value-noise heightmap terrain, greedy-meshed rendering
with a day/night cycle driving sky color and global lighting, a grounded
player (gravity/jump/walk/collision) with raycast block break/place on
the top screen, and a touch-driven hotbar + inventory grid + 2x2 crafting
panel on the bottom screen. Mobs/survival stats are not built yet.

See [DEVELOPMENT.md](DEVELOPMENT.md) for build instructions and the note on
texture assets.
