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
// GPIO driver for the address data bus.
// ---

// -- address pins (outputs) : D2-D6 ; D2 is MSB, D6 is LSB
const uint8_t ADDRESS_PINS[] = {6, 5, 4, 3, 2};
const uint8_t sizeof_ADDRESS_PINS =
    sizeof(ADDRESS_PINS) / sizeof(ADDRESS_PINS[0]);
uint8_t Address_value = 0;

// ** Setup GPIOs for the address as output.
void Address_setupGpios() {
  for (uint8_t i = 0; i < sizeof_ADDRESS_PINS; i++) {
    pinMode(ADDRESS_PINS[i], OUTPUT);
    digitalWrite(ADDRESS_PINS[i], LOW); // Optional: set initial state
  }
}

// ** Cycle the address value.
void Address__increment() { Address_value = (Address_value + 1) & 0x1F; }

// ** Apply each bit of the address value to the corresponding GPIOs.
void Address__emit() {
  uint8_t addr = Address_value;
  for (uint8_t i = 0; i < sizeof_ADDRESS_PINS; i++) {
    digitalWrite(ADDRESS_PINS[i], (1 == (addr & 1)) ? HIGH : LOW);
    addr = addr >> 1;
  }
}

// ** Emit next address.
void Address__loop() {
  Address__increment();
  Address__emit();
}
