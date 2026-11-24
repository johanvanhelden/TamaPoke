# TamaPoke forks: research and cherry-pick guide

> **Snapshot, 2026-11-24.** Everything here describes the public repositories as they were on this date.
> Forks move fast (most were last pushed in Sept–Nov 2026); re-check before acting on any commit.

Method: `git clone --filter=blob:none --no-checkout` of each public repo over HTTPS, then `git log`,
`git ls-tree`, `git show HEAD:<file>`, `git diff --name-status`, and `git rev-list --left-right --count`
against upstream `socquique/TamaPoke` HEAD (`a441be2`). Star counts and "updated" dates come from the
public forks listing of `socquique/TamaPoke`. Legend: **[V]** verified in repo content, **[R]** taken from
the fork's README and not checked in code, **[U]** unverified.

## Baseline and corrections to the brief

- **Upstream is at v1.22, not v1.17/1.18.** `socquique/TamaPoke` HEAD `a441be2` (2026-11-23) has
  `web/manifest.json` version `1.22` and a CHANGELOG entry for 1.22. The upstream README badge still says
  `v1.17`, which is stale. [V]
- **The owner's repo is at ShadowEnemyx HEAD.** `/home/developer/projects/TamaPoke` HEAD is `b91a02e`,
  which is identical to `ShadowEnemyx/TamaPoke` HEAD (firmware 1.36.2). [V]
- Upstream has 94 commits that ShadowEnemyx does not have (`+87/-94`). These include the 1.19–1.22 changes
  (e.g. the 1.22 PWR ghost-press fix and the 16-bit ball-minigame score). [V]

## Licensing: inconsistency that matters for cherry-picking

