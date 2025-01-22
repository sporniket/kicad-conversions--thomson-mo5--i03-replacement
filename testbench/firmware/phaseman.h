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
// Main loop phases, alternating between read and write phases
// ---
// The main loop is processed 20 times per seconds, and each
// processing is either : 
//
// * a read phase, when data lines and user input is read ; 
// * or a write phase, when the address lines and
//   the status led are updated, and reports may be sent to the
//   Serial line.

const unsigned int LOOP_INTERVAL_MS = 50; // 1000 ms / 20 = 50 ms

enum Phase { PHASE_READ, PHASE_WRITE };

Phase currentPhase = PHASE_READ;

// ---
// To be implemented
// ---

// ** What to do during read phase
void handleReadPhase();

// ** What to do during write phase
void handleWritePhase();

// CALL THIS IN loop()
void loopPhases() {
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
