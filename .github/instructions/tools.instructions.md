---
applyTo: "tools/*.py,tools/*.sh"
---

# Build and asset tools

- Scripts are Python 3 (standard library plus Pillow where already used) and Bash with `set -euo pipefail`.
- `build_pages.sh --check` and `build_web_local.sh --check` validate a build without replacing installer artifacts; prefer them while iterating. `build_web.sh` and `build_web_gen3.sh` are frozen (upstream `1.36.2`) and refuse to run.
- Version strings and the `FQBN` are duplicated across the build scripts, `TamaPoke.ino` and `web/` manifests: change them together.
- Sprite tooling downloads from PMD SpriteCollab (CC BY-NC); keep caches in the gitignored `tools/downloads/` and `tools/pmd_cache/`.
