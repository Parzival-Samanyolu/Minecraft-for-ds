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

`data/texture.bin` is a hand-rolled 16x16 RGB555 placeholder (a green/dirt
checker), not a Minecraft texture — Mojang's textures are copyrighted, so
this repo won't ship them. `tools/make_placeholder_texture.py` regenerates
it if you want a different placeholder.

The real block atlas (deliverable 2+) should read PNGs from a resource pack
you already legally own (e.g. your own Minecraft install's default
textures), converted at build time — not committed to the repo. Plan is a
small conversion script (Pillow) that packs the 16x16 tiles you point it at
into one or more direct-color NDS textures and a block-id -> UV table,
downsampled/palettized as needed once we know how many distinct blocks fit
in the ~656KB VRAM budget alongside chunk geometry.
