#!/bin/bash
# Builds the public installer site that GitHub Pages serves for the integration branch.
# Phase 1 compiles the public release profile (never TAMAPOKE_LOCAL_TEST); phase 2
# assembles the site. It only writes into the output directory and temp directories.
set -euo pipefail

cd "$(dirname "$0")/.."
FQBN="esp32:esp32:esp32s3:CDCOnBoot=cdc,FlashSize=16M,PSRAM=opi,PartitionScheme=app3M_fat9M_16MB"
OUT="_site"
BUILD_DIR=""
CHECK_ONLY=0

usage() {
  echo "Usage: bash tools/build_pages.sh [--out <dir>] [--check] [--assemble-only <build-dir>]" >&2
  echo "  --assemble-only: skip arduino-cli; <build-dir> holds TamaPoke.ino.{bootloader,partitions}.bin," >&2
  echo "                   boot_app0.bin and TamaPoke.ino.bin (the arduino-cli build output)." >&2
  echo "  --check:         assemble into a temporary directory and delete it afterwards." >&2
  exit 2
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --out) [[ $# -ge 2 ]] || usage; OUT="$2"; shift 2 ;;
    --assemble-only) [[ $# -ge 2 ]] || usage; BUILD_DIR="$2"; shift 2 ;;
    --check) CHECK_ONLY=1; shift ;;
    *) usage ;;
  esac
done

if ! grep -Eq '^#define DEX_COUNT 386$' dex.h; then
  echo "Refusing Pages build: dex.h must contain exactly 386 species." >&2
  exit 1
fi
FW_VERSION="$(sed -nE '/^#elif defined\(TAMAPOKE_GEN3_RELEASE\)/{n;s/^#define FW_VERSION "([^"]+)".*/\1/p;}' TamaPoke.ino)"
if [[ ! $FW_VERSION =~ ^[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
  echo "Refusing Pages build: cannot read a plain FW_VERSION from TamaPoke.ino (release branch of the profile block, got '$FW_VERSION')." >&2
  exit 1
fi

COMMIT="${GITHUB_SHA:-$(git rev-parse HEAD)}"
SHORT="${COMMIT:0:7}"
BRANCH="${GITHUB_REF_NAME:-$(git rev-parse --abbrev-ref HEAD)}"
BRANCH_TAG="$(printf '%s' "$BRANCH" | tr -c 'A-Za-z0-9\n' '-' | sed -E 's/^-+|-+$//g')"
BRANCH_TAG="${BRANCH_TAG:-local}"
# '+' is not filename-safe, so the tag uses '-' everywhere.
TAG="$FW_VERSION-$BRANCH_TAG-$SHORT"
BUILT_AT="$(date -u +%Y-%m-%dT%H:%M:%SZ)"

STAGE="$(mktemp -d "${TMPDIR:-/tmp}/tamapoke-pages.XXXXXX")"
trap 'rm -rf "$STAGE"' EXIT
if [[ $CHECK_ONLY -eq 1 ]]; then
  OUT="$STAGE/site"
fi

if [[ -z $BUILD_DIR ]]; then
  BUILD_DIR="$STAGE/build"
  mkdir -p "$BUILD_DIR"
  echo "Compiling public release profile $FW_VERSION..."
  arduino-cli compile --fqbn "$FQBN" \
    --build-property compiler.cpp.extra_flags=-DTAMAPOKE_GEN3_RELEASE \
    --build-path "$BUILD_DIR" .
fi

declare -A PART_SRC=(
  [bootloader]="TamaPoke.ino.bootloader.bin"
  [partitions]="TamaPoke.ino.partitions.bin"
  [boot_app0]="boot_app0.bin"
  [app]="TamaPoke.ino.bin"
)
for suffix in bootloader partitions boot_app0 app; do
  if [[ ! -s $BUILD_DIR/${PART_SRC[$suffix]} ]]; then
    echo "Missing or empty ${PART_SRC[$suffix]} in $BUILD_DIR." >&2
    exit 1
  fi
done

echo "Assembling site in $OUT..."
rm -rf "$OUT"
mkdir -p "$OUT/firmware"
for suffix in bootloader partitions boot_app0 app; do
  cp "$BUILD_DIR/${PART_SRC[$suffix]}" "$OUT/firmware/tamapoke-$TAG-$suffix.bin"
done
cp web/installer-gen3.js web/sprites.pak web/sprites-gen3-update.pak "$OUT/"
: > "$OUT/.nojekyll"

# index.html pins its assets to the firmware version; re-pin them to the commit and
# show the commit next to the version. Each substitution must hit exactly once.
cp web/index.html "$OUT/index.html"
subst() {
  local pattern="$1" replacement="$2"
  if [[ $(grep -cF -- "$pattern" "$OUT/index.html") -ne 1 ]]; then
    echo "web/index.html changed: expected exactly one line containing '$pattern'." >&2
    exit 1
  fi
  sed -i "s|$pattern|$replacement|" "$OUT/index.html"
}
subst 'manifest="manifest.json?v='"$FW_VERSION"'"' 'manifest="manifest.json?v='"$SHORT"'"'
subst 'src="installer-gen3.js?v='"$FW_VERSION"'"' 'src="installer-gen3.js?v='"$SHORT"'"'
subst 'Gen 1–3</p>' 'Gen 1–3 · build <code>'"$SHORT"'</code></p>'

cat > "$OUT/manifest.json" <<EOF
{
  "name": "TamaPoke",
  "version": "$TAG",
  "released": "${BUILT_AT%%T*}",
  "new_install_prompt_erase": true,
  "builds": [
    {
      "chipFamily": "ESP32-S3",
      "parts": [
        { "path": "firmware/tamapoke-$TAG-bootloader.bin", "offset": 0 },
        { "path": "firmware/tamapoke-$TAG-partitions.bin", "offset": 32768 },
        { "path": "firmware/tamapoke-$TAG-boot_app0.bin", "offset": 57344 },
        { "path": "firmware/tamapoke-$TAG-app.bin", "offset": 65536 }
      ]
    }
  ]
}
EOF

cat > "$OUT/build-info.json" <<EOF
{
  "commit": "$COMMIT",
  "short": "$SHORT",
  "branch": "$BRANCH",
  "built_at": "$BUILT_AT",
  "firmware_version": "$FW_VERSION"
}
EOF

# The firmware embeds the plain FW_VERSION, the manifest carries the tagged one.
python3 tools/validate_web_build.py \
  --dex-header dex.h --app "$OUT/firmware/tamapoke-$TAG-app.bin" \
  --pak "$OUT/sprites.pak" --manifest "$OUT/manifest.json" \
  --expected-version "$TAG" --embedded-version "$FW_VERSION" \
  --expected-dex 386 --expected-files 503 --expected-thumbs 251
python3 tools/validate_web_build.py \
  --pak "$OUT/sprites-gen3-update.pak" --expected-files 271 --expected-thumbs 386

# Final guard: only the known public files, and nothing from the debug profile.
EXPECTED_FILES="$(printf '%s\n' \
  .nojekyll build-info.json index.html installer-gen3.js manifest.json \
  sprites-gen3-update.pak sprites.pak \
  "firmware/tamapoke-$TAG-app.bin" "firmware/tamapoke-$TAG-boot_app0.bin" \
  "firmware/tamapoke-$TAG-bootloader.bin" "firmware/tamapoke-$TAG-partitions.bin" | sort)"
ACTUAL_FILES="$(cd "$OUT" && find . -type f | sed 's|^\./||' | sort)"
if [[ $ACTUAL_FILES != "$EXPECTED_FILES" ]]; then
  echo "Refusing Pages build: unexpected site contents:" >&2
  diff <(printf '%s\n' "$EXPECTED_FILES") <(printf '%s\n' "$ACTUAL_FILES") >&2 || true
  exit 1
fi
if LEAKS="$(grep -rlaE 'TAMAPOKE_LOCAL_TEST|TESTMON|TESTEVO|manifest-local|dev\.html|release-gen3|sprites-gen2' "$OUT")"; then
  echo "Refusing Pages build: debug or local-only content found in:" >&2
  echo "$LEAKS" >&2
  exit 1
fi

if [[ $CHECK_ONLY -eq 1 ]]; then
  echo "CHECK OK: Pages site for $TAG validated; nothing was published."
else
  echo "OK: Pages site for $TAG written to $OUT."
fi
