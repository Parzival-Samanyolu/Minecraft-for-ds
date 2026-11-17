# Building

BlocksDS project (https://blocksds.skylyrac.net). With BlocksDS installed
(or inside the `skylyrac/blocksds:slim-latest` Docker image):

```
make
```

produces `minecraft-for-ds.nds`. `make clean` to reset.

CI builds the ROM on every push via `.github/workflows/build.yml` using that
Docker image and uploads it as a workflow artifact.

Toolchain note: this originally built with devkitARM/libnds 2.x. That ROM
only ever showed two white screens in Delta, and in DeSmuME both CPUs hang
in the toolchain's own startup handshake before `main()` is reached, so it
was moved to BlocksDS, whose ROMs boot in DeSmuME and older melonDS cores.
The BlocksDS build has been run in DeSmuME (terrain on the top screen, HUD
on the bottom). Real-hardware and Delta behavior (input feel, framerate)
still need checking on a device.

One more thing that bit the first builds: `World` is ~100 KB and must not
be a stack local -- the ARM9 stack is only 16 KB. It is `static` in
`main()` for that reason.

# Texture assets

`data/texture.bin` is 10 hand-rolled 16x16 RGB555 placeholders (speckled
flat colors, not real art), one per `TEX_*` id in `include/block.h`, back
to back. Not Minecraft textures — Mojang's are copyrighted, so this repo
won't ship them. `tools/make_placeholder_texture.py` regenerates the blob.

Each block gets its own small bound GL texture rather than one shared UV
atlas. That's a deliberate change from the milestone-1 plan: greedy meshing
merges same-block runs into one big quad and relies on `GL_TEXTURE_WRAP_S/T`
to tile the texture across it, and hardware wrap only repeats the *whole*
bound texture — it can't repeat just a sub-rectangle of a shared atlas
without sampling into a neighboring tile. Ten 16x16 RGB555 textures is 5KB;
even a full real block palette (a few dozen blocks x a few face variants)
stays well under 1% of the ~656KB VRAM budget, so an atlas's usual VRAM
win doesn't buy anything here — the per-texture-bind approach is both
simpler and correct for greedy-merged geometry. The real atlas (from a
resource pack you own, not committed to the repo, converted to this same
"many small direct-color textures" layout) can drop in later the same way.

# Milestone 2 — chunked world, terrain, greedy meshing, fly camera

New pure-C core (`chunk.h/c`, `noise.h/c`, `worldgen.h/c`, `mesh.h/c`,
`world.h/c`) has no libnds dependency and builds with the host's own gcc —
`tests/build.sh` compiles and runs `tests/host_test.c` against it. That
matters here specifically because this sandbox has no ARM cross toolchain
or hardware/emulator to test against, so anything that can be verified
without libnds gets verified for real rather than eyeballed. NDS-specific
glue (`render_nds.c`, the camera/input loop in `main.c`) still only gets
checked by the CI build.

**Chunk size: 16x32x16 (8192 blocks, 8KB/chunk).** Full world height lives
in one chunk — no vertical stacking yet — which is simpler and, at 8KB, a
non-issue against the 4MB RAM budget even loaded 49-wide. 32 blocks tall
comfortably fits hills (terrain height range is clamped to 6..26) with
headroom on both ends.

**Terrain gen: 4-octave integer value noise**, not Perlin/simplex — value
noise only needs a hash + lerp per sample vs. Perlin's per-corner gradient
dot products, and the ARM9 has no hardware FPU so every multiply in the
per-column height function (up to 256 columns/chunk) is worth avoiding.
Everything in `noise.c` is 24.8 fixed-point integer math, no floats.
Heightmap-only for now: grass/dirt/stone layering plus a sea-level water
table and beaches, no caves or overhangs yet (those need real 3D density
noise, not just a heightmap, and would cost more per-block compute).

**Meshing: full greedy meshing**, not just naive per-face culling, chosen
because the numbers below don't work otherwise. `tests/host_test.c` builds
a real 3x3-chunk sample world and counts both: naive exposed-face culling
alone produces ~13,000 faces (26,000 tris); greedy merging the same
terrain gets that down to ~1,900 quads (~3,800 tris) — a 6.8x reduction.
The DS's rasterizer budget is roughly 120k tris/sec, i.e. ~4,000 tris at a
steady 30fps, so naive culling alone would already blow the budget on a
3x3 chunk window before a single frame of overdraw or a mob or the UI. The
mesher does NOT cull faces against neighboring chunks (a chunk's own edge
is always treated as exposed), which draws a thin extra strip of quads at
every chunk seam — a real but small inefficiency, not a correctness bug
(it never leaves a hole), left as a known future optimization.

**Streaming radius: 1 chunk (a 3x3 window, 9 chunks), and it's also the
render distance** since there's no frustum culling yet — everything loaded
gets drawn every frame. Measured via host_test: radius 1 costs ~3,800 tris
total (fits the ~4,000/frame budget with a little room for a HUD/mobs
later); radius 2 (5x5) already costs ~10,400 tris, well over. So radius is
staying small on purpose, per the DS-scoping brief, until frustum culling
or distance LOD can pay for a bigger number honestly. World storage at
radius 1 is ~100KB (9 chunks x ~11.3KB including cached mesh quads) —
trivial. Chunks stream via a fixed 3x3 *toroidal* grid (each chunk's slot
is `(cx mod 3, cz mod 3)`), so moving the camera never allocates: crossing
a chunk border just regenerates the 3 slots that changed identity in
place. host_test's walk simulation confirms this — 3 chunks rebuilt on a
same-row border crossing, 9 on a jump far enough that nothing overlaps.

**Vertex coordinate scale — a real hardware gotcha caught before ever
touching libnds:** NDS vertices are `v16`, a 4.12 fixed-point 16-bit type,
so each component only holds about ±8.0. A chunk is up to 32 blocks tall;
at 1 render-unit per block that already can't be expressed. Fix is
`BLOCK_UNIT = 0.2` (`include/render_nds.h`) — every position (chunk
translation, vertex data, camera) is scaled into render space by this
factor, keeping the worst case (a full-height chunk face) at 6.4, safely
under the limit. `host_test.c`'s `worstCaseLocalCoord()` reproduces
`render_nds.c`'s exact corner math and checks every generated quad against
it — currently the real worst case is 3.8 (terrain doesn't reach the full
32-block ceiling), well clear of the ~7.9997 limit. This is the kind of
bug that would otherwise only show up as corrupted/wrapped geometry on
real hardware with no error message, so it seemed worth a host-side check.

**Not verified from this sandbox**: actual frame rate, and whether the
above triangle-budget math holds up against the *real* DS rasterizer
(the "~120k tris/sec" figure is a commonly cited ballpark, not something
measured here) — that needs real hardware or Delta, same caveat as
milestone 1.

# Milestone 3 — player movement/collision, block break/place

The milestone-2 fly camera is gone; `Player` (`player.h/c`) is a grounded
character now: gravity, jump, walking, AABB-vs-voxel collision. It has no
libnds dependency either, so `tests/host_test.c` drives real physics steps
against a real generated world instead of trusting the math by inspection.

**Collision**: per-axis AABB resolution (move X, resolve; move Z, resolve;
move Y, resolve) against `worldIsCollidable()` — standard technique, and
cheap enough that a full 3D box-vs-blocks sweep per axis per frame is a
non-issue at these block counts. `blockIsCollidable()` reuses the exact
same rule as mesh opacity (air and water don't block you) rather than
duplicating it. Water isn't swimmable yet, just walkable-through.

**A bug host_test.c actually caught**: the first version of the Y-axis
resolution left the player wherever their last successful step happened to
land, rather than snapping to the surface. That meant landing height was
off by up to one frame's fall distance — the test (drop the player from
above the world, check landed height against the real terrain height)
failed with the player resting 0.2 blocks above the ground instead of
exactly on it. Fixed by snapping feet to `floor(y) + 1` on a downward
collision instead of leaving position wherever it stopped. A second test
(walk into a hand-placed wall for 5 seconds of held input) checks the
horizontal case doesn't tunnel either — it doesn't, the player stops
exactly at the wall face. Both regressions would otherwise have been "why
does the player float" or "why did I fall through the floor" bugs only
discoverable on real hardware with no debugger.

**"Unloaded chunk = solid"**: `worldIsCollidable()` treats a block outside
the streamed radius-1 window as collidable rather than open air. This was
a deliberate choice (documented in milestone 2 already) so the player
can't fall into an ungenerated void at the edge of the loaded area — but
it means walking to the edge of the loaded world currently feels like
hitting an invisible wall rather than the world simply not existing yet.
That's an acceptable stand-in until streaming radius grows or terrain
extends further than render distance; noting it here because it's exactly
the kind of thing that reads as a bug if you don't know it's on purpose
(host_test.c's own player-drop test tripped over it once, when the test
order let an earlier streaming-walk check move the loaded window away
from the block being tested).

**Block break/place**: `raycast.c` marches the camera ray in fixed
0.05-block steps (not a DDA voxel walk — reach is only 5 blocks, so the
extra steps are free, and it sidesteps the axis-aligned edge cases a grid
DDA is prone to) and reports both the hit block and the empty cell just
before it. Break clears the hit block; place always places stone into
that empty cell — there's no inventory yet (milestone 4), so "what to
place" is hardcoded for now. `worldSetBlock()` rebuilds only the edited
chunk's mesh, not its neighbors' — editing a block on a chunk seam can
leave a redundant internal face on the neighbor's already-built edge
(harmless, just an extra unseen quad), same tradeoff as the meshing seam
note in milestone 2, not fixed here either.

**Controls**: D-pad look (yaw/pitch), A/B walk forward/back, L/R strafe,
X jump, Y break the targeted block, SELECT place the selected hotbar item,
START quit. As of milestone 4 the bottom screen is the touch inventory
HUD (see below), not debug text.

**Not verified from this sandbox**: same as before — actual movement feel,
jump height, and whether 0.05-block raycast steps are fine-grained enough
in practice all need real hardware or Delta to judge. The physics numbers
(gravity, jump speed, walk speed) are unitless guesses tuned by "seems
about right" reasoning, not measured against anything.

# Milestone 4 — inventory, hotbar, bottom-screen touch UI

Breaking a block now adds it to the inventory (`inventory.h/c`, no libnds
dependency, host-tested) instead of just deleting it, and placing uses
whatever's in the selected hotbar slot instead of always being stone —
empty-handed placing is a no-op. Inventory is 32 slots: the first 8 are
the hotbar (always visible), the rest a 3x8 backing grid. Tap a hotbar
slot to select it; tap a grid slot to swap it with whatever's currently
selected. That's the whole interaction model — no drag, no split-stack,
no shift-click — deliberately simpler than real Minecraft's inventory
rules, and good enough to move items around with two taps.

**The debug text console is gone.** Milestones 1-3 used
`consoleDemoInit()` for status text on the bottom screen; that's a
different NDS background mode than what a real touch UI needs
(`BgType_Bmp16`, a directly-writable 16-bit bitmap layer), and mixing a
text console with a bitmap layer on separate BG slots without them
visually colliding was more risk than it was worth for what the spec
actually asked for. So the bottom screen is now 100% the inventory/hotbar
HUD, per the brief, and there's no on-screen debug readout anymore.

**Why a hand-rolled bitmap HUD instead of sprites/tiles**: the mode table
in libnds' `video.h` only allows `BgType_Bmp16` (a real-color bitmap you
can plot individual pixels into) on BG2/BG3 in `MODE_5_2D` — verified by
pulling the actual header rather than assuming, since picking the wrong
mode would just silently show a blank screen with no error. Given that,
plotting slot borders/icons as raw pixels (`hud_nds.c`) was simpler than
setting up a tile/palette pipeline for something this small: 32 inventory
slots + 1 target preview, each a 20x20 cell (16x16 icon, reusing the same
`TEX_*` blob the 3D renderer already loads, plus a 2px border). One
real gotcha caught by reading the format doc closely: `BgType_Bmp16`
pixels are `aBBBBBGGGGGRRRRR` — the alpha bit controls whether a pixel
renders at all, and our `TEX_*` texture data never sets it (the 3D
`GL_RGB` texture format forces opacity in hardware instead, so it was
never needed there). Blitting a texture tile into the bitmap HUD without
OR-ing in that bit would render every icon as fully transparent — nothing
visibly wrong at the C level, just an empty-looking HUD. `blitIcon()` in
`hud_nds.c` ORs `BIT(15)` onto every copied pixel to account for this.

