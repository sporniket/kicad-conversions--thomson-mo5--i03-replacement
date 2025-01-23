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
// Report : management and send to serial
// ---
// When a full cycle (automatic mode) or a new address has been probed
// (manual mode), a report is sent to serial ; either a full report
// (automatic mode), either a single data point report (manual mode)
// ---
uint8_t Report__full_raw[32] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
const uint8_t SIZEOF__Report__full_raw =
    sizeof(Report__full_raw) / sizeof(Report__full_raw[0]);

const uint8_t MAX__Report__full_raw = SIZEOF__Report__full_raw - 1;

// ** Send the address to serial as right-aligned, space-padded decimal
// ** representation followed by leading-zero-filled binary representation.
//
// e.g. print 8 as " 8 /* 01000 */ "
void Report__printAddress(uint8_t value) {
  if (value < 10) {
    Serial.print(" ");
  }
  Serial.print(value);
  Serial.print(" /* ");
  if (value < 2) {
    Serial.print("0000");
  } else if (value < 4) {
    Serial.print("000");
  } else if (value < 8) {
    Serial.print("00");
  } else if (value < 16) {
    Serial.print("0");
  }
  Serial.print(value, BIN);
  Serial.print(" */ ");
}

// ** Send the data to serial as leading-zero-filled binary representation of 8
// digits
//
// e.g. print 8 as "0b00001000"
void Report__printDataAs8BinaryDigits(uint8_t value) {
  Serial.print("0b");
  if (value < 2) {
    Serial.print("0000000");
  } else if (value < 4) {
    Serial.print("000000");
  } else if (value < 8) {
    Serial.print("00000");
  } else if (value < 16) {
    Serial.print("0000");
  } else if (value < 32) {
    Serial.print("000");
  } else if (value < 64) {
    Serial.print("00");
  } else if (value < 128) {
    Serial.print("0");
  }
  Serial.print(value, BIN);
}

// ** Send the data to serial as leading-zero-filled binary representation,
// ** followed by an leading-zero-filled hexadecimal representation,
// ** followed by a right-aligned, space-filled decimal representation.
//
// e.g. print 8 as "0b00001000 ; // 0h08 //   8"
void Report__printData(uint8_t value) {
  Report__printDataAs8BinaryDigits(value);
  Serial.print(" ; // 0h");
  if (value < 16) {
    Serial.print("0");
  }
  Serial.print(value, HEX);
  Serial.print(" // ");
  if (value < 10) {
    Serial.print("  ");
  } else if (value < 100) {
    Serial.print(" ");
  }
  Serial.print(value);
}

void Report_registerValue(uint8_t address_value, uint8_t data_value) {
  if (address_value >= SIZEOF__Report__full_raw)
    return;
  Report__full_raw[address_value] = data_value;
}

void Report_emitSingleValue(uint8_t address_value) {
  if (0 == address_value) {
    Serial.println("// BEGIN Content of the color PROM of the Thomson MO5");
    Serial.println("uint8_t data_of_prom[32] ;");
  }
  Serial.print("data_of_prom[");
  Report__printAddress(address_value);
  Serial.print("] = ");
  Report__printData(Report__full_raw[address_value]);
  Serial.println();
  if (MAX__Report__full_raw == address_value) {
    Serial.println("// END Content of the color PROM of the Thomson MO5");
    Serial.println();
  }
}

void Report_emitFull() {
  Serial.println();
  Serial.println("// BEGIN Content of the color PROM of the Thomson MO5");
  Serial.println("uint8_t data_of_prom[] = {");
  for (uint8_t i = 0; i < SIZEOF__Report__full_raw; i++) {
    Serial.print("  ");
    Report__printDataAs8BinaryDigits(Report__full_raw[i]);
    if (i < SIZEOF__Report__full_raw - 1) {
      Serial.print(',');
    }
    Serial.println();
  }
  Serial.println("};");
  Serial.println("// END Content of the color PROM of the Thomson MO5");
}