1. **Upstream relicensed from MIT to CC BY-NC-SA 4.0.** Commit `a735806` (2026-11-21, "License: code from MIT
   to CC BY-NC-SA 4.0") is in upstream HEAD. CHANGELOG 1.22 says versions up to 1.21 stay MIT and that
   forks keep the same license. The upstream LICENSE file and README both say CC BY-NC-SA. [V]
2. **Every fork's LICENSE file is MIT,** and every fork's README says its code is MIT. The forks' LICENSE
   files carry "Copyright 2026 Quique Tortosa" (ShadowEnemyx, DylanPDao, Mu-Muu, Spocky2024, ccs21) or
   "Copyright 2026 Cheng Ren" (crenberries428). [V]
3. **None of the forks contains commit `a735806`,** so none of them has the CC BY-NC-SA code. Their MIT
   claim is consistent with a base at or before 1.21. [V]
4. **Consequence:** code taken from upstream commits at or after `a735806` (1.22 and later) is CC BY-NC-SA
   with a share-alike clause. Code from MIT-era commits (≤1.21) is MIT in the forks. Mixing the two in one
   file or one binary makes the result NC-SA. **Get a legal opinion before shipping merged code.** This is
   an analysis, not legal advice.
5. **Sprites are a separate restriction.** Forks use PMD SpriteCollab sprites under CC BY-NC 4.0
   (ccs21 README, Dylan README "Sprites: PMD SpriteCollab everywhere"). Upstream's README says its sprites
   are also CC BY-NC-SA. [R]
6. **Dylan's README** says "Original: MIT © 2026 Quique Tortosa" while upstream now says NC-SA. The fork is
   internally consistent only for the pre-1.22 base. [R]

## Repo facts per fork

Ahead/behind is against upstream HEAD `a441be2`. `+A` = fork-only commits, `-B` = upstream-only commits.
Counts were computed by `git rev-list --left-right --count` and match GitHub's numbers. [V]

### Upstream: socquique/TamaPoke (baseline, not a fork)

- Firmware 1.22 (manifest, CHANGELOG). Gen 1 only, 151 species. 8 UI languages (per Mu-Muu's comparison table). [V/R]
- Features: ball minigame, battle stats, growth by level, web installer, Pokédex, streak/bond/medals. [R]
- Tooling: `.github/workflows/ci.yml`, `test/` with shims (`test_power`, `test_bootbutton`, `test_pet`, …),
  `tools/sdcard/`, `tools/flash_175c.sh`. [V]
- LICENSE: CC BY-NC-SA 4.0 (since 1.22). [V]

### ShadowEnemyx/TamaPoke (owner's base)

- Stars 74, last push 2026-11-06, `+87/-94`. Fork point `58320b4`. Firmware **1.36.2**. [V]
- License: MIT, "Copyright 2026 Quique Tortosa". Fork point is pre-relicense, so no `a735806`. [V]
- Species: Gen 1–3, 386 (README, including shiny variants). [R]
- Languages: 6 (EN, DE, ES, FR, IT, PT). [R]
- Game mechanics: wild battles, catching, training, five minigames, daily goals, expeditions with an item
  inventory, Hoenn tour sites, region and starter choice on new Gen-3 saves, evolution with branching
  conditions, persistent step counter (top left and on the Steps card), walking rewards (daily 500/2000/5000
  steps; lifetime ranks 10k–250k), Pokédex filters, browser save backup/restore with an automatic safety
  backup. [R]
- Hardware: same Waveshare 1.75 board. Gen-3 install needs two `.pak` files (`sprites.pak` #1–251 and
  `sprites-gen3-update.pak` #252–386). Web installer for Chrome/Edge. [R/V]
- Tooling: `tests/` (battle_tests, pet_tests, audio_stub, web_serial_tests, stubs), `tools/emu` is **not**
  present in SE's tree but is present in the owner's repo, `tools/pmd_test`, `tools/sdcard`, `web/`. No
  CI workflow (`.github/` absent). [V]
- Save/NVS: `pet.cpp` uses NVS namespace `tamapoke` (same as upstream). Update without "Erase device" keeps
  the save (README). [V/R]
- Memory/PSRAM: framebuffer in PSRAM per the shared GFX library setup; no SE-specific figures. [U]
- New source files vs upstream 1.22: `battle.cpp/.h`, `imu.cpp/.h`, `species_chirp.cpp/.h`, `thumb_format.cpp/.h`,
  `time_utils.h`, `dayphase.h`, `AGENTS.md`, `AI_HANDOFF.md`, `HOENN_WALK_LOCAL.md`, `progress.md`. [V]
- Diff vs upstream (name-status): 546 added, 46 deleted, 31 modified, 3 renamed. [V]

### DylanPDao/TamaPoke

- Stars 34, last push 2026-09-01, `+187/-102`. Firmware **3.11** (manifest). [V]
- License: MIT (LICENSE and README), "Copyright 2026 Quique Tortosa". Does not contain `a735806`. [V]
- Species: "all 386 completable" (Gen 1–3). Note: README mentions `DEX_TBL[152]` in `gen_dex.py`, which
  looks inconsistent with 386. [R/U]
- Languages: 6 (EN default, ES, FR, DE, IT, PT). [R]
- Game mechanics: **five regions** (Kanto, Johto, Hoenn, Sinnoh, Unova), each with 8 leaders, Elite 4 and
  champion, easy and hard. README says four ladders are checked against pret disassemblies (0 differences
  across 39 trainers), Unova is written from knowledge and marked NOT VERIFIED. Battle system with
  moves/types/trainers. LAN battle. Party, save, bonds, streaks, medals, lifecycle with three endings, eggs
  by rarity, Pokédex. Procedural Game Boy-style synth music (`gbsynth`). [R/V for file names]
- Hardware: CO5300 AMOLED 466×466, CST9217 touch, AXP2101, PCF85063, ES8311 codec. Same as owner's board. [R]
- Tooling: **desktop SDL emulator `tools/emu/` (51 files, compiles the real firmware)**, `tools/check_installer.py`,
  `tools/pmd_test`, `tools/backs`, `tools/pokeapi_cache.old` (151 files, legacy cache), `web/` with
  `sprites-{kanto,johto,hoenn,sinnoh,unova,kalos}.pak`. No CI. Issues: 10 open, 1 PR. [V]
- New source files vs upstream: `battle.cpp/.h`, `party.cpp/.h`, `save.cpp/.h`, `link.cpp/.h`, `linknow.cpp/.h`,
  `gbsynth.cpp/.h`, `moves.h`, `trainers.h`, `types.h`, `music.h`, `avatars.h`, `backs.h`, `badges.h`,
  `noart.h`. [V]
- Save/NVS: `save.cpp` and `party.cpp` use NVS namespace `tamapoke`. Key-level compatibility with the
  owner's `pet.cpp` is **not analysed** [U]. Upstream's 1.22 note on keeping saves across updates applies only
  to upstream's own layout. [R]
- Memory/PSRAM: framebuffer in PSRAM (GFX library). [R]

### crenberries428/TamaPoke

- Stars 9, last push 2026-11-04, `+256/-102`. Firmware **3.30** (manifest). [V]
- **Built on Dylan's fork:** Dylan's HEAD is an ancestor of crenberries' HEAD (verified with
  `merge-base --is-ancestor`). Fork point is Dylan HEAD `6cd0466`. [V]
- License: MIT, "Copyright 2026 Cheng Ren". Does not contain `a735806`. [V]
- Species: "all 386 completable" [R]. Languages: 8 per README badge; the full list is cut off in the
  README excerpt I read [U].
- Game mechanics: everything in Dylan's fork, plus steps (`steps.cpp`: QMI8658 accelerometer, reward at
  2500 steps, STEPS command), a jukebox with songs (`music.h`, `music_ids.h`), Pokémon cries (`cry.cpp`,
  `cries.h`), day/dusk/night and alternative biome backgrounds, CJK UI fonts (`cjk.h`, `cjkfont.h`,
  `uifont.h`), settings screen, logo. README says 120-slot box (not verified). [R/V]
- Tooling: `tools/emu/` (61 files, the SDL emulator), `tools/fonts`, `tools/check_installer.py`,
  `tools/pmd_test`, `docs/screens/` (46 images), `.claude/`, `CLAUDE.md`, `HANDOVER.md`. No CI. [V]
- New source files vs upstream: `steps.cpp/.h`, `cry.cpp/.h`, `cries.h`, `cjk.h`, `cjkfont.h`, `uifont.h`,
  `logo.h`, `music_ids.h`, `imu.cpp/.h`, plus Dylan's set. [V]
- Diff vs upstream: 333 added, 347 deleted, 35 modified, 1 renamed. [V]
- Save/NVS: same `tamapoke` namespace as Dylan's `save.cpp`. [V/U]
- Memory: CJK fonts and cries add flash and possibly PSRAM load [U].

### Mu-Muu/TamaPoke ("TamaPoke with Chess")

- Stars 20, last push 2026-11-23, `+60/-38`. Firmware manifest `2026.11.23-fork 447c612` (not semver).
  Fork point is upstream `1fff020`, which is upstream **1.18**. [V]
- License: MIT (LICENSE and README "The code is MIT"). Does not contain `a735806`. [V]
- Species: **Gen 1–9, 1025**, one Pokédex page per region (README). [R]
- Languages: English and Simplified Chinese (`README.zh-CN.md`, `i18n.cpp`). [V/R]
- Game mechanics: **no battles**. Growing by age with level matching, mega evolution, natures, sickness,
  abilities. Points from chess.com games, steps (also from an iPhone), hockey practice, and region rewards
  → shop and fun. Eleven tournament events (no fighting). Eight gyms per region (72 total), 3-on-3 team
  events, Elite Four. Fun mini-games (orchard, fishing, mining, treasure). Friends by putting two boards
  side by side (encrypted gifts, mail, photos). Farm, album, stats page. Real weather, sunrise/sunset,
  seasons, holidays. Optional Stack-chan control. [R/V for file names]
- Hardware: same board. **PSRAM OPI and 16 MB flash required**; build uses
  `PartitionScheme=app3M_fat9M_16MB` (README). Optional 3D-printed case. [R]
- Tooling: `test/` with 20+ test files (chess, friend, gym, mail, steps, sport, stackchan, wifipick, …),
  `.github/workflows/ci.yml` and `pages.yml`, `web/` (installer), `website/` (demo), `tools/sdcard` (2044 files),
  `tools/mailbox-worker`, `tools/voice.py`. [V]
- Save/NVS: adds SD-card backup (`backup.cpp`, daily or on demand). NVS keys not analysed [U].
  `config_private.example.h` pattern is used for Wi-Fi/chess credentials. [V]
- New source files vs upstream: `chess.cpp/.h`, `farm.cpp/.h`, `fun.h/.ino`, `gyms.h/.ino`, `holiday.cpp/.h`,
  `link.cpp/.h`, `linkproto.cpp/.h`, `mail.cpp/.h`, `mailbox.cpp/.h`, `memoir.cpp/.h`, `phonesteps.cpp/.h`,
  `photos.cpp/.h`, `regions.h`, `shop.ino`, `sports.ino`, `stackchan.cpp/.h`, `steps.h`, `walltime.cpp/.h`,
  `weather.cpp/.h`, `wifisync.cpp/.h`, and more. [V]
- Diff vs upstream: 2081 added, 23 deleted, 46 modified. [V]

### Spocky2024/TamaPoke_MrsSpock

- Stars 3, last push 2026-10-22, `+124/-46`. Firmware **v6.78** (Changelog.txt). [V]
- License: MIT for code (LICENSE, README). Sprites CC BY-NC-SA. Does not contain `a735806`. [V]
- Species: **Gen 1–4, 493** (`gen1-kanto.pak` … `gen4-sinnoh.pak`). [R/V]
- Languages: English and German. [R]
- Game mechanics: Tamagotchi-style gameplay. Step collection, 30–60 minute expeditions with random events,
  magic berries, training "punching bag" (Boxsack) with touch zones, status pages. [R]
- Hardware: same board, ESP32S3 Dev Module, 16 MB flash, **OPI PSRAM required** (466×466×16-bit framebuffer
  ≈ 434 KB). U8g2 font data for CJK and Hangul (font data only). [R/V]
- Tooling: web installer (`web/`), German manual PDFs, `pics/`, `sdcard.zip`, no CI, no tests. [V]
  Binary assets (`sdcard.zip`, PDFs) are committed in the repo.
- New source files vs upstream: `bgmons.h`, `imu.cpp/.h`, `dex.h`, `i18n.*`. Diff vs upstream: 32 added,
  372 deleted, 20 modified. [V]

### ccs21/TamaPoke_Piggy_Kor (Korean localisation)

- Stars 6, last push 2026-09-21 (commit date; the forks listing says 2026-09-20). `+29/-90`. [V]
- License: MIT for repo source, with a note that sprites are CC BY-NC 4.0 and user-supplied assets are
  combined at flash time. [V/R]
- Species: **Gen 1 only, 151**. Languages: **Korean only** (README). [R]
- Hardware: 1.75 and 1.75C (32 MB flash). Firmware distinguishes the two by flash size. [R]
- Mechanics: "reworked growth speed, care calls, sleep and sitter, time correction, power saving" (README). [R]
- Tooling: Windows **flasher** (`flasher/`, C#, `TamaPoke-Flasher-v2.2.6.exe`), `Build-Public.ps1`,
  `Export-BatteryLog.ps1`, `docs/`, `licenses/`, `samples/`, `THIRD_PARTY_NOTICES.md`. No CI. [V]
- Layout: all firmware is under `firmware/`, so the tree is not a direct overlay of upstream: 70 added,
  391 deleted, 4 modified. [V]
- Source files: `firmware/korean_canvas.cpp/.h`, `firmware/generated/{dex_names_ko,help_ko,i18n_strings_ko,
  korean_font_data,manual_qr}.h`, `firmware/adpcm_clip.cpp/.h`, `firmware/walk_sensor.cpp/.h`,
  `firmware/time_sync.cpp/.h`, `firmware/power_log.cpp/.h`, `firmware/communication.cpp/.h`,
  `firmware/visual_assets.cpp/.h`, `firmware/moves.cpp/.h`, `firmware/battle.cpp/.h`. [V]
- Save: flasher asks whether to keep existing Korean save data (README). Save-format compatibility with
  upstream is **not analysed** [U].

### Other forks listed on socquique/TamaPoke/forks (not analysed)

Listed on the forks page with stars and last-updated date only. Not cloned, not analysed:
djyf1 (2★, 2026-09-03), shotah (1★, 2026-09-08), NairolfDev/TamaPoke-fight (0★, 2026-10-14, name suggests
battles), marwachs (0★, 13 open PRs, 2026-11-12), ljh841004-code (1 PR, 2026-10-21), LivesAtHome (2026-11-16),
l-suriano/poketrek (2026-10-31), kky3013-oss/TamaPoke-KR (2026-08-24, name suggests Korean), Kingtutz
(2026-10-01), KevinSays (list truncated). [V listing, U content]

## Feature matrix (feature × fork)

Legend: ✔ present, ◐ partial or different, ✘ absent, ? unverified. Base column is upstream 1.22.

| Feature | Upstream 1.22 | ShadowEnemyx (owner base) | Dylan | crenberries428 | Mu-Muu | Spocky2024 | ccs21 |
|---|---|---|---|---|---|---|---|
| Firmware version | 1.22 | 1.36.2 | 3.11 | 3.30 | 2026.11.23-fork (base 1.18) | v6.78 | ? |
| Species | 151 (Gen 1) | 386 (Gen 1–3) | 386 (Gen 1–3) | 386 | 1025 (Gen 1–9) | 493 (Gen 1–4) | 151 (Gen 1) |
| Languages | 8 | 6 | 6 | 8 | 2 (EN, zh-CN) | 2 (EN, DE) | 1 (KO) |
| Wild battles | ✘ (battle stats only) | ✔ | ✔ | ✔ | ✘ | ✘ | ◐ (battle.cpp) |
| Regions / gyms / Elite 4 | ✘ | ◐ region choice on new Gen-3 save | ✔ 5 regions | ✔ 5 regions | ◐ 8 gyms × regions, team events | ✘ | ✘ |
| LAN battle | ✘ | ✘ | ✔ | ✔ | ✘ (friend play over two boards) | ✘ | ✘ |
| Step counter | ✘ | ✔ | ✘ | ✔ (accel) | ✔ (incl. iPhone) | ✔ | ✔ (walk_sensor) |
| Chess.com / online points | ✘ | ✘ | ✘ | ✘ | ✔ | ✘ | ✘ |
| Expeditions / trips | ✘ | ✔ | ✘ | ✘ | ✔ (fun/farm) | ✔ | ✘ |
| Emulator (desktop) | ✘ | ✘ (owner repo has `tools/emu`) | ✔ tools/emu | ✔ tools/emu | ✘ | ✘ | ✘ |
| Tests | ✔ `test/` | ✔ `tests/` | ✘ | ✘ | ✔ `test/` (20+) | ✘ | ✘ |
| CI workflow | ✔ | ✘ | ✘ | ✘ | ✔ ci.yml + pages.yml | ✘ | ✘ |
| Web installer | ✔ | ✔ | ✔ | ✔ | ✔ | ✔ | ✘ (Windows flasher) |
| Save backup | ✘ | ✔ browser | ✘ | ✘ | ✔ SD daily/on-demand | ✘ | ✘ |
| Music / audio | ✔ (ball minigame) | ✔ synthetic chirps | ✔ gbsynth synth music | ✔ jukebox + cries | ✔ (voice.py, audio.cpp) | ✔ bgmons | ✔ ADPCM clips |
| Stack-chan control | ✘ | ✘ | ✘ | ✘ | ✔ | ✘ | ✘ |
| CJK / Korean fonts | ✘ | ✘ | ✘ | ✔ CJK fonts | ✔ zhfont.h | ◐ U8g2 font data | ✔ Korean canvas/font |
| Board | 1.75 | 1.75 | 1.75 | 1.75 | 1.75 (PSRAM OPI, 16 MB) | 1.75 (PSRAM OPI, 16 MB) | 1.75 / 1.75C |

## Feature-by-feature diff vs base TamaPoke (upstream 1.22)

- **Dylan:** adds battle, party, save, LAN link, regions/gyms, synth music, emulator. Diff vs upstream is
  278 added, 347 deleted, 33 modified; the deletions are not individually analysed. [V]
- **crenberries428:** Dylan's whole set plus steps, cries, jukebox, CJK fonts, backgrounds. [V]
- **Mu-Muu:** almost entirely new (2081 added). Keeps upstream's core `pet.cpp`, `audio`, `i18n`, `rtcbat`,
  `sdmon`. Drops upstream's battle stats in favour of growth, points and events. [V]
- **Spocky:** 32 added, 20 modified, 372 deleted vs upstream (the deletions are not individually analysed).
  Adds Gen 1–4 `.pak` files and a training minigame. [V/R]
- **ccs21:** a separate Korean firmware tree under `firmware/`, with a Windows flasher. [V]
- **ShadowEnemyx (owner base):** adds Gen 2–3, battles, expeditions, step counter, save backup, sprite-pak
  split. [V/R]

## Cherry-pick guide

Owner's repo (= ShadowEnemyx HEAD) already has: `battle.cpp/.h`, `imu.cpp/.h`, `species_chirp.cpp/.h`,
`thumb_format.cpp/.h`, `time_utils.h`, `dayphase.h`, `tests/`, `tools/emu/`, `tools/sdcard/`, `tools/pmd_test/`,
`web/`. It does **not** have: `party.cpp`, `save.cpp`, `link.cpp`, `steps.cpp`, `gbsynth.cpp`, `cry.cpp`,
`chess.cpp`, `farm.cpp`, `mail.cpp`, `backup.cpp`, `korean_canvas.cpp`, or any Gen 4 data.

Integration difficulty: **low** = drop-in or small patch; **medium** = touches shared files but no
architecture change; **high** = a second, competing subsystem with its own save format or file names.

| Feature | Implemented in | Key files | Difficulty | Conflicts with owner's code | Licensing caveat |
|---|---|---|---|---|---|
| Desktop emulator | Dylan, crenb (also owner's repo) | `tools/emu/*`, `tools/check_installer.py` | Already ported | None. Check that owner's `tools/emu` matches Dylan's latest. | Dylan MIT (pre-1.22 base) |
| Step counter | ShadowEnemyx (owner base), crenb (accel), Mu-Muu (steps + iPhone), Spocky, ccs21 | crenb: `steps.cpp/.h`, `imu.cpp`; Mu-Muu: `steps.h`, `phonesteps.cpp` | Owner has one already. Prefer keeping SE's; crenb's accel threshold is a candidate upgrade. Mu-Muu's iPhone path needs Wi-Fi/server. | `imu.cpp` exists in both: merge by hand. | MIT |
| Battle system | Dylan (`battle.cpp`, `moves.h`, `types.h`, `trainers.h`, `party.cpp`), crenb (same, built on Dylan) | `battle.cpp/.h`, `party.cpp/.h`, `moves.h`, `types.h`, `trainers.h` | **High.** Same filename (`battle.cpp`) as owner's SE battle; Dylan's needs `party.cpp` and its own save layout. Do not merge files. Port as a replacement module behind a flag. | `battle.cpp/.h` name clash; depends on party and save layout. | MIT (Dylan, pre-1.22 base) |
| Region and starter selection | SE (owner base has it for Gen 3); Dylan and crenb (5 regions); Mu-Muu (`regions.h`, different concept) | Dylan: `trainers.h`, `badges.h`, `web/sprites-*.pak`; Mu-Muu: `regions.h`, `gyms.h/.ino` | Owner already has region/starter selection for Gen 3. Only the gym ladders (`trainers.h`) are a clean candidate. Medium. | Owner's SE region UI would clash with Dylan's region pill. | Dylan's Unova roster is self-described unverified. Check before using. |
| LAN battle | Dylan (`link.cpp`, `linknow.cpp`), crenb (same) | `link.cpp/.h`, `linknow.cpp/.h` | **High.** Needs Dylan's battle system and party code. Dylan's `link.cpp` name clashes with Mu-Muu's `link.cpp` (different protocol). | Depends on battle+party. | MIT (pre-1.22) |
| chess.com points | Mu-Muu (`chess.cpp/.h`, `config_private.example.h`) | `chess.cpp/.h`, `test/test_chess.cpp`, `shop.ino` | **High.** Needs Wi-Fi, secrets handling, and Mu-Muu's shop and points economy. | Owner has no shop, points or Wi-Fi sync layer. | MIT; uses the chess.com public API (check its terms) |
| 1025 species | Mu-Muu | `species.h`, `dex.h`, `tools/sdcard` (2044 files), `regions.h` | **High** and data-heavy: 1025 sprites on SD, SD index in RAM. Owner's `thumb_format` and `.pak` split would need reworking. | `species.h` and `dex.h` clash. Sprite pipeline changes. | PMD sprites CC BY-NC; NC applies to the sprite set regardless of code license |
| Save backup | ShadowEnemyx (browser backup, owner base), Mu-Muu (SD daily/on-demand, `backup.cpp`) | `backup.cpp/.h` | Owner has browser backup. Mu-Muu's SD backup is medium: it needs the SD layer (`sdmon`). | `sdmon.cpp` clash if Mu-Muu's version differs. | MIT |
| Korean i18n | ccs21 (Korean-only tree), crenb (CJK fonts), Spocky (U8g2 glyphs) | ccs21: `firmware/i18n_strings_ko.h`, `firmware/generated/*`, `korean_canvas.cpp`, `korean_font_data.h`; Spocky: U8g2 font data | Medium. Adding a language to owner's `i18n.cpp` is easier than taking ccs21's tree. Korean font data source and license are unverified. | ccs21's tree is not an overlay: `i18n.cpp/.h` and `pet.cpp` diverge. | Korean font license **unverified**. |
| Gen 4 (Sinnoh) | Spocky (`gen4-sinnoh.pak`), Dylan and crenb (Sinnoh gym roster) | Spocky: `web/`; Dylan/crenb: `trainers.h` | Medium-high: need Gen 4 species data, dex tables (`tools/dex_data.py`, `gen_dex.py`), and sprites. Spocky's Gen 4 build logic is **unverified**. | Owner's dex pipeline is Gen 1–3. | PMD sprites NC; Gen 4 data source unverified |
| Music and audio | Dylan (`gbsynth.cpp`, `music.h`: synthetic GB-style music), crenb (`music.h`, `music_ids.h`, `cry.cpp`, `cries.h`: jukebox and cries), Spocky (`bgmons.h`), ccs21 (`adpcm_clip.cpp`) | `gbsynth.cpp/.h`, `cry.cpp/.h`, `cries.h`, `music.h` | Medium. `gbsynth` is self-contained. Cry data needs checking. | `audio.cpp` clash; owner's `species_chirp` is the chirp path. | **Check `cries.h` and `adpcm_clip`: they may contain game audio.** SE's README says it uses no game audio or ROM samples. Keep to that policy. |
| Mega evolution, weather, seasons, holidays | Mu-Muu (`holiday.cpp`, `weather.cpp`, `walltime.cpp`) | as listed | Medium-high, data- and network-dependent. | Clock and NTP code overlaps with `rtcbat`. | MIT |
| Stack-chan control | Mu-Muu (`stackchan.cpp/.h`) | as listed | Low-medium but optional hardware. | None if kept optional. | MIT |
| Upstream 1.19–1.22 fixes (e.g. PWR ghost press, 16-bit ball score) | Upstream `socquique/TamaPoke` (not in any fork) | Named in upstream CHANGELOG 1.22; file locations in the owner's tree not checked | Low: small patches. | Small. | **CC BY-NC-SA (commit ≥ `a735806`).** Do not copy without a licence decision. | **These are CC BY-NC-SA (commit ≥ `a735806`).** Do not copy without a licence decision. |

Fork lineage (verified):
- `socquique/TamaPoke` (upstream) → ShadowEnemyx (fork point `58320b4`, owner base) and Mu-Muu (fork point
  `1fff020`, upstream 1.18). [V]
- `socquique/TamaPoke` → DylanPDao (fork point at upstream `8e7d9b7`) → crenberries428 (built on Dylan's HEAD
  `6cd0466`). [V]
- Spocky2024 and ccs21 fork points not computed; both differ structurally from upstream. [U]

## Recommended next steps

1. Decide the licence question before taking any code from upstream 1.22 or later, or from any fork that
   later merges it.
2. Port Dylan's battle system as a separate, flag-gated module. Do not merge `battle.cpp`.
3. Take the 1025-species and Korean work only if the owner wants a different product; both are large.
4. Re-run the snapshot counts before acting on any fork.
