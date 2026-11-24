---
name: web-installer
description: 'Build, serve and test the TamaPoke browser installer in web/ (Web Serial flashing, firmware parts, sprite packs, local dev page vs public page). Use when changing web/*.html, installer-gen3.js, manifests or sprite packs, preparing a local flash of the board, or checking that the public installer has no debug controls.'
---

# Web installer

The installer flashes firmware and copies sprites to the microSD over Web Serial. It needs desktop Chrome or Edge and a USB-connected board; neither is available to an agent, so the developer does the actual flashing. An agent builds, validates and serves.

## Pages

| Page | Manifest | Purpose |
| --- | --- | --- |
| `web/index.html` | `manifest.json` | Public installer, firmware `1.36.2`, no debug controls |
| `web/release-gen3.html` | `manifest-gen3.json` | Local copy of the public flow |
| `web/dev.html` | `manifest-local.json` | Debug build `1.36.2-hoenn-walk-local` with click-driven diagnostics |
| GitHub Pages site | generated `manifest.json` | Built by `tools/build_pages.sh` from `web/index.html` for every push to `johan`; firmware parts named `tamapoke-<FW_VERSION>-<branch>-<shortsha>-*.bin` |

## Workflow

```bash
bash tools/build_web_local.sh --check   # compile + validate, changes nothing
bash tools/build_web_local.sh           # refresh the four local firmware parts and the Gen-3 pack
python3 -m http.server 8000 --directory web
```

Then the developer opens `http://127.0.0.1:8000/dev.html` in Chrome or Edge and flashes with **Erase device off** (erasing deletes the pet's save). The server must keep running while flashing. Start it as a detached background process and stop it when done.

## GitHub Pages

`.github/workflows/pages.yml` runs `make test`, then `tools/build_pages.sh`, then deploys the result on every push to `johan` (or manually via workflow_dispatch). The script compiles the public release profile (`TAMAPOKE_GEN3_RELEASE`) and assembles the site (`index.html` with a commit-pinned manifest and script URL, `installer-gen3.js`, the two sprite packs, the four firmware parts, `build-info.json`, `.nojekyll`).

```bash
make pages-check                                   # compile + assemble + guards in a temp dir (needs Arduino CLI)
bash tools/build_pages.sh                          # writes _site (gitignored)
bash tools/build_pages.sh --assemble-only <dir>    # no compile; <dir> = arduino-cli build output with the 4 parts
python3 -m http.server 8000 --directory _site
```

The guard fails the build when the site contains anything outside the expected file list or the strings `TAMAPOKE_LOCAL_TEST`, `TESTMON`, `TESTEVO`, `manifest-local`, `dev.html`. `dev.html`, `manifest-local.json`, `manifest-gen3.json`, historical firmware and `sprites-gen2-update.pak` never go public.

## Rules

- Full install = firmware + `sprites.pak` + `sprites-gen3-update.pak`, in that order. No `sprites-gen3-full.pak`.
- Public pages never contain debug controls; `tests/web_serial_tests.mjs` guards the dev page script (`make test`).
- Do not enable or switch GitHub Pages, push, run the workflow or tag a release; the developer does that. The `johan` Pages workflow itself is approved.
- Validate with `tools/validate_web_build.py` (invoked by the build scripts) after changing packs or manifests.
