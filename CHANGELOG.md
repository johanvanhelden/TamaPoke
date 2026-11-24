# Changelog

All notable changes of this fork (`johanvanhelden/TamaPoke`, branch `johan`) relative
to upstream. The format follows [Keep a Changelog](https://keepachangelog.com/).

Upstream's own changes are not repeated here: see
[ShadowEnemyx/TamaPoke](https://github.com/ShadowEnemyx/TamaPoke) (this fork's
base, mirrored by the `main` branch) and
[socquique/TamaPoke](https://github.com/socquique/TamaPoke) (the original project).

## Why use this fork

- The installer at <https://johanvanhelden.github.io/TamaPoke/> is built by CI
  from every push to `johan`, so it always serves the latest build of this branch.
- You can try the firmware, the UI and the synthetic audio on a desktop with
  `make emu` (or `make emu-fast` for 60x time) before flashing the board.
- Every installer build is traceable: the page shows the commit and the
  firmware files are named after it.
- Differences from upstream, including any firmware changes, are listed per
  entry below. Whenever a change affects compatibility with upstream saves, the
  entry says so.

`FW_VERSION` stays aligned with upstream. Pages builds are tagged
`<version>-johan-<shortsha>` in the firmware file names and the manifest; the
firmware binary embeds only the plain version.

## [Unreleased]

### Added

- GitHub Pages installer published by CI from every push to `johan`: native tests,
  public Gen-3 release build with arduino-cli, both sprite packs, build info
  (`build-info.json`). The site never contains `dev.html`, the local manifest,
  historical firmware or debug commands (`tools/build_pages.sh`,
  `.github/workflows/pages.yml`, `make pages-check`).
- Desktop SDL emulator (`tools/emu/`, `make emu`, `make emu-fast`, `make emu-shot`)
  that runs the real firmware sources, with a `--fast` time speed-up, serial
  commands, step and shake simulation, headless screenshots and scripted taps.
  Derived from DylanPDao/TamaPoke's `tools/emu` (see `CREDITS.md`).
- Emulator audio: the real `audio.cpp` output is played through SDL.
- `FORKS.md`: comparison of public forks and a cherry-pick guide.
- Agent tooling for GitHub Copilot (`AGENTS.md`, `.github/` instructions, skills,
  shell guard hook, `.dev/` checks) and `make` targets (`make help`).

### Changed

- The Pages installer is labeled as the johan fork build (page title, a line with
  the upstream firmware version and commit) and links to this repository, its
  changelog and upstream (`tools/build_pages.sh`).
- `README.md` and `web/README.md` describe this fork, its installer and the
  emulator.
- Documentation of the branch strategy (`main` mirrors upstream, `johan` is the
  flashed integration branch).

### Fixed

- `pet.cpp`: cast `R_COMUN` to satisfy `-Werror=extra` on gcc 13 (no
  behavior change).
- Emulator: the save is persisted on every exit path (steps below a 100-step
  boundary, SIGHUP, `ESP.restart()`) and autosaved every 30 s.
