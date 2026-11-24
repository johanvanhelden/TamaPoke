---
applyTo: "tests/**"
---

# Native tests

- Tests are plain C++17 with the stubs in `tests/stubs/` (`Arduino.h`, `Preferences.h`), built by `tests/Makefile` with `-Wall -Wextra -Werror`. `web_serial_tests.mjs` checks the installer page script with Node.
- Run `make test` from the repository root (or `make test` in `tests/`). A new test file needs a rule in `tests/Makefile`.
- Add or update a test for logic changes in `pet.cpp`, `battle.cpp`, `dex.h` or `i18n.cpp` when it gives meaningful regression coverage. Layout-only changes do not need tests.
- Test behavior through the public API of `Pet`/battle code, not internals. A test is done when a deliberate break in the feature code would make it fail.
- Keep tests simple: no loops or conditionals in test bodies unless they are helpers; avoid copying the same case several times.
- On exFAT volumes `._*` files break the build; this checkout is on ext4 so it does not apply.
