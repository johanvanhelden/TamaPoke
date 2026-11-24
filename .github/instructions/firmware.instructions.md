---
applyTo: "*.ino,*.cpp,*.h"
---

# Firmware (repository root)

- `TamaPoke.ino` is the sketch (UI, input, serial console, main loop); game logic lives in `pet.cpp`/`pet.h`, `battle.cpp`, `dex.h`/`species.h` (generated data), `i18n.cpp`, `imu.cpp`, `audio.cpp`, `sdmon.cpp`, `rtcbat.cpp`. Hardware access stays in the hardware modules, so logic remains testable on the host.
- Keep logic that can be tested natively out of the `.ino`: put it in a `.cpp`/`.h` that `tests/` can build with the stubs in `tests/stubs/`.
- The same sources are compiled by the desktop emulator with stubbed hardware (`tools/emu/`). A new call to an Arduino/ESP32 API that the stubs lack needs a stub in `tools/emu/Arduino.h` or `tools/emu/host_impl.cpp`; run `make emu-build` to check.
- UI strings are ASCII only. Add new strings for every language in `i18n.cpp`.
- Large buffers belong in PSRAM (`ps_malloc`); the render path must not allocate.
- Use the `deadlineActive`/`deadlineReached` helpers from `time_utils.h` for `millis()` deadlines (wrap-safe).
- Do not change save keys or struct layouts without a migration test; the pet's save lives in NVS.
- Debug serial commands (`TESTMON`, `TESTEVO`, ...) exist only in `TAMAPOKE_LOCAL_TEST` builds.
- Read the `firmware-development` skill before building or flashing.
