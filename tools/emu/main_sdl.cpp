// SDL front end for the desktop emulator: window, mouse -> touch, serial on
// stdin, NVS persistence and a headless screenshot mode.
#include <SDL.h>   // sdl2-config (or build.sh's rootless copy) puts the SDL2 dir on the include path
#include "Arduino.h"
#include "Arduino_GFX_Library.h"
#include "Preferences.h"
#include <chrono>
#include <ctime>
#include <string>
#include <deque>
#include <vector>
#include <unistd.h>
#include <sys/select.h>

uint32_t g_seed = 0xC0FFEE;   // reseeded from the clock in main()
FakeSerial Serial;
FakeESP ESP;
FakeWire Wire;
volatile int g_touchX = 0, g_touchY = 0;
volatile bool g_touchDown = false;

void emuSetSpriteDir(const char *);
void emuAddSteps(uint32_t n);
void emuShake();
void setup();
void loop();
extern Arduino_Canvas *gfx;

#define PANEL 466

// millis() lives in clock.cpp so the headless tests share it
void FakeESP::restart() { Serial.println("emu: ESP.restart() -> exiting"); exit(0); }

// --- stdin-backed serial ---
static std::deque<std::string> g_lines;
static std::string g_partial;
static void pumpStdin() {
  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(0, &fds);
  struct timeval tv = { 0, 0 };
  while (select(1, &fds, nullptr, nullptr, &tv) > 0) {
    char buf[512];
    ssize_t n = read(0, buf, sizeof(buf));
    if (n <= 0) break;
    for (ssize_t i = 0; i < n; i++) {
      if (buf[i] == '\n') { g_lines.push_back(g_partial); g_partial.clear(); }
      else g_partial += buf[i];
    }
    FD_ZERO(&fds); FD_SET(0, &fds); tv = { 0, 0 };
  }
}
int FakeSerial::available() { return g_lines.empty() ? 0 : 1; }
String FakeSerial::readStringUntil(char) {
  if (g_lines.empty()) return String("");
  String s(g_lines.front());
  g_lines.pop_front();
  return s;
}

// --- NVS persistence ---
void nvsLoad(const char *path) {
  FILE *f = fopen(path, "rb");
  if (!f) return;
  uint32_t n = 0;
  if (fread(&n, 4, 1, f) != 1) { fclose(f); return; }
  for (uint32_t i = 0; i < n; i++) {
    uint32_t kl = 0, vl = 0;
    if (fread(&kl, 4, 1, f) != 1 || kl > 64) break;
    std::string k(kl, 0);
    if (fread(&k[0], 1, kl, f) != kl) break;
    if (fread(&vl, 4, 1, f) != 1 || vl > 4096) break;
    std::vector<uint8_t> v(vl);
    if (vl && fread(v.data(), 1, vl, f) != vl) break;
    nvs()[k] = v;
  }
  fclose(f);
}
void nvsSave(const char *path) {
  FILE *f = fopen(path, "wb");
  if (!f) return;
  uint32_t n = nvs().size();
  fwrite(&n, 4, 1, f);
  for (auto &kv : nvs()) {
    uint32_t kl = kv.first.size(), vl = kv.second.size();
    fwrite(&kl, 4, 1, f); fwrite(kv.first.data(), 1, kl, f);
    fwrite(&vl, 4, 1, f); if (vl) fwrite(kv.second.data(), 1, vl, f);
  }
  fclose(f);
}

// Pixels outside the round panel exist in the framebuffer but are invisible on
// the hardware, so they are dimmed rather than hidden.
static uint32_t toRgb(const uint16_t *fb, int x, int y) {
  uint16_t c = fb[y * PANEL + x];
  uint32_t r = ((c >> 11) & 31) * 255 / 31;
  uint32_t g = ((c >> 5) & 63) * 255 / 63;
  uint32_t b = (c & 31) * 255 / 31;
  int dx = x - 233, dy = y - 233;
  if (dx * dx + dy * dy > 233 * 233) { r /= 4; g /= 4; b /= 4; }
  return (r << 16) | (g << 8) | b;
}

static void writePPM(const char *path) {
  FILE *f = fopen(path, "wb");
  if (!f) { fprintf(stderr, "cannot write %s\n", path); return; }
  const uint16_t *fb = gfx->buffer();
  fprintf(f, "P6\n%d %d\n255\n", PANEL, PANEL);
  for (int y = 0; y < PANEL; y++)
    for (int x = 0; x < PANEL; x++) {
      uint32_t c = toRgb(fb, x, y);
      fputc(c >> 16, f); fputc((c >> 8) & 255, f); fputc(c & 255, f);
    }
  fclose(f);
  printf("wrote %s\n", path);
}

