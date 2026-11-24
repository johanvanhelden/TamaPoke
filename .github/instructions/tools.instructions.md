---
applyTo: "tools/*.py,tools/*.sh"
---

# Build and asset tools

- Scripts are Python 3 (standard library plus Pillow where already used) and Bash with `set -euo pipefail`.
- `build_web*.sh --check` validates a build without replacing installer artifacts; prefer it while iterating.
- Version strings and the `FQBN` are duplicated across the build scripts, `TamaPoke.ino` and `web/` manifests: change them together.
- Sprite tooling downloads from PMD SpriteCollab (CC BY-NC); keep caches in the gitignored `tools/downloads/` and `tools/pmd_cache/`.
