# TamaPoke desktop emulator

Runs the **real firmware** (`TamaPoke.ino`, `pet.cpp`, `battle.cpp`, `i18n.cpp`, `species_chirp.cpp`, `thumb_format.cpp`) on your computer in a window you can click. Only the hardware layer is replaced. Derived from DylanPDao/TamaPoke's `tools/emu` (see `CREDITS.md`).

```bash
make emu-setup   # once, only if libsdl2-dev is not installed and you have no sudo
make emu         # build and run (real time)
make emu-fast    # 60x time: one game minute per real second
```

Or directly: `bash tools/emu/build.sh && tools/emu/tamapoke-emu --scale 2 --fast 60`.

It is **not a substitute for the board**: timing, touch behaviour, DMA tearing, PSRAM pressure, audio and battery only exist on hardware.

## Using it

| Input | Effect |
|---|---|
| Click / drag | touch / swipe (gestures resolve on release, as on the device) |
| Hold `W` | walk: ~2 steps per real second (feeds the step counter) |
| `S` | shake |
| Terminal | the serial console: `STATS`, `HEALTH`, `STEPS`, `TIME`, `SHAKE`, `WIPE`, ... (debug commands work: the emulator builds with `TAMAPOKE_LOCAL_TEST`) |
| `Esc` / close window | quit; the save is written |

Pixels outside the round bezel are dimmed, not hidden, so you can see what the real panel would clip.

## Options

| Flag | Meaning |
|---|---|
| `--scale N` | window zoom (default 2) |
| `--fast N` | run the game clock N× faster. The speed-up is **suspended while you touch the panel**, because the sketch times gestures on the same clock (a tap must be under 1.5 s) |
| `--save FILE` | where NVS is persisted (default `tamapoke.nvs` in the current directory) |
| `--wipe` | delete the save first |
| `--sprites DIR` | sprite directory (default `tools/sdcard/mons`) |
| `--shot FILE.ppm` | headless: run without a window, write one frame and exit |
| `--shot-ms N` | game time to run before the capture (default 3000) |
| `--tap X,Y` | with `--shot`: tap at panel coordinates; repeatable. Taps run after a 1 s boot |

`make emu-shot TAPS="233,145" MS=2000` wraps the headless mode and writes `.dev/tmp/shot.ppm`.

## How it works

| File | Stands in for |
|---|---|
| `Arduino.h`, `Wire.h`, `esp_sleep.h` | `String`, `random`, `Serial` (wired to stdin), `attachInterrupt` (the SDL layer raises the touch INT by hand), ESP/sleep stubs |
| `clock.cpp` | `millis()`, including `--fast` scaling |
| `Preferences.h` | NVS, backed by a file so the pet survives restarts |
| `Arduino_GFX_Library.h` | the canvas: every primitive the sketch uses, into an RGB565 buffer |
| `TouchDrvCSTXXX.hpp` | the CST9217, fed by the mouse |
| `host_impl.cpp` | SD (sprites and thumbnails from a directory), RTC, battery, audio (silent), IMU (steps/shake from the keyboard) |
| `font.cpp` | the classic 5×7 GFX glyphs |
| `genproto.py` | the function prototypes the Arduino build normally generates |
| `main_sdl.cpp` | window, input, NVS file, headless capture |
| `setup-sdl.sh` | rootless SDL2 install into `.sdl/` |

When the firmware starts using a new hardware API, add a stub to these files; `make emu-build` shows what is missing.
