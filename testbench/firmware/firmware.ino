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

// =====================================================
// Management variables
// =====================================================

// ---
// Main loop phases, alternating between read and write phases
// ---
#include "phaseman.h"

// ---
// Status LED (the onboard LED) management
// ---
#include "StatusLed.h"

// ---
// Management of GPIOs
// ---
// GPIOs are used for 4 tasks :
// * Enabling/disabling the power supply for the device under test. Disabling
// the power supply allow to swap the device to test.
// * Select the address sent to the device under test
// * Read the data from the device under test
// * User inputs (one action button and two toggle switches)
// ---

// -- Power control pins (outputs) : A0
const uint8_t POWER_CONTROL_PINS[] = {A0};
const uint8_t sizeof_POWER_CONTROL_PINS =
    sizeof(POWER_CONTROL_PINS) / sizeof(POWER_CONTROL_PINS[0]);

// -- address pins (outputs) : D2-D6 ; D2 is MSB, D6 is LSB
#include "Address.h"

// -- data pins (inputs) : D7-D12, A1 ; A1 is MSB (bit 7), A1 is LSB,
// bit 6 is always 0
const uint8_t DATA_PINS[] = {7, 8, 9, 10, 11, 12, A1};
const uint8_t sizeof_DATA_PINS = sizeof(DATA_PINS) / sizeof(DATA_PINS[0]);
uint8_t data_value = 0;

// -- user input pins (buttons and toggle switches) : A2, A5-A4 ;
const uint8_t INPUT_PINS[] = {A2, A5, A4};
const uint8_t sizeof_INPUT_PINS = sizeof(INPUT_PINS) / sizeof(INPUT_PINS[0]);

// -- -- A2 = Action push-button
const uint8_t INPUT_PIN_ACTION = 0;
bool on_action = false;
uint8_t action_value = LOW;
const uint8_t ACTION_COOLDOWN_START = 3;
uint8_t action_cooldown = ACTION_COOLDOWN_START;
bool action_is_performing = false;
uint8_t action_remaining = 0;

// -- -- A5 = Mode toggle ; 0-> Manual, 1-> Automatic
const uint8_t INPUT_PIN_MODE_TOGGLE = 1;
bool onchange_mode = false;
uint8_t mode_value = LOW;

// -- -- A4 = Enable power toggle ; 0-> OFF, 1-> ON
const uint8_t INPUT_PIN_POWER_ENABLE = 2;
bool onchange_powerEnable = false;
uint8_t powerEnable_value = LOW;

// ---
// Data report sent to serial
// ---
#include "Report.h"

// =====================================================
// Prepare...
// =====================================================

void setup() {
  // -- onboard LED
  pinMode(LED_BUILTIN, OUTPUT);

  // -- other setups
  setupGpios();
  setupSerial();
  StatusLed_toPass();
}

void setupGpios() {
  // -- power control pins
  for (uint8_t i = 0; i < sizeof_POWER_CONTROL_PINS; i++) {
    pinMode(POWER_CONTROL_PINS[i], OUTPUT);
    digitalWrite(POWER_CONTROL_PINS[i], LOW);
  }

  // -- address pins
  Address_setupGpios();

  // -- data pins
  for (uint8_t i = 0; i < sizeof_DATA_PINS; i++) {
    pinMode(DATA_PINS[i], INPUT);
  }

  // -- user input pins
  for (uint8_t i = 0; i < sizeof_INPUT_PINS; i++) {
    pinMode(INPUT_PINS[i], INPUT);
  }
}

void setupSerial() {
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
}

// =====================================================
// Main loop
// =====================================================

void loop() { loopPhases(); }

void handleReadPhase() {
  // Code to read inputs

  // -- read data pins
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
  data_value = data_buffer;
  // also update report.
  Report_registerValue(Address_value, data_value);
  if (action_is_performing && LOW == mode_value) {
    Report_emitSingleValue(Address_value);
  }

  // -- read user input pins
  on_action = readInputButton(INPUT_PIN_ACTION, action_value);
  onchange_mode = readToggleSwitch(INPUT_PIN_MODE_TOGGLE, mode_value);
  onchange_powerEnable =
      readToggleSwitch(INPUT_PIN_POWER_ENABLE, powerEnable_value);
}

bool readInputButton(uint8_t button, uint8_t &value) {
  const uint8_t in = digitalRead(INPUT_PINS[button]);
  bool trigger = (HIGH == in && LOW == value);
  value = in;
  return trigger;
}

bool readToggleSwitch(uint8_t toggle, uint8_t &value) {
  const uint8_t in = digitalRead(INPUT_PINS[toggle]);
  bool onchange = (in != value);
  value = in;
  return onchange;
}

void handleWritePhase() {
  // Code to update outputs
  updateStatusLed();
  handlePower();
  handleAction();
}

void handlePower() {
  if (!onchange_powerEnable)
    return;

  for (uint8_t i = 0;
       i < sizeof(POWER_CONTROL_PINS) / sizeof(POWER_CONTROL_PINS[0]); i++) {
    digitalWrite(POWER_CONTROL_PINS[i], powerEnable_value);
  }
}

void handleAction() {
  // the handling is bogus, for now just
  // emitSingleReport/incrementAddress/emitAddress
  Report_emitSingleValue(Address_value);
  Address__loop();
  return;

  // skip that
  if (action_is_performing) {
    if (action_remaining > 0) {
      --action_remaining;
    } else if (0 == Address_value) {
      Report_emitFull();
      action_is_performing = false;
    }
    Address__loop();
  }
  if (action_cooldown > 0) {
    Serial.println("// -- cooldown > 0");
    --action_cooldown;
    return;
  }
  if (!on_action) {
    Serial.println("// -- on_action is false");
    if (action_value == LOW) {
      Serial.println("// -- action_value is LOW");
    } else {
      Serial.println("// -- action_value is HIGH");
    }
    return;
  } else {
    Serial.println("// -- on_action is true");
  }

  if (LOW == mode_value) // manual mode
    handleAction_manual();
  else
    handleAction_auto();
}

void handleAction_manual() {
  // TODO
  Address__loop();
  action_is_performing = true;
}

void handleAction_auto() {
  if (!action_is_performing) {
    Address_value = 0;
    action_is_performing = true;
    action_remaining = 1;
  }
}
