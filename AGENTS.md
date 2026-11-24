# TamaPoke

Pokémon tamagotchi firmware (Arduino C++, ESP32-S3) for the Waveshare ESP32-S3-Touch-AMOLED-1.75 (round 466×466 AMOLED, CST9217 touch, AXP2101 PMU, PCF85063 RTC, QMI8658 IMU, ES8311 audio, microSD), plus a browser-based web installer in `web/`. This is a private personal fork of ShadowEnemyx/TamaPoke (Gen 1–3, 386 species), itself a fork of socquique/TamaPoke. See [README.md](README.md) for the product and [FORKS.md](FORKS.md) for how other forks differ.

This is a living document. If something here is wrong or stale, fix it rather than working around it.

## Where instructions live

- This file: what applies to every task.
- `.github/instructions/*.instructions.md`: conventions per path, loaded automatically for matching files.
- `.github/skills/`: `emulator`, `firmware-development`, `web-installer`. Activate the matching skill when you work in that domain.
- `.github/copilot-instructions.md`: language and review focus.
- `AI_HANDOFF.md` (German, long) holds the version chronicle and architecture notes; read it only when you need history. `progress.md` and `HOENN_WALK_LOCAL.md` are public-facing status notes.

## Command execution

Everything runs on the host (WSL2, Ubuntu); there is no Docker. Use the `make` targets below (`make help` lists them).

| Task | Command (from the repository root) |
| --- | --- |
| Native logic tests (firmware logic + Web Serial) | `make test` |
| Agent hook tests | `make test-agent-tools` |
| Check agent docs for staleness | `make ai-docs-check` |
| Fast checks without toolchain | `make validate` |
| Build + run the desktop emulator | `make emu` (`make emu-fast` = 60× time) |
| Headless emulator screenshot | `make emu-shot TAPS="233,145" MS=2000` |
| Validate installer builds (needs Arduino CLI) | `make web-check` |

- A `preToolUse` hook (`.github/hooks/`) denies `gh`, `git push`, `esptool` erase commands and `find /`. When it denies something, take the stated alternative instead of working around it.
- Never read or print `.gh-auth/` (a local GitHub login leftover).

## Validation

- Run the smallest check that covers the change. Logic goes first into the native tests (`tests/`), then the emulator, and only then the hardware.
- Native tests and the emulator do not replace hardware: display clipping, real touch, IMU, audio volume, SD transfer and save preservation are only proven on the board.
- After changing `AGENTS.md`, `README.md`, `.github/instructions/`, `.github/skills/` or the `Makefile`, run `make ai-docs-check`. Changes to `.github/hooks/` are covered by `make test-agent-tools`.

## Conventions

- Follow the conventions of sibling files. The firmware is plain C++ in a few large files; do not introduce frameworks or new base folders without approval.
- Firmware UI text stays ASCII (the bitmap font has no accents or umlauts). Code, identifiers, comments and docs are US English; `AI_HANDOFF.md` is the only German document.
- Do not write comments that restate the code.
- Do not endanger the save (ESP32 NVS). New logic needs native tests before UI or hardware work; avoid save-format migrations unless they are the task.
- Audio stays synthetic: no original Pokémon cries, ROM samples or other copyrighted audio.
- Put scratch files in `.dev/tmp/` (gitignored) and clean up afterwards.

## Release and installer rules

- Never publish, release, switch Pages or change the PR without an explicit request. Local building, testing and the local web installer are always fine.
- Firmware updates are flashed with "Erase device" off, or the save is lost.
- A Gen-3 install always has two steps: flash the firmware **and** send both sprite packs (`sprites.pak`, `sprites-gen3-update.pak`) to the microSD. `sprites-gen3-full.pak` does not exist (it exceeded GitHub's 100 MB limit) and must never be required.
- `web/dev.html`, `web/manifest-local.json` and `TAMAPOKE_LOCAL_TEST` never go into the public installer (`web/index.html`, `web/manifest.json`). The public build has no debug commands (`TESTMON`, `TESTEVO`, ...).
- Do not commit historical firmware binaries in bulk; only the four parts of an approved version.
- `FW_VERSION` in `TamaPoke.ino`, `tools/build_web_gen3.sh`, `tools/build_web_local.sh` and the manifests in `web/` must agree. The code wins; fix the docs afterwards.

## Branch strategy

- `main` is a clean mirror of `upstream/main` (ShadowEnemyx/TamaPoke). Never commit on it.
- `johan` is the integration branch: the one that is flashed to the board. It holds the emulator, agent tooling and own features.
- Work on `feature/<name>` branches cut from `johan` and merge them back into `johan`.
- Take upstream updates by fast-forwarding `main`, then `git merge main` into `johan` (merge, no rebase).
- Cherry-pick from other forks onto a `feature/*` branch, one feature per commit (see `FORKS.md`).
- Keep new features in new files with minimal hooks into `TamaPoke.ino`, and do not reformat existing code, so upstream merges stay small.
- Remotes and git config (`upstream`, `rerere`) are set up by the developer; do not change them.

## Data handling

- Never put credentials, tokens or personal data in prompts, logs, scratch files or commits.
- Sprites come from PMD SpriteCollab (CC BY-NC). Keep `CREDITS.md` accurate when you add third-party code or assets.

## Process

- This is a private project: do not use the `gh` CLI. Use plain `git` for local history and ask the developer when GitHub information is needed.
- Commit only when asked, and never push: the developer reviews and pushes. Write commit messages as [Conventional Commits](https://www.conventionalcommits.org/) with the `Co-authored-by: Copilot <223556219+Copilot@users.noreply.github.com>` trailer.
- Open a pull request only when asked.
- Keep replies concise.
