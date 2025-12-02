# STM32F103C8T6 8x8 USB‑MIDI Controller (PlatformIO)

Proyek ini siap-compile dengan PlatformIO (framework Arduino) untuk STM32F103C8T6 (Blue Pill). Default mapping pin sudah ditentukan dan dicetak di Serial pada startup untuk verifikasi.

Ringkasan PIN yang digunakan (default):
- Matrix Rows (inputs, internal pull-up): PA0, PA1, PA2, PA3, PA4, PA5, PA6, PA7
- Matrix Cols (outputs, drive LOW to scan): PB3, PB4, PB5, PB6, PB7, PB8, PB9, PB10
- Pitch axes (ADC inputs): PB0 (X), PB1 (Y)
- Extra buttons (6): PB11, PB12, PB13, PB14, PB15, PA8

Penting:
- Jangan gunakan PA11 dan PA12 jika Anda ingin USB device (MIDI) bekerja. PA11 = USB D- , PA12 = USB D+.
- Jika ingin ubah wiring, edit file src/config.h (satu tempat pusat untuk pin mapping).

Fitur:
- 8x8 button matrix (Note On/Off)
- 6 extra buttons (transport/Bank/Sustain)
- Two analog axes: X -> Pitch Bend (14-bit), Y -> CC1 (0..127)
- USB-MIDI output (class-compliant attempted via MIDIUSB)
- PlatformIO environment: bluepill_f103c8

Build & flash:
1. Install PlatformIO (VSCode atau CLI).
2. pio run
3. pio run -t upload (gunakan ST-Link atau uploader Anda)

Debug / Verifikasi:
- Buka Serial Monitor (115200) setelah boot. Anda akan melihat daftar pin mapping seperti:
  Row 0 -> PA0
  ...
  Col 0 -> PB3
  Pitch X -> PB0
  Extra Btn 0 -> PB11
- Jika device tidak muncul sebagai MIDI di host:
  - Cek apakah core/board mendukung USB device profile untuk MIDI.
  - Coba board yang known-good untuk USB MIDI (Teensy, RP2040 dengan TinyUSB, SAMD21, STM32F4).

Modifikasi:
- Ubah pin di src/config.h
- Ubah BASE_NOTE / MIDI_CHANNEL / NOTE_VELOCITY juga di src/config.h

License: MIT
