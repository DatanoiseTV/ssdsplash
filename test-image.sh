#!/bin/bash

echo "Testing SSD1306 Splash Image Display"
echo "====================================="

# Create a simple test image using ImageMagick (if available)
if command -v convert >/dev/null 2>&1; then
    echo "Creating test images..."
    
    # Create a simple 128x64 test image
    convert -size 128x64 xc:white \
        -fill black -pointsize 20 -gravity center \
        -annotate 0 "HELLO\nWORLD" \
        test-128x64.png
    
    # Create a larger test image for scaling
    convert -size 256x128 xc:white \
        -fill black -pointsize 30 -gravity center \
        -annotate 0 "SCALED\nIMAGE" \
        test-large.png
    
    echo "Test images created: test-128x64.png, test-large.png"
else
    echo "ImageMagick not found. Please create test images manually."
fi

echo ""
echo "To test image display:"
echo "1. Copy test images to Raspberry Pi"
echo "2. Start ssdsplash daemon: sudo systemctl start ssdsplash"
echo "3. Test commands:"
echo "   ssdsplash-send -t img test-128x64.png"
echo "   ssdsplash-send -t img -s test-large.png"
echo "   ssdsplash-send -t quit"