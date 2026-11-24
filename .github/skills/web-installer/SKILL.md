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

## Workflow

```bash
bash tools/build_web_local.sh --check   # compile + validate, changes nothing
bash tools/build_web_local.sh           # refresh the four local firmware parts and the Gen-3 pack
python3 -m http.server 8000 --directory web
```

Then the developer opens `http://127.0.0.1:8000/dev.html` in Chrome or Edge and flashes with **Erase device off** (erasing deletes the pet's save). The server must keep running while flashing. Start it as a detached background process and stop it when done.

## Rules

- Full install = firmware + `sprites.pak` + `sprites-gen3-update.pak`, in that order. No `sprites-gen3-full.pak`.
- Public pages never contain debug controls; `tests/web_serial_tests.mjs` guards the dev page script (`make test`).
- Do not publish, tag a release or switch GitHub Pages; the developer does that.
- Validate with `tools/validate_web_build.py` (invoked by the build scripts) after changing packs or manifests.
