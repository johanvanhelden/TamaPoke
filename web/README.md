# TamaPoke web installers

## Current public installer

End users currently use:

<https://shadowenemyx.github.io/TamaPoke/web/>

(This fork's installer is published at <https://johanvanhelden.github.io/TamaPoke/>,
see below.)

`index.html` and `manifest.json` serve the public Gen‑3 release `1.36.2` (the tracked upstream artifacts) with
Pokémon #1–386. They expose no debug controls and require the complete two-step
firmware-plus-sprites installation.

## Published installer (GitHub Pages)

Every push to `johan` runs `.github/workflows/pages.yml`: `make test`, then
`tools/build_pages.sh` (public `TAMAPOKE_GEN3_RELEASE` build), then a Pages
deploy. The site always carries the latest `johan` commit (shown on the page and
in `build-info.json`); firmware files are named
`tamapoke-<version>-<branch>-<shortsha>-*.bin` (for example `0.1.0-johan-<shortsha>`, where `0.1.0` is the fork's `FW_VERSION`). `dev.html`,
`manifest-local.json` and historical firmware are never published. The page is
branded as the johan fork build (title, fork firmware version, upstream base version, commit) with links
to the repository, changelog and upstream; `tools/build_pages.sh` does this, not
`web/index.html`.

One-time setup on the fork:

1. Enable Actions (Settings > Actions > General).
2. Settings > Pages > Source = **GitHub Actions**.
3. Settings > Environments > `github-pages` > Deployment branches: add `johan`
   (the default only allows the default branch).
4. Start the first run with a push to `johan` or the **Run workflow** button.

Flash with **Erase device** off to keep the save. A Gen‑3 install is the
firmware plus both sprite packs (`sprites.pak`, `sprites-gen3-update.pak`).
Build locally with `make pages-check` or `bash tools/build_pages.sh`.

## Gen‑3 pages

- `dev.html` + `manifest-local.json`: fork local build `0.1.0-local` (the `FW_VERSION` of the local profile), compiled with
  `TAMAPOKE_LOCAL_TEST`. It includes click-driven Pokémon, evolution, battle,
  IMU and step diagnostics.
- `release-gen3.html` + `manifest-gen3.json`: local reference copy of the same
  public-style `1.36.2` page, compiled with `TAMAPOKE_GEN3_RELEASE`.

The release preview deliberately exposes only one sprite action. It transfers
`sprites.pak` and then `sprites-gen3-update.pak`, giving every user the complete
#1–386 set. Firmware flashing without this second step is an incomplete Gen‑3
installation.

There is no `sprites-gen3-full.pak`. The former combined file exceeded 100 MB;
the two-package sequence keeps every GitHub-hosted file below the limit.

## Files

- `sprites.pak`: 502 normal/Shiny files for #1–251 plus 251-entry `thumbs.bin`
  (503 bundle entries).
- `sprites-gen3-update.pak`: 270 normal/Shiny files for #252–386 plus the full
  386-entry `thumbs.bin` (271 bundle entries).
- `firmware/tamapoke-<version>-*.bin`: bootloader, partitions, boot_app0 and app
  parts used by ESP Web Tools without overwriting the NVS save partition.

## Build and validate

```bash
bash tools/build_pages.sh --check       # fork Pages site (public release build)
bash tools/build_web_local.sh --check   # Gen 3 debug build for dev.html
bash tools/build_web_gen2_legacy.sh --check # pinned historical Gen 2
```

`make web-check` runs the first two. `build_web_local.sh` reads its version from
`FW_VERSION` of the `TAMAPOKE_LOCAL_TEST` profile in `TamaPoke.ino` (for example
`0.1.0-local`) and refuses to run when `manifest-local.json` disagrees. On a
version bump re-point `manifest-local.json` (version, part paths) and `dev.html`
(`?v=` and the label) to the new `-local` version. Omit `--check` to refresh the
local firmware parts (`web/firmware/tamapoke-<version>-*.bin`, gitignored) and
the Gen‑3 sprite bundle. The legacy Gen‑2 command is read-only.

**Frozen upstream flow:** `build_web.sh` and `build_web_gen3.sh` describe how the
tracked upstream `1.36.2` artifacts (`manifest.json`, `manifest-gen3.json`,
`index.html`, `release-gen3.html`, `firmware/tamapoke-1.36.2-*.bin`) were built.
They refuse to run because the source now carries the fork version; the fork's
public installer is built by `tools/build_pages.sh` in CI.

Each build validates its firmware version, Dex count, package entry counts and
thumbnail structure/count. The public Gen‑3 build also rejects binaries that
contain local `TESTMON` or `TESTEVO` command strings.

## Test locally

Web Serial and ESP Web Tools require HTTPS or localhost:

```bash
python3 -m http.server 8000 --directory web
```

- <http://127.0.0.1:8000/> — public Gen‑3 page
- <http://127.0.0.1:8000/dev.html> — Gen‑3 debug page
- <http://127.0.0.1:8000/release-gen3.html> — Gen‑3 public preview

Use desktop Chrome or Edge. Close the ESP Web Tools flash dialog before
reconnecting for sprite transfer because only one program can own the serial
port. Leave **Erase device** unchecked when preserving an existing save.

Both Gen‑3 pages use bounded serial reads/writes. A timeout or USB disconnect
cancels the transfer, closes the port and returns the interface to the connect
state so the user can reconnect.

## Release policy

The public flow must retain the mandatory two-step warning and the single,
complete sprite button. `dev.html` and `manifest-local.json` must never replace
the public page or manifest.

Sprites are from
[PMD SpriteCollab](https://github.com/PMDCollab/SpriteCollab) under CC BY-NC;
see [`../CREDITS.md`](../CREDITS.md).
