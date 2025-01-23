// =====================================================
// ___       ___
//  |  |__| |__
//  |  |  | |___
//
// ___       __         __   __                 __   __
//  |  |__| /  \  |\/| /__` /  \ |\ |     |\/| /  \ /__
//  |  |  | \__/  |  | .__/ \__/ | \|     |  | \__/ .__)
//
//  __   __        __   __      __   __   __
// /  ` /  \ |    /  \ |__)    |__) |__) /  \  |\/|
// \__, \__/ |___ \__/ |  \    |    |  \ \__/  |  |
//
// ___  ___  __  ___  __   ___       __
//  |  |__  /__`  |  |__) |__  |\ | /  ` |__|
//  |  |___ .__/  |  |__) |___ | \| \__, |  |
//
// =====================================================
// (c)2024-2025 David SPORN
// SPDX-License-Identifier: CC0-1.0
// =====================================================

// ---
// GPIO driver for the data bus.
// ---

// -- data pins (inputs) : D7-D12, A1 ; A1 is MSB (bit 7), A1 is LSB,
// bit 6 is always 0
const uint8_t DATA_PINS[] = {7, 8, 9, 10, 11, 12, A1};
const uint8_t sizeof_DATA_PINS = sizeof(DATA_PINS) / sizeof(DATA_PINS[0]);
uint8_t Data_value = 0;

// ** Setup GPIOs for the data as input.
void Data_setupGpios() {
  for (uint8_t i = 0; i < sizeof_DATA_PINS; i++) {
    pinMode(DATA_PINS[i], INPUT);
  }
}

// ** Read input pins and reconstruct Data_value.
void Data_read() {
  uint8_t data_buffer = 0;
  for (uint8_t i = 0; i < sizeof_DATA_PINS; i++) {
    data_buffer = data_buffer << 1;
    if (HIGH == digitalRead(DATA_PINS[i])) {
      data_buffer |= 1;
    }
    if (0 == i) {
      // skip bit 6
      data_buffer = data_buffer << 1;
    }
  }
  Data_value = data_buffer;
}