**What's host-tested vs. CI-only**: `hud_layout.c` (slot pixel positions,
touch-point-to-slot hit testing) has no libnds dependency, so
`tests/host_test.c` checks all 32 slots are on-screen, that tapping a
slot's center resolves back to that same slot, that no two slots'
rectangles overlap, and that the target-preview box doesn't double as a
real slot. `inventory.c` is checked too: stacking across the 64-item cap
splits into two slots correctly, and a deliberate swap-then-take sequence
moves and consumes exactly the items it should. None of that touches
`hud_nds.c` itself (the actual pixel plotting, `bgInitSub`, the video
mode) — that's CI-compile-verified only, same caveat as everything
NDS-specific so far: I still can't see the screen.

# Milestone 5 — crafting

Three recipes (`craft.c`), on purpose kept to the letter of "keep the
initial list small and extensible":

| input | output |
|---|---|
| 1 wood | 4 planks |
| 2 planks | 4 sticks |
| 4 planks | 1 crafting table |

Sticks (`ITEM_STICK`) are the first non-placeable item — they reuse the
same `BlockId`-as-item-id space as everything else (values `>= BLOCK_COUNT`
just mean "not a real voxel block", see `block.h`) rather than adding a
parallel item-type system for one entry. The crafting table is placeable
but doesn't do anything special yet — it's there because "wood -> planks
-> table" is the traditional first Minecraft recipe chain and it costs
nothing to include, not because tables gate crafting here (nothing does;
crafting works anywhere, there's no "must be near a table" rule since
there's no real reason to add that restriction yet).

**Matching is shapeless and strict**: a recipe fires when every filled
cell in the 2x2 grid holds the *same* ingredient and the filled count
exactly equals what the recipe wants (1, 2, or 4 here). No partial
matches, no arrangement-sensitivity — simpler than real Minecraft's
shaped recipes, appropriate for a grid this small with only three inputs.

**UI/interaction**: the crafting panel (2x2 grid + output slot + a mode
toggle button) sits top-right of the touch HUD, away from the inventory
grid/hotbar and target preview so nothing overlaps (checked for real —
see below). It's a mode rather than a separate screen: tapping the
toggle flips `craftMode`; while it's on, tapping a hotbar/inventory slot
moves one item from that stack into the crafting grid instead of
selecting/swapping, tapping a filled grid cell returns its item to the
inventory, and tapping the output slot (which previews the matching
recipe's result, if any) collects it and clears the grid. This is
simpler than Minecraft's own click-to-hold-drag-to-place interaction —
no cursor-holds-an-item state — chosen because two-tap-per-action is
easy to get right on a touchscreen without a stylus-drag threshold to
tune, and there's nothing here to visually test to confirm a fancier
interaction actually feels good.

**Testing**: `craft.c` is fully host-tested — each of the three recipes
end to end (place ingredients, match, collect, verify the right output
lands in inventory), a mismatched-ingredients case confirmed to *not*
match, and `craftReturnCell` confirmed to hand the item back rather than
lose it. `hud_layout.c`'s region table (all 32 inventory slots + 4 craft
cells + the output slot + the toggle, 38 regions total) is checked the
same way milestone 4's was: every region on-screen, every region's center
hit-tests back to itself with the right kind *and* index, and no two
regions overlap — confirming the hand-picked craft-panel coordinates
actually land clear of the inventory grid, hotbar, and target preview
before ever reaching a device to find out the hard way. As always, the
pixel output itself (`hud_nds.c`'s new craft-panel drawing) is
CI-compile-verified only.

# Milestone 6 — day/night cycle and lighting

`daynight.c` is a single float (seconds into the current day) with
functions mapping it to a sky color, a global 0..1 brightness multiplier,
and a day/night boolean mob spawning will read once mobs exist. No
libnds dependency, host-tested for the things that would otherwise only
show up by staring at a screen: light level and sky color actually get
darker going into night and lighter coming out (not just "different"),
the transitions are continuous (no snapping — checked by sampling 2000
steps across a full cycle and asserting no single step jumps more than a
small epsilon), and time wraps around correctly instead of `t` growing
without bound.

**Day length is 4 minutes, not Minecraft's 20.** Nothing about the
implementation cares about the exact number — `DAY_LENGTH_SECONDS` is one
constant — but 20 minutes means nobody testing this on a handheld or in
Delta would ever see it change during a normal play session. Also unlike
Minecraft: day and night aren't equal length. Day is 90% of the cycle
(`DAYNIGHT_SUNSET_START = 0.4` to `DAYNIGHT_NIGHT_START = 0.5` is the only
transition window before night, `DAYNIGHT_SUNRISE_START = 0.9` back to
day) — night is intentionally short, since mobs (next milestone) will
spawn during it and a long fumble-in-the-dark stretch isn't worth it yet
without mob AI to make that time interesting.

**Lighting is a global multiplier, not per-block light propagation.**
Real Minecraft tracks a light value per block and propagates it from sky
and light sources through the world — meaningfully more compute and
memory than this project's per-chunk mesh cache currently accounts for,
and there are no light-emitting blocks yet to make that investment pay
off. Instead, `render_nds.c`'s `renderWorld()` now takes a `lightMul`
that multiplies into every face's existing fixed shade (the
top/bottom/side brightness values from milestone 2) — cheap, and the
result is still every face getting relatively darker at night, just not
with real shadows or point lights. Night floor is 0.35, not 0, so terrain
stays legible rather than going to a black screen.

**Not verified from this sandbox**: whether the sky color and lighting
actually read as "day" and "night" to a human looking at the screen —
the RGB values and the 0.35 floor are judgment calls made without being
able to see them rendered. Same standing caveat as every NDS-visual
choice so far.

# Milestone 7 — mobs, survival stats (final priority-list item)

Two mob kinds (`mob.h/c`), matching the brief exactly: one passive
(wanders, flees the player) and one hostile (wanders, chases and attacks
the player). No pathfinding — wander picks a random heading and commits
to it for 1.5-4 seconds before picking another, flee/chase just aim
straight at or away from the player every frame, so a mob genuinely can
and will get stuck against a wall it doesn't route around. That's the
explicitly-scoped behavior from the brief, not an oversight.

**`entity.c` is new and both `player.c` and `mob.c` now depend on it.**
It's the AABB-vs-voxel collision code (box-vs-block overlap, per-axis
resolve-and-slide, snap-to-surface on landing) that used to live only in
`player.c`, pulled out once mobs needed the exact same logic at a
different box size. This is a refactor, not new behavior — confirmed by
rerunning milestone 3's existing fall/wall-collision host tests
unchanged afterward and getting bit-identical results (still lands at
exactly y=13.000, still stops at exactly x=10.700 against the wall).

