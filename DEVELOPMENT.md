# Building

Standard devkitPro/libnds project. Needs `devkitARM` and `libnds` from the
devkitPro pacman repo (`sudo dkp-pacman -S nds-dev`), with `DEVKITARM` set
in your environment, then:

```
make
```

produces `minecraft-for-ds.nds`. `make clean` to reset.

CI builds the ROM on every push via `.github/workflows/build.yml` using the
`devkitpro/devkitarm` Docker image and uploads it as a workflow artifact.

Toolchain note: this was written in a sandbox with no ARM cross toolchain
and no route to the devkitPro package mirror, so nothing here has been
compiled locally — it's been checked against the actual libnds headers/
examples on GitHub and validated through the CI build above instead. Actual
hardware and Delta behavior (input mapping, save handling, real framerate)
still need a real device/emulator, which is outside what this environment
can do — that testing is on you.

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

**Fly camera**: no player/physics yet (that's milestone 3). D-pad turns
(left/right yaw, up/down pitch), A/B move forward/back, X/Y move up/down,
L/R strafe. Bottom screen shows position, current center chunk, and how
many chunks were rebuilt on the last streaming update.

**Not verified from this sandbox**: actual frame rate, and whether the
above triangle-budget math holds up against the *real* DS rasterizer
(the "~120k tris/sec" figure is a commonly cited ballpark, not something
measured here) — that needs real hardware or Delta, same caveat as
milestone 1.
