#include <Arduino.h>
#include "config.h"
#include <MIDIUSB.h>

/*
  main.cpp untuk 8x8 matrix USB-MIDI controller
  Semua pin didefinisikan di src/config.h. Program ini:
  - Scan matrix 8x8 (rows=inputs pullup, cols=drive low)
  - 6 extra buttons (transport/bank/sustain)
  - 2 potensiometer (pitch X -> Pitch Bend, pitch Y -> CC1)
  - Mengirim USB-MIDI via MIDIUSB library
*/

// Helper: print human-readable pin name for common PA/PB/PC pins used here
void printPinName(uint8_t pin) {
  switch (pin) {
    // PA0..PA15
    case PA0:  Serial.print(F("PA0"));  return;
    case PA1:  Serial.print(F("PA1"));  return;
    case PA2:  Serial.print(F("PA2"));  return;
    case PA3:  Serial.print(F("PA3"));  return;
    case PA4:  Serial.print(F("PA4"));  return;
    case PA5:  Serial.print(F("PA5"));  return;
    case PA6:  Serial.print(F("PA6"));  return;
    case PA7:  Serial.print(F("PA7"));  return;
    case PA8:  Serial.print(F("PA8"));  return;
    case PA9:  Serial.print(F("PA9"));  return;
    case PA10: Serial.print(F("PA10")); return;
    case PA11: Serial.print(F("PA11")); return;
    case PA12: Serial.print(F("PA12")); return;
    case PA13: Serial.print(F("PA13")); return;
    case PA14: Serial.print(F("PA14")); return;
    case PA15: Serial.print(F("PA15")); return;

    // PB0..PB15
    case PB0:  Serial.print(F("PB0"));  return;
    case PB1:  Serial.print(F("PB1"));  return;
    case PB2:  Serial.print(F("PB2"));  return;
    case PB3:  Serial.print(F("PB3"));  return;
    case PB4:  Serial.print(F("PB4"));  return;
    case PB5:  Serial.print(F("PB5"));  return;
    case PB6:  Serial.print(F("PB6"));  return;
    case PB7:  Serial.print(F("PB7"));  return;
    case PB8:  Serial.print(F("PB8"));  return;
    case PB9:  Serial.print(F("PB9"));  return;
    case PB10: Serial.print(F("PB10")); return;
    case PB11: Serial.print(F("PB11")); return;
    case PB12: Serial.print(F("PB12")); return;
    case PB13: Serial.print(F("PB13")); return;
    case PB14: Serial.print(F("PB14")); return;
    case PB15: Serial.print(F("PB15")); return;

    // PC0..PC15 (fallback in case you change pins)
    case PC0:  Serial.print(F("PC0"));  return;
    case PC1:  Serial.print(F("PC1"));  return;
    case PC2:  Serial.print(F("PC2"));  return;
    case PC3:  Serial.print(F("PC3"));  return;
    case PC4:  Serial.print(F("PC4"));  return;
    case PC5:  Serial.print(F("PC5"));  return;
    case PC6:  Serial.print(F("PC6"));  return;
    case PC7:  Serial.print(F("PC7"));  return;
    case PC8:  Serial.print(F("PC8"));  return;
    case PC9:  Serial.print(F("PC9"));  return;
    case PC10: Serial.print(F("PC10")); return;
    case PC11: Serial.print(F("PC11")); return;
    case PC12: Serial.print(F("PC12")); return;
    case PC13: Serial.print(F("PC13")); return;
    case PC14: Serial.print(F("PC14")); return;
    case PC15: Serial.print(F("PC15")); return;

    default:
      // fallback: print numeric value
      Serial.print(F("PIN#"));
      Serial.print((int)pin);
      return;
  }
}

// State arrays
static bool keyState[MATRIX_ROWS * MATRIX_COLS];
static bool lastKeyState[MATRIX_ROWS * MATRIX_COLS];
static unsigned long lastChange[MATRIX_ROWS * MATRIX_COLS];

