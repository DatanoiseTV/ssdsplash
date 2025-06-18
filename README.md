# ssdsplash

A lightweight splash screen daemon for OLED and TFT displays that can display boot progress and messages on embedded Linux systems like Raspberry Pi.

**Project URL:** https://github.com/DatanoiseTV/ssdsplash

## Features

- Pure userland implementation using I2C and SPI
- Support for multiple display types:
  - SSD1306 OLED displays (128x64 and 128x32) via I2C
  - SSH1106 OLED displays (128x64) via I2C
  - ILI9341 TFT displays (240x320) via SPI
- Unix socket communication for real-time updates
- Text display with multiple lines
- Built-in 5x7 bitmap font
- TrueType font support (.ttf files via stb_truetype)
- Configurable font sizes for TrueType fonts
- Progress bar with percentage
- Image display (PNG, JPEG, and other formats via stb_image)
- Automatic image scaling and centering
- Dithering for monochrome displays
- Early boot integration via systemd
- Graceful shutdown to free I2C/SPI for other applications
- Static linking for embedded deployment

## Supported Displays

| Display Type | Resolution | Interface | Notes |
|--------------|------------|-----------|-------|
| SSD1306      | 128x64     | I2C       | Most common OLED, default |
| SSD1306      | 128x32     | I2C       | Smaller OLED variant |
| SSH1106      | 128x64     | I2C       | Similar to SSD1306, different controller |
| ILI9341      | 240x320    | SPI       | Color TFT display |

## Hardware Requirements

- One of the supported displays (see table above)
- I2C connection (default: /dev/i2c-1) for OLED displays
- SPI connection (default: /dev/spidev0.0) for TFT displays
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
# For SSD1306 128x64 display (default)
sudo ssdsplash

# For SSD1306 128x32 display
sudo ssdsplash -t 128x32

# For SSH1106 128x64 display
sudo ssdsplash -t ssh1106

# For ILI9341 240x320 TFT display
sudo ssdsplash -t ili9341

# Custom I2C device
sudo ssdsplash -d /dev/i2c-0

# Custom I2C address (default: 0x3C)
sudo ssdsplash -a 0x3D

# Custom SPI device for ILI9341
sudo ssdsplash -t ili9341 -d /dev/spidev0.1

# Combine options
sudo ssdsplash -d /dev/i2c-1 -a 0x3D -t 128x32
```

### Command Line Options

```
  -d, --device DEVICE    Device path (default: /dev/i2c-1 for I2C, /dev/spidev0.0 for SPI)
  -a, --address ADDR     I2C address: 0x3C or 0x3D (default: 0x3C)
  -t, --type TYPE        Display type: 128x64, 128x32, ili9341, ssh1106 (default: 128x64)
  -h, --help             Show this help
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

# Shutdown daemon (frees I2C/SPI)
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

### For Raspberry Pi with SSD1306/SSH1106 (I2C):

```
Display  ->  Raspberry Pi
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

### For Raspberry Pi with ILI9341 (SPI):

```
ILI9341  ->  Raspberry Pi
VCC      ->  3.3V (Pin 1)
GND      ->  GND (Pin 6)
CS       ->  CE0 (Pin 24)
RESET    ->  GPIO25 (Pin 22)
DC       ->  GPIO24 (Pin 18)
SDI      ->  MOSI (Pin 19)
SCK      ->  SCLK (Pin 23)
LED      ->  3.3V (Pin 1)
SDO      ->  MISO (Pin 21)
```

Enable SPI:
```bash
sudo raspi-config
# Interface Options -> SPI -> Enable
```

## Configuration

The daemon supports:
- **Display types:** 128x64 (default), 128x32, ssh1106, ili9341
- **I2C devices:** /dev/i2c-1 (default), /dev/i2c-0, etc.
- **SPI devices:** /dev/spidev0.0 (default), /dev/spidev0.1, etc.
- **I2C addresses:** 0x3C (default), 0x3D
- **Fonts:** 
  - Built-in 5x7 bitmap font (default)
  - TrueType fonts (.ttf files) with configurable sizes
  - Automatic fallback to bitmap font if TrueType loading fails
- **Image formats:** PNG, JPEG, BMP, TGA, and others (via stb_image)
- **Image processing:** Automatic RGB to grayscale conversion with dithering (for monochrome displays)
- **Scaling:** Original size (centered) or scaled to fit display

## Display Type Details

### SSD1306 (128x64, 128x32)
- **Interface:** I2C
- **Colors:** Monochrome (black/white)
- **Common uses:** Small status displays, embedded systems
- **Addresses:** Usually 0x3C or 0x3D

### SSH1106 (128x64)
- **Interface:** I2C  
- **Colors:** Monochrome (black/white)
- **Notes:** Similar to SSD1306 but uses different controller chip
- **Differences:** Requires page-by-page data writing instead of bulk transfer

### ILI9341 (240x320)
- **Interface:** SPI
- **Colors:** 16-bit color (65,536 colors)
- **Common uses:** Larger displays, graphical interfaces
- **Notes:** Much higher resolution and color capability

## Testing

Use the included test script to verify display support:

```bash
./test-displays.sh
```

## Similar to psplash

Like psplash, ssdsplash provides:
- Daemon/client architecture
- Progress reporting
- Text messages
- Early boot capability
- Clean shutdown

## License

This project is provided as-is for educational and development purposes.