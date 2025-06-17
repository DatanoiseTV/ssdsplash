#!/bin/bash

set -e

echo "Installing SSD1306 Splash Screen for Buildroot/SysV..."

# Check if running as root
if [ "$EUID" -ne 0 ]; then
    echo "Please run as root (use sudo)"
    exit 1
fi

# Build the project
echo "Building..."
make clean
make

# Install binaries and SysV init script
echo "Installing binaries and SysV init script..."
make install-sysv

echo "Installation complete!"
echo ""
echo "To enable at boot (SysV):"
echo "  update-rc.d S30ssdsplash defaults"
echo ""
echo "To start now:"
echo "  /etc/init.d/S30ssdsplash start"
echo ""
echo "To test:"
echo "  ssdsplash-send -t text 'Hello Buildroot!'"