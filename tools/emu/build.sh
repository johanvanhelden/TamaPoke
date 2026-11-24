#!/bin/bash
# Builds the TamaPoke desktop emulator from the real firmware sources.
#
#   bash tools/emu/build.sh && tools/emu/tamapoke-emu --fast 60
#
# Needs SDL2 and g++. Without root, run tools/emu/setup-sdl.sh once and this
# script uses that copy. The hardware is stubbed, so the Arduino toolchain is
# not required.
set -e
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
cd "$HERE"

SDL_LOCAL="$HERE/.sdl/usr"
if [ -d "$SDL_LOCAL/include/SDL2" ]; then
  SDL_CFLAGS="-I$SDL_LOCAL/include/SDL2 -I$SDL_LOCAL/include/x86_64-linux-gnu -D_REENTRANT"
  SDL_LIBS="-L$SDL_LOCAL/lib/x86_64-linux-gnu -lSDL2 -Wl,--disable-new-dtags,-rpath,$SDL_LOCAL/lib/x86_64-linux-gnu"
elif command -v sdl2-config >/dev/null 2>&1; then
  SDL_CFLAGS="$(sdl2-config --cflags)"
  SDL_LIBS="$(sdl2-config --libs)"
else
  echo "SDL2 not found. Debian/Ubuntu: apt install libsdl2-dev (or run tools/emu/setup-sdl.sh without root); macOS: brew install sdl2" >&2
  exit 1
fi

# The Arduino build generates function prototypes for the .ino automatically;
# a plain g++ build does not, and the sketch calls plenty of functions before
# defining them. Regenerated every build so it can never go stale.
python3 genproto.py "$ROOT/TamaPoke.ino"

# .ino is C++ but g++ will not compile that extension
{ echo '#include "proto.h"'; cat "$ROOT/TamaPoke.ino"; } > sketch.cpp

# The emulator is a local test build: it uses the Gen-3 source profile.
g++ -std=c++17 -O1 -w \
  -I. -I"$ROOT" \
  -DTAMAPOKE_LOCAL_TEST \
  -DSPRITE_DIR="\"$ROOT/tools/sdcard/mons\"" \
  $SDL_CFLAGS \
  -o tamapoke-emu \
  sketch.cpp "$ROOT/pet.cpp" "$ROOT/i18n.cpp" "$ROOT/battle.cpp" "$ROOT/species_chirp.cpp" "$ROOT/thumb_format.cpp" \
  host_impl.cpp font.cpp clock.cpp main_sdl.cpp \
  $SDL_LIBS

echo "built: $HERE/tamapoke-emu"
