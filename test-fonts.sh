#!/bin/bash

echo "Testing SSD1306 Splash TrueType Font Support"
echo "============================================="

echo ""
echo "This script tests TrueType font functionality."
echo "You'll need a TTF font file for testing."
echo ""

# Common font locations on Linux systems
COMMON_FONTS=(
    "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"
    "/usr/share/fonts/TTF/arial.ttf"
    "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf"
    "/System/Library/Fonts/Arial.ttf"  # macOS
)

FONT_FOUND=""
for font in "${COMMON_FONTS[@]}"; do
    if [ -f "$font" ]; then
        FONT_FOUND="$font"
        echo "Found system font: $font"
        break
    fi
done

if [ -z "$FONT_FOUND" ]; then
    echo "No system TrueType fonts found in common locations."
    echo "Please specify a TTF font file path manually."
    echo ""
    echo "Usage examples:"
    echo "  ssdsplash-send -t text \"Bitmap Font Text\""
    echo "  ssdsplash-send -t text -f /path/to/font.ttf -z 12 \"Small TTF Text\""
    echo "  ssdsplash-send -t text -f /path/to/font.ttf -z 16 \"Medium TTF Text\""
    echo "  ssdsplash-send -t text -f /path/to/font.ttf -z 20 \"Large TTF Text\""
else
    echo ""
    echo "Test commands with found font:"
    echo "  ssdsplash-send -t text \"Bitmap Font\""
    echo "  ssdsplash-send -t text -f \"$FONT_FOUND\" -z 12 \"TTF Size 12\""
    echo "  ssdsplash-send -t text -f \"$FONT_FOUND\" -z 16 \"TTF Size 16\""
    echo "  ssdsplash-send -t text -f \"$FONT_FOUND\" -z 20 \"TTF Size 20\""
fi

echo ""
echo "Prerequisites:"
echo "1. Start daemon: sudo systemctl start ssdsplash"
echo "2. Run test commands above"
echo "3. Stop daemon: ssdsplash-send -t quit"