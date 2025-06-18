#!/bin/bash

echo "Testing display support for different display types..."

echo "Testing SSD1306 128x64 (default):"
echo "./bin/ssdsplash -t 128x64 --help"
./bin/ssdsplash -t 128x64 --help | head -5

echo ""
echo "Testing SSD1306 128x32:"
echo "./bin/ssdsplash -t 128x32 --help"
./bin/ssdsplash -t 128x32 --help | head -5

echo ""
echo "Testing SSH1106 128x64:"
echo "./bin/ssdsplash -t ssh1106 --help"
./bin/ssdsplash -t ssh1106 --help | head -5

echo ""
echo "Testing ILI9341 240x320:"
echo "./bin/ssdsplash -t ili9341 --help"
./bin/ssdsplash -t ili9341 --help | head -5

echo ""
echo "All display types are supported!"
echo ""
echo "Usage examples:"
echo "  SSD1306 128x64: ./bin/ssdsplash -t 128x64 -d /dev/i2c-1 -a 0x3C"
echo "  SSD1306 128x32: ./bin/ssdsplash -t 128x32 -d /dev/i2c-1 -a 0x3C"
echo "  SSH1106 128x64: ./bin/ssdsplash -t ssh1106 -d /dev/i2c-1 -a 0x3C"
echo "  ILI9341 240x320: ./bin/ssdsplash -t ili9341 -d /dev/spidev0.0"