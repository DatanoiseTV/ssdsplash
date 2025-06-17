# SSD1306 Splash Screen for Embedded Linux

A lightweight splash screen daemon for SSD1306 OLED displays (128x64 and 128x32) that can display boot progress and messages on embedded Linux systems like Raspberry Pi.

## Features

- Pure userland implementation using I2C
- Support for 128x64 and 128x32 SSD1306 displays
- Unix socket communication for real-time updates
- Text display with multiple lines
- Built-in 5x7 bitmap font
- TrueType font support (.ttf files via stb_truetype)
- Configurable font sizes for TrueType fonts
- Progress bar with percentage
- Image display (PNG, JPEG, and other formats via stb_image)
- Automatic image scaling and centering
- Dithering for monochrome display
- Early boot integration via systemd
- Graceful shutdown to free I2C for other applications
- Static linking for embedded deployment

## Hardware Requirements

- SSD1306 OLED display (128x64 or 128x32)
- I2C connection (default: /dev/i2c-1)
- Raspberry Pi or similar embedded Linux system

## Building

```bash
make
```

## Installation

### For systemd-based systems (Raspberry Pi OS, Ubuntu, etc.)
```bash
sudo make install
sudo systemctl enable ssdsplash
```

### For Buildroot/SysV init systems
```bash
sudo make install-sysv
# Or use the helper script:
sudo ./install-buildroot.sh
```

## Usage

### Starting the daemon manually

```bash
# For 128x64 display (default)
sudo ssdsplash

# For 128x32 display
sudo ssdsplash -t 128x32

# Custom I2C device
sudo ssdsplash -d /dev/i2c-0

# Custom I2C address (default: 0x3C)
sudo ssdsplash -a 0x3D

# Combine options
sudo ssdsplash -d /dev/i2c-1 -a 0x3D -t 128x32
```

### Sending commands

```bash
# Display text (bitmap font)
ssdsplash-send -t text "Loading configuration..."

# Display text with TrueType font
ssdsplash-send -t text -f /path/to/font.ttf -z 16 "Custom Font Text"

# Display text on specific line
ssdsplash-send -t text -l 1 "Starting services"

# Show progress (42 out of 100)
ssdsplash-send -t progress -v 42

# Show progress with custom maximum
ssdsplash-send -t progress -v 150 -m 200

# Display image (centered, original size)
ssdsplash-send -t img /path/to/logo.png

# Display image (scaled to fit screen)
ssdsplash-send -t img -s /path/to/splash.jpg

# Clear screen
ssdsplash-send -t clear

# Shutdown daemon (frees I2C)
ssdsplash-send -t quit
```

## Integration with Boot Scripts

Add to your boot scripts or systemd services:

```bash
# Early boot - show logo
ssdsplash-send -t img -s /etc/ssdsplash/boot-logo.png

# Show boot progress
ssdsplash-send -t text "Initializing hardware..."
ssdsplash-send -t progress -v 25

# During boot
ssdsplash-send -t text "Loading kernel modules..."
ssdsplash-send -t progress -v 75

# When ready to start main application
ssdsplash-send -t quit
```

## Init System Integration

### Systemd (Raspberry Pi OS, Ubuntu, etc.)
The service starts early in the boot process and can be controlled via:

```bash
sudo systemctl start ssdsplash
sudo systemctl stop ssdsplash
sudo systemctl status ssdsplash
```

### SysV Init (Buildroot, embedded systems)
Control via traditional init scripts:

```bash
sudo /etc/init.d/S30ssdsplash start
sudo /etc/init.d/S30ssdsplash stop
sudo /etc/init.d/S30ssdsplash restart
```

## Wiring

For Raspberry Pi with SSD1306:

```
SSD1306  ->  Raspberry Pi
VCC      ->  3.3V (Pin 1)
GND      ->  GND (Pin 6)
SCL      ->  SCL (Pin 5)
SDA      ->  SDA (Pin 3)
```

Enable I2C:
```bash
sudo raspi-config
# Interface Options -> I2C -> Enable
```

## Configuration

The daemon supports:
- Display types: 128x64 (default), 128x32
- I2C devices: /dev/i2c-1 (default), /dev/i2c-0, etc.
- I2C addresses: 0x3C (default), 0x3D
- Fonts: 
  - Built-in 5x7 bitmap font (default)
  - TrueType fonts (.ttf files) with configurable sizes
  - Automatic fallback to bitmap font if TrueType loading fails
- Image formats: PNG, JPEG, BMP, TGA, and others (via stb_image)
- Image processing: Automatic RGB to grayscale conversion with dithering
- Scaling: Original size (centered) or scaled to fit display

## Similar to psplash

Like psplash, ssdsplash provides:
- Daemon/client architecture
- Progress reporting
- Text messages
- Early boot capability
- Clean shutdown

## License

This project is provided as-is for educational and development purposes.