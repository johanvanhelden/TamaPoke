#!/bin/bash
# Builds and validates the unpublished local Gen-3 candidate.
set -euo pipefail

cd "$(dirname "$0")/.."
ROOT="$PWD"
FQBN="esp32:esp32:esp32s3:CDCOnBoot=cdc,FlashSize=16M,PSRAM=opi,PartitionScheme=app3M_fat9M_16MB"
CHECK_ONLY=0

if [[ ${1:-} == "--check" ]]; then
  CHECK_ONLY=1
  shift
fi
if [[ $# -ne 0 ]]; then
  echo "Usage: bash tools/build_web_local.sh [--check]" >&2
  exit 2
fi
if ! grep -Eq '^#define DEX_COUNT 386$' dex.h; then
  echo "Refusing local build: dex.h must contain exactly 386 species." >&2
  exit 1
fi
# FW_VERSION of the TAMAPOKE_LOCAL_TEST profile is the single source of the version.
VERSION="$(sed -nE '/^#ifdef TAMAPOKE_LOCAL_TEST/{n;s/^#define FW_VERSION "([^"]+)".*/\1/p;}' TamaPoke.ino)"
if [[ ! $VERSION =~ ^[0-9]+\.[0-9]+\.[0-9]+-local$ ]]; then
  echo "Refusing local build: cannot read FW_VERSION (MAJOR.MINOR.PATCH-local) of the TAMAPOKE_LOCAL_TEST profile from TamaPoke.ino (got '$VERSION')." >&2
  exit 1
fi
MANIFEST_VERSION="$(sed -nE 's/^  "version": "([^"]+)".*/\1/p' web/manifest-local.json)"
if [[ $MANIFEST_VERSION != "$VERSION" ]]; then
  echo "Refusing local build: web/manifest-local.json version '$MANIFEST_VERSION' differs from FW_VERSION '$VERSION'; update the manifest, its part paths and web/dev.html." >&2
  exit 1
fi

STAGE="$(mktemp -d "${TMPDIR:-/tmp}/tamapoke-local.XXXXXX")"
trap 'rm -rf "$STAGE"' EXIT
mkdir -p "$STAGE/build" "$STAGE/firmware"

echo "Compiling local Gen-3 candidate..."
arduino-cli compile --fqbn "$FQBN" \
  --build-property compiler.cpp.extra_flags=-DTAMAPOKE_LOCAL_TEST \
  --build-path "$STAGE/build" --export-binaries .

cp "$STAGE/build/TamaPoke.ino.bootloader.bin" "$STAGE/firmware/tamapoke-$VERSION-bootloader.bin"
cp "$STAGE/build/TamaPoke.ino.partitions.bin" "$STAGE/firmware/tamapoke-$VERSION-partitions.bin"
cp "$STAGE/build/boot_app0.bin" "$STAGE/firmware/tamapoke-$VERSION-boot_app0.bin"
cp "$STAGE/build/TamaPoke.ino.bin" "$STAGE/firmware/tamapoke-$VERSION-app.bin"

echo "Packing local Gen-3 sprite update..."
python3 tools/pack_bundle.py --gen3 --output "$STAGE/sprites-gen3-update.pak"

python3 tools/validate_web_build.py \
  --dex-header dex.h --app "$STAGE/firmware/tamapoke-$VERSION-app.bin" \
  --pak web/sprites.pak --manifest web/manifest-local.json \
  --expected-version "$VERSION" --expected-dex 386 --expected-files 503 --expected-thumbs 251
python3 tools/validate_web_build.py \
  --pak "$STAGE/sprites-gen3-update.pak" --expected-files 271 --expected-thumbs 386

if [[ $CHECK_ONLY -eq 1 ]]; then
  echo "CHECK OK: local installer files were not changed."
  exit 0
fi

for suffix in bootloader partitions boot_app0 app; do
  src="$STAGE/firmware/tamapoke-$VERSION-$suffix.bin"
  dst="$ROOT/web/firmware/tamapoke-$VERSION-$suffix.bin"
  cp "$src" "$dst.new"
  mv "$dst.new" "$dst"
done
cp "$STAGE/sprites-gen3-update.pak" "$ROOT/web/sprites-gen3-update.pak.new"
mv "$ROOT/web/sprites-gen3-update.pak.new" "$ROOT/web/sprites-gen3-update.pak"
echo "OK: local Gen-3 candidate and sprite bundles refreshed."
