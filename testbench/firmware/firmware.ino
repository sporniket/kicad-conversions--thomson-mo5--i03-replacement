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

// =====================================================
// Management variables
// =====================================================

// ---
// Main loop phases, alternating between read and write phases
// ---
// The main loop is processed 20 times per seconds, and each
// processing is either a read phase, when data lines and user
// input is read ; or a write phase, when the address lines and
// the status led are updated, and reports may be sent to the
// Serial line.

const unsigned int LOOP_INTERVAL_MS = 50; // 1000 ms / 20 = 50 ms

enum Phase { PHASE_READ, PHASE_WRITE };

Phase currentPhase = PHASE_READ;

// ---
// Status LED (the onboard LED) management
// ---
// The status LED is either a solid light (nothing to report)
// or an animated blinking to report an OK (the data read from
// an address is the same value as expected) or a KO (data read
// from an address is different from expected value).
// ---
// Animations are described by an array of 10 values (HIGH or LOW),
// the LED is updated ten times per seconds, thus each step is displayed
// for 1/10 of second.

// The OK animation is two quick blinks then a long pause.
uint8_t STATUS_LED_SEQUENCE_PASS[] = {HIGH, LOW, HIGH, LOW, LOW,
                                      LOW,  LOW, LOW,  LOW, LOW};

// The KO animation is a slow, 50% duty cycle blink.
uint8_t STATUS_LED_SEQUENCE_WRONG[] = {HIGH, HIGH, HIGH, HIGH, HIGH,
                                       LOW,  LOW,  LOW,  LOW,  LOW};

// Animation management
// -- We keep track of the current animation
uint8_t *StatusLed_sequence =
    STATUS_LED_SEQUENCE_PASS; // NULL, STATUS_LED_SEQUENCE_PASS or
                              // STATUS_LED_SEQUENCE_WRONG ;

// -- The current step of the animation
uint8_t StatusLed_index = 9;

// -- The limit on which one goes back to step 0
const uint8_t STATUS_LED_INDEX_MAX = 10;

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
const uint8_t ADDRESS_PINS[] = {6, 5, 4, 3, 2};
const uint8_t sizeof_ADDRESS_PINS =
    sizeof(ADDRESS_PINS) / sizeof(ADDRESS_PINS[0]);
uint8_t address_value = 0;

// -- data pins (inputs) : D7-D12, A7 ; D7 is MSB (bit 7), A7 is LSB,
// bit 6 is always 0
const uint8_t DATA_PINS[] = {7, 8, 9, 10, 11, 12, A7};
const uint8_t sizeof_DATA_PINS = sizeof(DATA_PINS) / sizeof(DATA_PINS[0]);
uint8_t data_value = 0;

// -- user input pins (buttons and toggle switches) : A6-A4 ;
const uint8_t INPUT_PINS[] = {A6, A5, A4}; // A6-A4
const uint8_t sizeof_INPUT_PINS = sizeof(INPUT_PINS) / sizeof(INPUT_PINS[0]);

// -- -- A6 = Action push-button
const uint8_t INPUT_PIN_ACTION = 0;
bool on_action = false;
uint8_t action_value = LOW;
const uint8_t ACTION_COOLDOWN_START = 3;
uint8_t action_cooldown = ACTION_COOLDOWN_START;

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
// When a full cycle (automatic mode) or a new address has been probed
// (manual mode), a report is sent to serial ; either a full report
// (automatic mode), either a single data point report (manual mode)
// ---
uint8_t report_full_raw[32] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                               0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
const uint8_t sizeof_report_full_raw =
    sizeof(report_full_raw) / sizeof(report_full_raw[0]);

// =====================================================
// Prepare...
// =====================================================

void setup() {
  // -- onboard LED
  pinMode(LED_BUILTIN, OUTPUT);

  // -- other setups
  setupGpios();
  setupSerial();
}

void setupGpios() {
  // -- power control pins
  for (uint8_t i = 0; i < sizeof_POWER_CONTROL_PINS; i++) {
    pinMode(POWER_CONTROL_PINS[i], OUTPUT);
    digitalWrite(POWER_CONTROL_PINS[i], LOW);
  }

  // -- address pins
  for (uint8_t i = 0; i < sizeof_ADDRESS_PINS; i++) {
    pinMode(ADDRESS_PINS[i], OUTPUT);
    digitalWrite(ADDRESS_PINS[i], LOW); // Optional: set initial state
  }

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

void loop() {
  if (currentPhase == PHASE_READ) {
    // Handle input reading
    handleReadPhase();
    currentPhase = PHASE_WRITE; // Switch to the next phase
  } else if (currentPhase == PHASE_WRITE) {
    // Handle output updating
    handleWritePhase();
    currentPhase = PHASE_READ; // Switch to the next phase
  }

  delay(LOOP_INTERVAL_MS); // Wait for the next iteration
}

void handleReadPhase() {
  // Code to read inputs

  // -- read data pins
  uint8_t data_buffer = 0;
  for (uint8_t i = 0; i < sizeof_DATA_PINS; i++) {
    if (HIGH == digitalRead(DATA_PINS[i])) {
      data_buffer |= 1;
    }
    data_buffer = data_buffer << 1;
    if (0 == i) {
      // skip bit 6
      data_buffer = data_buffer << 1;
    }
  }
  data_value = data_buffer;
  // also update report.
  report_full_raw[address_value] = data_value;

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
  handleStatusLed();
  handlePower();
  handleAction();
}

void handleStatusLed() {
  if (NULL == StatusLed_sequence) {
    // just switch on the LED
    digitalWrite(LED_BUILTIN, HIGH);
    return;
  }
  StatusLed_index++;
  if (StatusLed_index >= STATUS_LED_INDEX_MAX) {
    StatusLed_index = 0;
  }
  if (StatusLed_sequence[StatusLed_index]) {
    digitalWrite(LED_BUILTIN, HIGH);
  } else {
    digitalWrite(LED_BUILTIN, LOW);
  }
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
  if (!on_action)
    return;

  if (LOW == mode_value) // manual mode
    handleAction_manual();
  else
    handleAction_auto();
}

void handleAction_manual() {
  // TODO
}

void handleAction_auto() {
  // TODO
}

void emitFullReport() {
  Serial.println();
  Serial.println("// BEGIN Content of the color PROM of the Thomson MO5");
  Serial.println("uint8_t data_of_prom[] = {");
  for (uint8_t i = 0; i < sizeof_report_full_raw; i++) {
    Serial.print("  0b");
    Serial.print(report_full_raw[i], BIN);
    if (i < sizeof_report_full_raw - 1) {
      Serial.print(',');
    }
    Serial.println();
  }
  Serial.println("};");
  Serial.println("// END Content of the color PROM of the Thomson MO5");
}

void emitSingleValueReport() {
  Serial.println();
  Serial.print("data_of_prom[");
  Serial.print(address_value);
  Serial.print("] = 0b");
  Serial.print(report_full_raw[address_value], BIN);
  Serial.println(" ;");
}