**Spawning** (`mobWorldTrySpawn`) picks a random loaded chunk and a
random column in it, scans down for the first standable surface (solid,
non-water block with two clear blocks above it), and gates the kind on
`dayNightIsNight()` — passive by day, hostile by night, one at a time
per 5-second tick, capped at `MOB_MAX_COUNT = 6`. Host-tested: spawned
mobs actually land on solid ground with headroom (checked against the
real chunk data, not assumed), day/night gating produces the right kind
every time, and the pool genuinely stops growing at the cap rather than
overflowing `mobs[MOB_MAX_COUNT]`.

**Combat is one-directional.** Hostile mobs deal `MOB_ATTACK_DAMAGE = 2`
on contact with a 1-second cooldown (host-tested: one hit lands, the very
next frame doesn't double it) — but there's no way to fight back. No
weapon items, no "hit a mob" interaction at all. This is a genuine scope
cut, not an oversight: the brief's priority list doesn't mention combat
or tools, and adding a health/damage/death system for mobs on top of
everything else here would be a meaningfully sized feature of its own.
Right now a hostile mob is a hazard to route around or flee from, not
something to kill.

**Survival stats** (`Player.health`/`hunger`, `playerSurvivalStep()` in
`player.c`) are on Minecraft's familiar 0-20 scale: hunger drains 1 every
15s, health regens 1 every 4s while hunger > 6, and starvation damages 1
every 8s once hunger hits 0 — floored at 1 HP, same as vanilla on normal
difficulty, so starvation alone can't kill you. **There is no food item
to eat yet** — the block palette (grass/dirt/stone/sand/water/wood/
leaves/coal ore/planks/table/stick) has nothing edible, and adding one
this late would be scope creep for a "keep the initial list small"
brief. So hunger is presently a one-way drain to the starvation floor
every session; the tick math is real and tested (host_test.c verifies
the drain schedule, the regen schedule, and that starvation stops exactly
at the floor, not below it), but there's currently no way back up. Taking
lethal damage (from a hostile mob — the only damage source right now)
respawns at the stored spawn point with full health and hunger restored,
also host-tested.

**Mobs render as plain textured boxes**, not models — there's no
skeletal/animated mesh pipeline in this project, and a full model per
mob type would be real scope for something this small in the DS's
triangle budget anyway. Each active mob is 12 triangles (a cuboid) using
the same per-face shading and day/night light multiplier as terrain, so
at the population cap of 6 that's at most 72 extra triangles on top of
the ~3800 from milestone 2's terrain budget — comfortably inside the
~4000/frame target.

**Health/hunger UI**: two small bars on the touch HUD (top-left, below
the target preview, above everything else) — not interactive, just a
readout, included in `hud_layout.c`'s region table so the overlap check
still covers them (41 regions checked now, still zero overlaps).

**Not verified from this sandbox**, same standing caveat as every
NDS-visual/feel choice in this project: whether mob movement reads as
"wandering" rather than jittery, whether the box models are legible at
DS resolution, and whether the damage/regen pacing feels reasonable
during actual play. All of the *logic* — collision, state transitions,
spawn validity, damage/regen/starvation math — is host-tested and
verified correct; how it plays is not something this sandbox can judge.

---

## Project status

All seven milestones from the original brief are built and CI-verified:
toolchain scaffold, chunked/streamed terrain with greedy meshing, player
movement/collision with block break/place, inventory/hotbar/touch UI,
crafting, day/night with lighting, and mobs with survival stats. What
remains is everything this sandbox structurally cannot do: running the
ROM on real hardware or in Delta, tuning anything by feel (movement
speed, mob aggro ranges, sky colors, damage pacing), and swapping the
placeholder textures for a real Minecraft resource pack you own (see the
Texture assets section above). That's the natural next step, and it
needs a human with a screen.
