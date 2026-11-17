# Minecraft-for-ds

A cut-down Minecraft-style voxel sandbox for real Nintendo DS hardware
(BlocksDS/libnds), scoped to the DS's actual budget: ARM9 @ 67MHz, 4MB RAM,
~656KB VRAM, fixed-function GPU. No redstone, no pathfinding, no
multiplayer.

Status: milestone 7, all seven brief milestones built -- streamed
16x32x16 chunks with integer value-noise terrain and greedy-meshed
rendering; a grounded player (gravity/jump/walk/collision) with raycast
block break/place; a touch-driven hotbar + inventory grid + 2x2 crafting
panel; a day/night cycle driving sky color and lighting; and passive/
hostile mobs (wander/flee/chase, no pathfinding) plus health/hunger
survival stats. See DEVELOPMENT.md's "Project status" section for what's
left, which is mostly things that need a real device or emulator to
judge, not more code.

See [DEVELOPMENT.md](DEVELOPMENT.md) for build instructions and the note on
texture assets.
