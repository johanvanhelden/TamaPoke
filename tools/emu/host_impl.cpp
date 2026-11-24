// Host versions of the hardware modules (SD card, RTC/PMU, audio, IMU). The
// sprite loader is the real TPK2/TPTH format, read from a directory instead of
// the SD card.
#include "Arduino.h"
#include "sdmon.h"
#include "rtcbat.h"
#include "audio.h"
#include "imu.h"
#include "thumb_format.h"
#include "dex.h"
#include <cstdio>
#include <string>

// Baked in by build.sh from the repo location; --sprites <dir> overrides it.
#ifndef SPRITE_DIR
#define SPRITE_DIR "../sdcard/mons"
#endif
static std::string g_spriteDir = SPRITE_DIR;
void emuSetSpriteDir(const char *d) { g_spriteDir = d; }

bool sdReady = true;
bool sdDirty = false;
SdThumbs thumbs;

static uint8_t *slurp(const std::string &path, uint32_t *size) {
  FILE *f = fopen(path.c_str(), "rb");
  if (!f) return nullptr;
  fseek(f, 0, SEEK_END);
  long n = ftell(f);
  fseek(f, 0, SEEK_SET);
  if (n <= 0) { fclose(f); return nullptr; }
  uint8_t *b = (uint8_t *)malloc(n);
  if (!b || fread(b, 1, n, f) != (size_t)n) { free(b); fclose(f); return nullptr; }
  fclose(f);
  *size = (uint32_t)n;
  return b;
}

bool PmdMon::load(uint16_t dexNum, bool shiny) {
  unload();
  char p[64];
  snprintf(p, sizeof(p), "%s/p%s%03u.bin", g_spriteDir.c_str(), shiny ? "s" : "", (unsigned)dexNum);
  uint32_t size = 0;
  blob = slurp(p, &size);
  if (!blob && shiny) {
    snprintf(p, sizeof(p), "%s/p%03u.bin", g_spriteDir.c_str(), (unsigned)dexNum);
    blob = slurp(p, &size);
  }
  if (!blob) return false;
  if (size < 7 || memcmp(blob, "TPK2", 4) != 0) { unload(); return false; }

  uint8_t nActs = blob[4];
  memcpy(&palCount, blob + 5, 2);
  if (palCount > 256 || (uint32_t)7 + palCount * 2 > size) { unload(); return false; }
  memcpy(pal, blob + 7, palCount * 2);

  const uint8_t *q = blob + 7 + palCount * 2, *end = blob + size;
  for (uint8_t i = 0; i < nActs && q + 4 <= end; i++) {
    uint8_t id = q[0], w = q[1], h = q[2], nf = q[3];
    q += 4;
    if (id >= PMD_NACTS || nf > 24) { unload(); return false; }
    uint32_t bytes = (uint32_t)nf * 2 + (uint32_t)w * h * nf;
    if (w == 0 || h == 0 || nf == 0 || q + bytes > end) { unload(); return false; }
    PmdAct &a = acts[id];
    a.w = w; a.h = h; a.frames = nf;
    for (uint8_t k = 0; k < nf; k++) { a.ms[k] = q[0] | (q[1] << 8); q += 2; }
    a.data = q;
    q += (uint32_t)w * h * nf;
    uint8_t base = 1;
    for (uint8_t f = 0; f < nf; f++) {
      const uint8_t *fr = a.data + (uint32_t)f * w * h;
      for (int r = h - 1; r >= 0; r--) {
        bool any = false;
        for (int c = 0; c < w && !any; c++) if (fr[r * w + c] != 0xFF) any = true;
        if (any) { if (r + 1 > base) base = r + 1; break; }
      }
    }
    a.base = base;
  }
  loaded = true;
  return true;
}

void PmdMon::unload() {
  if (blob) { free(blob); blob = nullptr; }
  for (auto &a : acts) { a.w = a.h = a.frames = a.base = 0; a.data = nullptr; }
  loaded = false;
}

bool SdThumbs::load() {
  unload();
  uint16_t parsedCount = 0;
  data = slurp(g_spriteDir + "/thumbs.bin", &dataSize);
  if (!data || !validateThumbBlob(data, dataSize, DEX_COUNT, &parsedCount)) {
    Serial.println("emu: no valid thumbs.bin (gallery without thumbnails)");
    unload();
    return false;
  }
  count = parsedCount;
  loaded = true;
  Serial.printf("emu: thumbnails loaded: %u\n", count);
  return true;
}

void SdThumbs::unload() {
  if (data) free(data);
  data = nullptr;
  dataSize = 0;
  count = 0;
  loaded = false;
}

const uint8_t *SdThumbs::get(int16_t dex) const {
  if (!loaded || dex < 1 || dex > count) return nullptr;
  uint32_t off;
  memcpy(&off, data + 6 + 4 * (dex - 1), 4);
  if (off > dataSize - 3) return nullptr;
  return data + off;
}

bool SdMon::load(uint16_t, bool) { return false; }   // legacy TPK1 path unused
void SdMon::unload() { if (data) { free(data); data = nullptr; } loaded = false; }
bool sdBegin() {
  Serial.printf("emu: sprites from %s\n", g_spriteDir.c_str());
  return true;
}
bool sdSerialCommand(const String &) { return false; }

// --- RTC / battery / PMU ---
static uint32_t g_epoch = 0;
bool rtcBegin() { return true; }
uint32_t rtcEpoch() { return g_epoch ? g_epoch + millis() / 1000 : 0; }
void rtcSetEpoch(uint32_t e) { g_epoch = e - millis() / 1000; }
bool batBegin() { return true; }
void pmuEnablePanel() {}
int batPercent() { return 87; }
bool batCharging() { return false; }
bool usbPresent() { return true; }
void setPowerCacheInterval(uint32_t) {}
void pwrSetup() {}
bool pwrShortPressed() { return false; }

// --- IMU: no sensor here; the SDL layer feeds steps while the W key is held ---
static uint32_t g_steps = 0, g_stepsTaken = 0;
static bool g_shake = false;
void emuAddSteps(uint32_t n) { g_steps += n; }
void emuShake() { g_shake = true; }
bool imuBegin() { return true; }
bool imuOk() { return true; }
void imuPoll(uint32_t, uint16_t) {}
bool imuShakeEdge() { bool s = g_shake; g_shake = false; return s; }
uint16_t imuTakeSteps() {
  uint32_t d = g_steps - g_stepsTaken;
  g_stepsTaken = g_steps;
  return d > 0xFFFF ? 0xFFFF : (uint16_t)d;
}
float imuLastMagG() { return 1.0f; }
float imuLastGyroDps() { return 0.0f; }
uint8_t imuAddr() { return 0x6B; }
uint32_t imuPedometer() { return g_steps; }
uint32_t imuSoftwarePedometer() { return g_steps; }

// Why the last run ended; the firmware's boot report reads this.
static int g_resetReason = ESP_RST_POWERON;
esp_reset_reason_t emuResetReason() { return g_resetReason; }
