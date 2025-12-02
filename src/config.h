#pragma once
// === Configuration - change pins & settings here ===
//
// FULL PIN LIST USED IN THIS PROJECT (default):
// - Matrix Rows (inputs, internal pull-up): PA0, PA1, PA2, PA3, PA4, PA5, PA6, PA7
// - Matrix Cols (outputs, drive LOW to scan): PB3, PB4, PB5, PB6, PB7, PB8, PB9, PB10
// - Pitch axes (ADC inputs): PB0 (PITCH_X_PIN), PB1 (PITCH_Y_PIN)
// - Extra buttons (6): PB11, PB12, PB13, PB14, PB15, PA8
//
// NOTE: Do NOT use PA11 / PA12 for other signals if you want USB device functionality.
//       PA11 = USB D- ; PA12 = USB D+
//       These are intentionally left unused in default mapping.

// Matrix size
#define MATRIX_ROWS 8
#define MATRIX_COLS 8

// Rows: PA0..PA7 (active LOW when column driven low)
const uint8_t ROW_PINS[MATRIX_ROWS] = {
  PA0, PA1, PA2, PA3, PA4, PA5, PA6, PA7
};

// Columns: PB3..PB10 (drive LOW to scan)
const uint8_t COL_PINS[MATRIX_COLS] = {
  PB3, PB4, PB5, PB6, PB7, PB8, PB9, PB10
};

// Pitch axes (ADC capable pins)
#define PITCH_X_PIN PB0  // analogRead => 0..4095
#define PITCH_Y_PIN PB1  // analogRead => 0..4095

// Extra buttons (6): avoid PA11/PA12 (USB)
#define EXTRA_BTN_COUNT 6
const uint8_t EXTRA_BUTTON_PINS[EXTRA_BTN_COUNT] = {
  PB11, PB12, PB13, PB14, PB15, PA8
};

// Debounce & scan timings
#define DEBOUNCE_MS 8
#define SCAN_DELAY_US 30

// MIDI settings
#define MIDI_CHANNEL 1        // 1..16
#define NOTE_VELOCITY 100    // fixed velocity for key presses (0..127)
#define BASE_NOTE 36         // MIDI note for (row0,col0), will increment across matrix

// Extra button CC mappings
#define BANK_CC 32
#define SUSTAIN_CC 64

// End of config
