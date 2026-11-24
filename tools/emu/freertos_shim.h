// Minimal FreeRTOS queue/task shim for the desktop emulator: just enough for
// audio.cpp's synth task (std::thread + mutex/condvar instead of the RTOS).
#pragma once
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <cstring>
#include <deque>
#include <mutex>
#include <thread>
#include <vector>

typedef uint32_t TickType_t;
typedef int BaseType_t;
#define portMAX_DELAY 0xFFFFFFFFu
#define pdMS_TO_TICKS(ms) ((TickType_t)(ms))   // 1 tick = 1 ms
#define pdTRUE 1
#define pdFALSE 0

// Real sleeps are only wanted on worker threads; the main thread's delay() stays
// a no-op so the SDL loop keeps running. Set it at the top of a task.
inline thread_local bool g_emuRealDelay = false;

struct EmuQueue {
  std::mutex m;
  std::condition_variable cv;
  std::deque<std::vector<uint8_t>> items;
  size_t depth, itemSize;
  EmuQueue(size_t d, size_t s) : depth(d), itemSize(s) {}
};
typedef EmuQueue *QueueHandle_t;

inline QueueHandle_t xQueueCreate(unsigned depth, unsigned itemSize) { return new EmuQueue(depth, itemSize); }

inline BaseType_t xQueueSend(QueueHandle_t q, const void *item, TickType_t wait) {
  std::unique_lock<std::mutex> lk(q->m);
  auto room = [&] { return q->items.size() < q->depth; };
  if (!room() && (wait == 0 || !q->cv.wait_for(lk, std::chrono::milliseconds(wait), room))) return pdFALSE;
  const uint8_t *b = (const uint8_t *)item;
  q->items.emplace_back(b, b + q->itemSize);
  q->cv.notify_all();
  return pdTRUE;
}

inline BaseType_t xQueueReceive(QueueHandle_t q, void *item, TickType_t wait) {
  std::unique_lock<std::mutex> lk(q->m);
  auto has = [&] { return !q->items.empty(); };
  if (wait == portMAX_DELAY) q->cv.wait(lk, has);
  else if (!has() && (wait == 0 || !q->cv.wait_for(lk, std::chrono::milliseconds(wait), has))) return pdFALSE;
  memcpy(item, q->items.front().data(), q->itemSize);
  q->items.pop_front();
  q->cv.notify_all();
  return pdTRUE;
}

inline unsigned uxQueueMessagesWaiting(QueueHandle_t q) {
  std::lock_guard<std::mutex> lk(q->m);
  return (unsigned)q->items.size();
}

typedef void (*TaskFunction_t)(void *);
typedef void *TaskHandle_t;
// Detached: the process exits while the task is parked on its queue.
inline BaseType_t xTaskCreatePinnedToCore(TaskFunction_t fn, const char *, uint32_t, void *arg,
                                          unsigned, TaskHandle_t *, int) {
  std::thread([fn, arg] { g_emuRealDelay = true; fn(arg); }).detach();
  return pdTRUE;
}
