#!/bin/bash

echo "Testing multiline text support..."

echo "Current multiline support analysis:"
echo "1. TrueType fonts: Support \\n characters (see truetype.c:101-106)"
echo "2. Bitmap fonts: Use -l parameter for line positioning"
echo ""

echo "Testing multiline text examples:"

echo "Test 1: Using line parameter for bitmap font"
echo "./bin/ssdsplash-send -t text -l 0 \"Line 1: Status OK\""
echo "./bin/ssdsplash-send -t text -l 1 \"Line 2: Loading...\""
echo "./bin/ssdsplash-send -t text -l 2 \"Line 3: Ready\""

echo ""
echo "Test 2: Using newlines in format strings"
echo "./bin/ssdsplash-send -t text \"First line\\nSecond line\\nThird line\""

echo ""
echo "Test 3: Multiline with formatting"
echo "SERVICE=nginx; STATUS=running; PID=1234"
echo "./bin/ssdsplash-send -t text \"Service: %s\\nStatus: %s\\nPID: %d\" \$SERVICE \$STATUS \$PID"

echo ""
echo "Current limitations:"
echo "- Bitmap font: Doesn't automatically handle \\n, needs -l parameter"
echo "- TrueType font: Handles \\n properly"
echo "- Printf formatting with \\n works for both (processed before display)"

echo ""
echo "Recommendations:"
echo "- Use TrueType fonts for automatic multiline support"
echo "- Use -l parameter with bitmap fonts for precise line control"
echo "- \\n in format strings works but rendering depends on font type"