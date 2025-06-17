#!/bin/bash

set -e

HOST="syso@rpi5.local"
REMOTE_DIR="/tmp/ssd1306-splash"
PROJECT_DIR="$(dirname "$0")"

echo "Deploying SSD1306 Splash to $HOST..."

# Create remote directory
ssh $HOST "mkdir -p $REMOTE_DIR"

# Copy source files
echo "Copying files..."
rsync -av --delete \
    --exclude='.git' \
    --exclude='obj' \
    --exclude='bin' \
    "$PROJECT_DIR/" "$HOST:$REMOTE_DIR/"

# Build on remote
echo "Building on remote host..."
ssh $HOST "cd $REMOTE_DIR && make clean && make"

# Install (requires sudo)
echo "Installing on remote host..."
ssh $HOST "cd $REMOTE_DIR && sudo ./install.sh"

echo "Deployment complete!"
echo ""
echo "To test on $HOST:"
echo "  ssh $HOST"
echo "  sudo systemctl start ssdsplash"
echo "  ssdsplash-send -t text 'Hello from SSH!'"