static bool extraState[EXTRA_BTN_COUNT];
static bool lastExtraState[EXTRA_BTN_COUNT];
static unsigned long lastExtraChange[EXTRA_BTN_COUNT];

static uint8_t currentBank = 0;
static bool sustainOn = false;

// Helpers
inline int idx(int r, int c) { return r * MATRIX_COLS + c; }

// MIDI helpers using MIDIUSB
void sendNoteOn(uint8_t channel, uint8_t note, uint8_t velocity) {
  midiEventPacket_t noteOn = {0x09, (uint8_t)(0x90 | ((channel - 1) & 0x0F)), note, velocity};
  MidiUSB.sendMIDI(noteOn);
  MidiUSB.flush();
}
void sendNoteOff(uint8_t channel, uint8_t note, uint8_t velocity) {
  midiEventPacket_t noteOff = {0x08, (uint8_t)(0x80 | ((channel - 1) & 0x0F)), note, velocity};
  MidiUSB.sendMIDI(noteOff);
  MidiUSB.flush();
}
void sendCC(uint8_t channel, uint8_t cc, uint8_t value) {
  midiEventPacket_t ccPacket = {0x0B, (uint8_t)(0xB0 | ((channel - 1) & 0x0F)), cc, value};
  MidiUSB.sendMIDI(ccPacket);
  MidiUSB.flush();
}
void sendPitchBend(uint8_t channel, uint16_t bend14) {
  uint8_t lsb = bend14 & 0x7F;
  uint8_t msb = (bend14 >> 7) & 0x7F;
  midiEventPacket_t pb = {0x0E, (uint8_t)(0xE0 | ((channel - 1) & 0x0F)), lsb, msb};
  MidiUSB.sendMIDI(pb);
  MidiUSB.flush();
}
void sendRealtime(uint8_t byte) {
  midiEventPacket_t rt = {0x0F, byte, 0x00, 0x00};
  MidiUSB.sendMIDI(rt);
  MidiUSB.flush();
}

// Initialize pins
void setupPins() {
  // Rows as inputs with pullups
  for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
    pinMode(ROW_PINS[r], INPUT_PULLUP);
  }
  // Columns as outputs, idle HIGH
  for (uint8_t c = 0; c < MATRIX_COLS; c++) {
    pinMode(COL_PINS[c], OUTPUT);
    digitalWrite(COL_PINS[c], HIGH);
  }
  // Extra buttons
  for (uint8_t i = 0; i < EXTRA_BTN_COUNT; i++) {
    pinMode(EXTRA_BUTTON_PINS[i], INPUT_PULLUP);
    extraState[i] = digitalRead(EXTRA_BUTTON_PINS[i]) == LOW;
    lastExtraState[i] = extraState[i];
    lastExtraChange[i] = millis();
  }
}

// Print pin mapping for verification (human readable)
void printPinMapping() {
  Serial.println(F("=== Pin mapping ==="));
  Serial.println(F("Rows:"));
  for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
    Serial.print(F("  Row "));
    Serial.print(r);
    Serial.print(F(" -> "));
    printPinName(ROW_PINS[r]);
    Serial.println();
  }

  Serial.println(F("Cols:"));
  for (uint8_t c = 0; c < MATRIX_COLS; c++) {
    Serial.print(F("  Col "));
    Serial.print(c);
    Serial.print(F(" -> "));
    printPinName(COL_PINS[c]);
    Serial.println();
  }

  Serial.print(F("Pitch X -> "));
  printPinName(PITCH_X_PIN);
  Serial.println();
  Serial.print(F("Pitch Y -> "));
  printPinName(PITCH_Y_PIN);
  Serial.println();

  Serial.println(F("Extra buttons:"));
  for (uint8_t i = 0; i < EXTRA_BTN_COUNT; i++) {
    Serial.print(F("  Btn "));
    Serial.print(i);
    Serial.print(F(" -> "));
    printPinName(EXTRA_BUTTON_PINS[i]);
    Serial.println();
  }
  Serial.println(F("==================="));
}