// Runs the firmware for `ms` of game time without a window and saves the frame.
static std::vector<std::pair<int, int>> g_taps;
static void runFor(uint32_t ms) {
  uint32_t start = millis();
  while (millis() - start < ms) { loop(); usleep(1000); }
}
static int headlessShot(const char *out, uint32_t ms) {
  setup();
  runFor(1000);
  for (auto &t : g_taps) {   // press, hold 120 ms, release, settle
    g_touchX = t.first; g_touchY = t.second; g_touchDown = true;
    emuFireInterrupt();
    runFor(120);
    g_touchDown = false;
    emuFireInterrupt();
    runFor(600);
  }
  runFor(ms);
  writePPM(out);
  return 0;
}

int main(int argc, char **argv) {
  g_seed = (uint32_t)time(nullptr);
  int scale = 2;
  const char *save = "tamapoke.nvs";
  const char *shotOut = nullptr;
  uint32_t shotMs = 3000;
  for (int i = 1; i < argc; i++) {
    if (!strcmp(argv[i], "--scale") && i + 1 < argc) scale = atoi(argv[++i]);
    else if (!strcmp(argv[i], "--fast") && i + 1 < argc) emuSetTimeScale(atoi(argv[++i]));
    else if (!strcmp(argv[i], "--save") && i + 1 < argc) save = argv[++i];
    else if (!strcmp(argv[i], "--sprites") && i + 1 < argc) emuSetSpriteDir(argv[++i]);
    else if (!strcmp(argv[i], "--shot") && i + 1 < argc) shotOut = argv[++i];
    else if (!strcmp(argv[i], "--shot-ms") && i + 1 < argc) shotMs = atoi(argv[++i]);
    else if (!strcmp(argv[i], "--tap") && i + 1 < argc) {
      int x, y;
      if (sscanf(argv[++i], "%d,%d", &x, &y) != 2) { fprintf(stderr, "--tap needs X,Y\n"); return 2; }
      g_taps.push_back({x, y});
    }
    else if (!strcmp(argv[i], "--wipe")) remove(save);
    else { fprintf(stderr, "unknown or incomplete option: %s\n", argv[i]); return 2; }
  }
  nvsLoad(save);
  if (shotOut) return headlessShot(shotOut, shotMs);   // no SDL at all

  if (SDL_Init(SDL_INIT_VIDEO) != 0) { fprintf(stderr, "SDL: %s\n", SDL_GetError()); return 1; }
  SDL_Window *win = SDL_CreateWindow("TamaPoke (emulator)", SDL_WINDOWPOS_CENTERED,
                                     SDL_WINDOWPOS_CENTERED, PANEL * scale, PANEL * scale, 0);
  SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (!ren) ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_SOFTWARE);
  SDL_Texture *tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_RGB888,
                                       SDL_TEXTUREACCESS_STREAMING, PANEL, PANEL);

  printf("TamaPoke emulator: click = touch, drag = swipe, hold W = walk (steps), S = shake, Esc = quit.\n");
  printf("Serial commands can be typed in this terminal. Time scale x%u (suspended while you touch).\n\n",
         emuTimeScale());

  setup();

  bool run = true;
  uint32_t lastStep = 0;
  std::vector<uint32_t> px(PANEL * PANEL);
  while (run) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_QUIT) run = false;
      // Every mouse event pulses the touch INT, the way the CST9217 does on the
      // board. The sketch ignores the panel entirely until that fires.
      else if (e.type == SDL_MOUSEBUTTONDOWN) {
        g_touchX = e.button.x / scale; g_touchY = e.button.y / scale; g_touchDown = true;
        emuFireInterrupt();
      } else if (e.type == SDL_MOUSEBUTTONUP) {
        g_touchDown = false;
        emuFireInterrupt();
      } else if (e.type == SDL_MOUSEMOTION && g_touchDown) {
        g_touchX = e.motion.x / scale; g_touchY = e.motion.y / scale;
        emuFireInterrupt();
      } else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) run = false;
      else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_s && !e.key.repeat) emuShake();
    }
    // ~2 steps per real second while W is held, regardless of --fast
    if (SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_W]) {
      uint32_t now = SDL_GetTicks();
      if (now - lastStep >= 500) { emuAddSteps(1); lastStep = now; }
    }
    pumpStdin();
    loop();

    if (gfx && gfx->frameReady) {
      gfx->frameReady = false;
      const uint16_t *fb = gfx->buffer();
      for (int y = 0; y < PANEL; y++)
        for (int x = 0; x < PANEL; x++) px[y * PANEL + x] = toRgb(fb, x, y);
      SDL_UpdateTexture(tex, nullptr, px.data(), PANEL * 4);
      SDL_RenderCopy(ren, tex, nullptr, nullptr);
      SDL_RenderPresent(ren);
    }
    SDL_Delay(5);
  }

  nvsSave(save);
  printf("\nemu: state saved to %s\n", save);
  SDL_DestroyTexture(tex);
  SDL_DestroyRenderer(ren);
  SDL_DestroyWindow(win);
  SDL_Quit();
  return 0;
}
