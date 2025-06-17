#!/bin/bash

set -e

echo "Installing SSD1306 Splash Screen..."

# Check if running as root
if [ "$EUID" -ne 0 ]; then
    echo "Please run as root (use sudo)"
    exit 1
fi

# Build the project
echo "Building..."
make clean
make

# Install binaries
echo "Installing binaries..."
install -D bin/ssdsplash /usr/bin/ssdsplash
install -D bin/ssdsplash-send /usr/bin/ssdsplash-send

# Install systemd service
echo "Installing systemd service..."
install -D systemd/ssdsplash.service /etc/systemd/system/ssdsplash.service

# Reload systemd
systemctl daemon-reload

echo "Installation complete!"
echo ""
echo "To enable at boot:"
echo "  systemctl enable ssdsplash"
echo ""
echo "To start now:"
echo "  systemctl start ssdsplash"
echo ""
echo "To test:"
echo "  ssdsplash-send -t text 'Hello World!'"