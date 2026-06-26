# NRF Pico Receiver

A Raspberry Pi Pico project that initializes and communicates with an nRF24L01+ radio module over SPI. The firmware configures the module for receive mode and transmit, listens on pipe 0, and prints status information over USB stdio.

## Project Structure

- `CMakeLists.txt` - Pico SDK project configuration and build settings.
- `main.c` - Application entry point, initialization, and main loop.
- `nrf.c` - nRF24L01 register access, configuration, transmit/receive helper functions.
- `nrf.h` - nRF24L01 register definitions and public function prototypes.
- `build/` - Generated build output and compiled firmware.

## Hardware

### Required

- Raspberry Pi Pico (or compatible RP2040 board)
- nRF24L01+ radio module
- 3.3V power supply for the nRF24L01+
- SPI wiring between Pico and nRF24L01+

### Default GPIO pin mapping

- `CE`   -> GPIO 16
- `CSN`  -> GPIO 17
- `SCK`  -> GPIO 18
- `MOSI` -> GPIO 19
- `MISO` -> GPIO 20

> Ensure the nRF24L01+ module is powered from 3.3V and not 5V.

## Features

- Initializes SPI on `spi0`
- Configures nRF24L01+ on channel 2440 MHz (RF channel 76)
- Uses 5-byte address `"00001"` for both RX and TX pipes
- Enables auto-ack and RX pipe 0
- Enters PRX receive mode and continuously polls the `STATUS` register
- Supports basic payload read/write and TX/RX control functions

## Build Instructions

From the project root:

```bash
mkdir -p build
cd build
cmake .. -G Ninja
ninja
```

Alternatively, use the existing VS Code task:

- `Compile Project`

This produces the executable in `build/nrf.elf` and additional outputs in `build/`.

## Flashing

Use the provided VS Code `Run Project` task or flash manually with `picotool`:

```bash
~/.pico-sdk/picotool/2.2.0-a4/picotool/picotool load build/nrf.elf -fx
```

If using UF2, copy `build/nrf.uf2` to the Pico boot drive.

## Runtime

The firmware initializes the nRF24L01+ and prints:

- `Initializing NRF...`
- `Waiting for packets...`
- `STATUS = 0xXX`

If initialization fails, it enters a tight loop after printing `NRF not detected!`.

## Notes

- USB stdio is enabled, UART stdio is disabled by default.
- The current example only polls the STATUS register; additional packet handling can be added in `main.c`.
- Modify `nrf_configure()` in `nrf.c` to change RF channel, data rate, payload size, or address settings.
