// I2S stub for the desktop emulator: samples go to the SDL audio device
// (emuAudioWrite in main_sdl.cpp), which also paces the caller to real time.
#pragma once
#include <cstddef>
#include <cstdint>

enum i2s_mode_t { I2S_MODE_STD = 0 };
enum i2s_data_bit_width_t { I2S_DATA_BIT_WIDTH_16BIT = 16 };
enum i2s_slot_mode_t { I2S_SLOT_MODE_STEREO = 2 };
enum i2s_std_slot_mask_t { I2S_STD_SLOT_BOTH = 3 };

size_t emuAudioWrite(const uint8_t *data, size_t bytes);

class I2SClass {
public:
  void setPins(int, int, int, int, int) {}
  bool begin(i2s_mode_t, uint32_t, i2s_data_bit_width_t, i2s_slot_mode_t, i2s_std_slot_mask_t) { return true; }
  size_t write(const uint8_t *data, size_t bytes) { return emuAudioWrite(data, bytes); }
};
