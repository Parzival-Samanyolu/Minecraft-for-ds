#!/bin/sh
# native build of the platform-independent core, for quick iteration
# without needing devkitARM. not part of the NDS build.
set -e
cd "$(dirname "$0")/.."
gcc -std=c99 -Wall -Wextra -O2 -Iinclude \
    tests/host_test.c source/noise.c source/worldgen.c source/mesh.c source/world.c \
    source/player.c source/raycast.c source/inventory.c source/hud_layout.c \
    source/craft.c source/daynight.c \
    -o /tmp/host_test -lm
/tmp/host_test