// Matrix scan
void scanMatrix() {
  for (uint8_t c = 0; c < MATRIX_COLS; c++) {
    // Drive this column low
    digitalWrite(COL_PINS[c], LOW);
    delayMicroseconds(SCAN_DELAY_US);
    // Read rows
    for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
      bool pressed = (digitalRead(ROW_PINS[r]) == LOW); // active low
      int id = idx(r, c);
      if (pressed != lastKeyState[id]) {
        lastChange[id] = millis();
        lastKeyState[id] = pressed;
      } else {
        // stable beyond debounce?
        if ((millis() - lastChange[id]) > DEBOUNCE_MS) {
          if (keyState[id] != pressed) {
            keyState[id] = pressed;
            // state changed - act on press/release
            uint8_t note = BASE_NOTE + id;
            if (pressed) {
              sendNoteOn(MIDI_CHANNEL, note, NOTE_VELOCITY);
            } else {
              sendNoteOff(MIDI_CHANNEL, note, NOTE_VELOCITY);
            }
          }
        }
      }
    }
    // Release column
    digitalWrite(COL_PINS[c], HIGH);
  }
}

// Extra buttons handling
void scanExtraButtons() {
  for (uint8_t i = 0; i < EXTRA_BTN_COUNT; i++) {
    bool pressed = (digitalRead(EXTRA_BUTTON_PINS[i]) == LOW);
    if (pressed != lastExtraState[i]) {
      lastExtraChange[i] = millis();
      lastExtraState[i] = pressed;
    } else {
      if ((millis() - lastExtraChange[i]) > DEBOUNCE_MS) {
        if (extraState[i] != pressed) {
          extraState[i] = pressed;
          // on press only
          if (pressed) {
            switch (i) {
              case 0: // Intro -> Start
              case 1: // Play -> Start
                sendRealtime(0xFA); // MIDI Start
                break;
              case 2: // Stop
                sendRealtime(0xFC); // MIDI Stop
                break;
              case 3: // Bank+
                if (currentBank < 127) currentBank++;
                sendCC(MIDI_CHANNEL, BANK_CC, currentBank);
                break;
              case 4: // Bank-
                if (currentBank > 0) currentBank--;
                sendCC(MIDI_CHANNEL, BANK_CC, currentBank);
                break;
              case 5: // Sustain toggle
                sustainOn = !sustainOn;
                sendCC(MIDI_CHANNEL, SUSTAIN_CC, sustainOn ? 127 : 0);
                break;
            }
          }
        }
      }
    }
  }
}

// Read pitch axes and send MIDI
static uint16_t lastPitchX = 0xFFFF;
static uint8_t lastPitchY = 0xFF;
void handlePitchAxes() {
  // analogRead range on STM32 Arduino core is 0..4095 (12-bit)
  uint16_t rawX = analogRead(PITCH_X_PIN);
  uint16_t rawY = analogRead(PITCH_Y_PIN);
  // map X to 0..16383 (14-bit) for Pitch Bend
  uint16_t bend = (uint32_t)rawX * 16383 / 4095;
  // map Y to 0..127 for CC1
  uint8_t cc1 = (uint32_t)rawY * 127 / 4095;

  // Only send updates when changed beyond small threshold to limit USB traffic
  if (abs((int)lastPitchX - (int)bend) > 4 || lastPitchX == 0xFFFF) {
    sendPitchBend(MIDI_CHANNEL, bend);
    lastPitchX = bend;
  }
  if (abs((int)lastPitchY - (int)cc1) > 1 || lastPitchY == 0xFF) {
    sendCC(MIDI_CHANNEL, 1, cc1); // CC1 = Modulation
    lastPitchY = cc1;
  }
}

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println(F("STM32F103C8T6 8x8 USB-MIDI controller starting..."));

  setupPins();

  // initialize state arrays
  for (int i = 0; i < MATRIX_ROWS * MATRIX_COLS; i++) {
    keyState[i] = false;
    lastKeyState[i] = false;
    lastChange[i] = millis();
  }

  printPinMapping();
}

void loop() {
  scanMatrix();
  scanExtraButtons();
  handlePitchAxes();

  // Small idle delay
  delay(2);
}
