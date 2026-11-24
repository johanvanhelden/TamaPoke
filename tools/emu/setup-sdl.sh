#!/bin/bash
# Installs SDL2 into tools/emu/.sdl without root, for machines where
# `apt install libsdl2-dev` is not an option (no sudo). Debian/Ubuntu only.
# build.sh picks it up automatically.
set -e
HERE="$(cd "$(dirname "$0")" && pwd)"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT
cd "$TMP"
apt-get download libsdl2-dev libsdl2-2.0-0 libsamplerate0 libdecor-0-0
for deb in *.deb; do dpkg -x "$deb" "$HERE/.sdl"; done
echo "SDL2 unpacked into $HERE/.sdl"
