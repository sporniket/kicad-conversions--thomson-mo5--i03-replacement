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

// ---
// Animation designators
enum StatusLedAnimation { NONE, PASS, WRONG };

StatusLedAnimation StatusLed_animation = PASS;

// -- We keep track of the actual sequence
uint8_t *StatusLed_sequence = STATUS_LED_SEQUENCE_PASS;

// ---
// Animation management
// -- The current step of the animation
uint8_t StatusLed_index = 9;

// -- The limit on which one goes back to step 0
const uint8_t STATUS_LED_INDEX_MAX = 10;

// ** Change and resets the animation sequence.
void StatusLed__changeAnimation(StatusLedAnimation animation) {
  StatusLed_animation = animation;
  StatusLed_index = 9;
  StatusLed_sequence = (StatusLedAnimation::PASS) ? STATUS_LED_SEQUENCE_PASS
                                                  : STATUS_LED_SEQUENCE_WRONG;
}

// ** alias to `StatusLed_changeAnimation(StatusLedAnimation::PASS)`
void StatusLed_toPass() {
  StatusLed__changeAnimation(StatusLedAnimation::PASS);
}

// ** alias to `StatusLed_changeAnimation(StatusLedAnimation::WRONG)`
void StatusLed_toWrong() {
  StatusLed__changeAnimation(StatusLedAnimation::WRONG);
}

// ** alias to `StatusLed_changeAnimation(StatusLedAnimation::NONE)`
void StatusLed_toNone() {
  StatusLed__changeAnimation(StatusLedAnimation::NONE);
}

// ** Cycle through the animation steps.
void StatusLed_animate() {
  StatusLed_index++;
  if (StatusLed_index >= STATUS_LED_INDEX_MAX) {
    StatusLed_index = 0;
  }
}

// ** Update the status led.
void updateStatusLed() {
  StatusLed_animate();
  switch (StatusLed_animation) {
  case NONE: {
    // just switch on the LED
    digitalWrite(LED_BUILTIN, HIGH);
    break;
  }
  default:
    if (StatusLed_sequence[StatusLed_index]) {
      digitalWrite(LED_BUILTIN, HIGH);
    } else {
      digitalWrite(LED_BUILTIN, LOW);
    }
  }
}
