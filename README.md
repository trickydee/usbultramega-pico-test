# Raspberry Pi Pico Test Repository

This is a test repository for experimenting with various Raspberry Pi Pico functions and peripherals.

## Overview

This repository contains test code and examples for working with the Raspberry Pi Pico microcontroller, including:

- **SSD1306 OLED Display** - I2C communication and image display
- **Image Display** - Converting and displaying bitmap images on OLED displays
- **I2C Communication** - Basic I2C setup and communication examples

## Project Structure

```
.
├── main.c                 # Main application code
├── ssd1306.c             # SSD1306 OLED display driver implementation
├── ssd1306.h             # SSD1306 OLED display driver header
├── atari_mono2_page.h    # Atari logo image data (120x64 pixels) - page-first format
├── image_data_raw.h      # Raw image data in row-first format (for reference)
├── font.h                # Font data for text rendering
├── CMakeLists.txt         # CMake build configuration
├── build.sh              # Build script
├── docs/
│   └── IMAGE_FORMAT_SPEC.txt  # Image format conversion documentation
└── README.md             # This file
```

## Features

### SSD1306 OLED Display Support

- Full SSD1306 driver implementation
- Page-first image format support
- Image display functionality
- Text rendering capabilities

### Image Display

The repository includes functionality to display images on SSD1306 OLED displays. Images must be converted to the page-first format (see `docs/IMAGE_FORMAT_SPEC.txt` for details).

**Current Example:** Atari logo (120x64 pixels)

## Building

### Prerequisites

- Raspberry Pi Pico SDK (included as submodule)
- ARM GCC toolchain
- CMake

### Build Steps

1. Initialize submodules:
   ```bash
   git submodule update --init --recursive
   ```

2. Build the project:
   ```bash
   ./build.sh
   ```

3. Flash to Pico:
   ```bash
   # Copy build-pico/serial_test.uf2 to the Pico's USB mass storage device
   ```

## Hardware Setup

### SSD1306 OLED Display

- **I2C Interface**
- **Pins:**
  - SDA: GPIO 8
  - SCL: GPIO 9
- **Address:** 0x3C (default)
- **Display:** 128x64 pixels

### Connections

```
Pico GPIO 8  → SSD1306 SDA
Pico GPIO 9  → SSD1306 SCL
Pico 3.3V    → SSD1306 VCC
Pico GND     → SSD1306 GND
```

## Image Format

Images must be converted to SSD1306 page-first format. See `docs/IMAGE_FORMAT_SPEC.txt` for detailed conversion instructions.

**Quick Summary:**
- Source format: Row-first, horizontal 1 bit per pixel
- Target format: Page-first, vertical 8 pixels per byte
- Conversion script: See documentation in `docs/IMAGE_FORMAT_SPEC.txt`

## Usage

The main application displays the Atari logo on the SSD1306 OLED display. Modify `main.c` to:

- Display different images
- Add text rendering
- Implement custom display patterns
- Test other Pico functions

## Documentation

- **Image Format Specification:** See `docs/IMAGE_FORMAT_SPEC.txt`
- **SSD1306 Driver:** See `ssd1306.h` for API documentation

## License

[Add your license here]

## Contributing

This is a test repository for experimentation. Feel free to use it as a reference or starting point for your own projects.

