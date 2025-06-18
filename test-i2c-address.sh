#!/bin/bash

echo "Testing SSD1306 I2C Address Support"
echo "===================================="
echo ""
echo "This script helps test different I2C addresses."
echo ""

# Check if i2cdetect is available
if command -v i2cdetect >/dev/null 2>&1; then
    echo "Scanning I2C bus for devices..."
    echo "Running: i2cdetect -y 1"
    echo ""
    sudo i2cdetect -y 1
    echo ""
    echo "Look for addresses 3c or 3d in the output above."
else
    echo "i2cdetect not found. Install i2c-tools:"
    echo "  sudo apt-get install i2c-tools"
fi

echo ""
echo "Test commands:"
echo ""
echo "1. Default address (0x3C):"
echo "   sudo ssdsplash"
echo ""
echo "2. Alternate address (0x3D):"
echo "   sudo ssdsplash -a 0x3D"
echo ""
echo "3. With specific I2C bus and address:"
echo "   sudo ssdsplash -d /dev/i2c-1 -a 0x3D"
echo ""
echo "4. Test after starting:"
echo "   ssdsplash-send -t text \"Testing I2C 0x3D\""
echo "   ssdsplash-send -t quit"