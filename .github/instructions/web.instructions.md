---
applyTo: "web/**"
---

# Web installer

- Pages: `index.html` + `manifest.json` (public), `release-gen3.html` + `manifest-gen3.json` (local copy of the public flow), `dev.html` + `manifest-local.json` (debug build). Shared logic in `installer-gen3.js`.
- Debug controls and `TAMAPOKE_LOCAL_TEST` artifacts never go into the public pages.
- The public flow always has both steps: firmware, then both sprite packs. Never require `sprites-gen3-full.pak`.
- `tests/web_serial_tests.mjs` parses `dev.html`; keep its expectations in sync when you change the page script.
- Binaries in `web/firmware/` are versioned deliberately: only the four parts of an approved version.
- Read the `web-installer` skill before building or serving the installer.
