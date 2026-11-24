# TamaPoke

See `AGENTS.md` for the project instructions and `README.md` for the product. Conventions per path live in `.github/instructions/`, domain workflows in `.github/skills/`.

## Language

Code, identifiers, comments, commit messages and docs are US English. Firmware UI strings are ASCII. `AI_HANDOFF.md` is the only German file.

## Coding standards

- Plain C++17 firmware in a few flat files; match the style of the file you edit.
- Comments only for non-obvious reasons (hardware quirks, timing, memory), never to restate code.
- Firmware must stay within the ESP32-S3 limits: PSRAM for big buffers, no blocking I/O in `loop()`, no heap churn in the render path.

## Review focus

There is no CI for the firmware, so review what no tool checks:

- **Save safety:** changes to `Pet` persistence or NVS keys must stay backward compatible with existing saves; check version and size handling.
- **Native tests:** logic changes in `pet.cpp`, `battle.cpp`, `dex.h` or `i18n.cpp` have a matching test in `tests/`.
- **Hardware assumptions:** timing, I2C use, memory and power changes that the emulator cannot show; call them out for a hardware check.
- **Installer consistency:** version strings, manifests and sprite-pack requirements agree (see `AGENTS.md`).
- **Public vs local build:** nothing from `TAMAPOKE_LOCAL_TEST` or `web/dev.html` leaks into the public installer.
- **ASCII UI text** and fonts: no accents in firmware strings.
- **Licensing:** third-party code or assets are credited in `CREDITS.md`; forks carry different licenses (see `FORKS.md`).
