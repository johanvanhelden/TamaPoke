---
name: firmware-development
description: 'Build, test and flash the TamaPoke ESP32-S3 firmware: native logic tests, Arduino CLI compile with the project FQBN, version strings, local test vs public release profiles, serial debug commands and save (NVS) safety. Use when changing firmware code, bumping a version, compiling for the board, or deciding how to test a change.'
---

# Firmware development

## Hardware and toolchain

Board: Waveshare ESP32-S3-Touch-AMOLED-1.75 (16 MB flash, OPI PSRAM, 466×466 CO5300 AMOLED over QSPI, CST9217 touch, AXP2101 PMU, PCF85063 RTC, QMI8658 IMU, ES8311 audio, microSD over SDMMC 1-bit).

```bash
FQBN="esp32:esp32:esp32s3:CDCOnBoot=cdc,FlashSize=16M,PSRAM=opi,PartitionScheme=app3M_fat9M_16MB"
```

CI (`.github/workflows/pages.yml`) installs the same pinned core and libraries; keep both in sync when you change a version. Validated libraries: ESP32 Arduino core 3.3.10, GFX Library for Arduino 1.6.7, SensorLib 0.4.1, XPowersLib 0.3.3. `arduino-cli` is needed for real builds (`make web-check`, `tools/build_web*.sh`); it is not needed for native tests or the emulator. If it is missing, tell the developer instead of installing it.

## Test ladder (cheapest first)

1. `make test`: native tests of `Pet`, battle code, thumbnails and the Web Serial script.
2. `make emu` / `make emu-shot`: the real firmware on the desktop (`emulator` skill), including 60× aging.
3. `make web-check` / `make pages-check`: full Arduino compile plus installer validation without replacing artifacts.
4. Flash the board with the local installer (`web-installer` skill). Hardware is the final judge for display clipping, touch, IMU, volume, SD transfer and save preservation.

## Build profiles

- `TAMAPOKE_LOCAL_TEST` (firmware `1.36.2-hoenn-walk-local`): debug serial commands (`TESTMON`, `TESTEVO`, ...) and the `web/dev.html` flow.
- `TAMAPOKE_GEN3_RELEASE` (firmware `1.36.2`): the public build without debug commands.
- Gen-3 source (`DEX_COUNT > 251`) without one of these defines fails on purpose (`#error`). The two defines are mutually exclusive.
- Bumping a version: change `FW_VERSION` in `TamaPoke.ino`, `tools/build_web_gen3.sh`, `tools/build_web_local.sh` and the manifests under `web/`, then run `make web-check`.

## Save safety

The pet lives in ESP32 NVS (the emulator keeps it in a `.nvs` file). Never ask the developer to erase the device for a firmware update. Test save-affecting changes by running an old save through the new build in the emulator (`--save` pointing at a copy) and adding a native test.

## Serial console

115200 baud over USB CDC. Commands like `STATS`, `HEALTH`, `SAVEINFO`, `STEPS`, `IMU`, `TIME` print state; `WIPE` and `ABANDON` destroy the pet, so never send them to a real board without asking.
