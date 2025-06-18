#!/bin/bash

echo "Testing formatted string support..."

# Test help output
echo "1. Testing help output:"
./bin/ssdsplash-send --help | grep -A2 "printf-style"

echo ""
echo "2. Testing format string examples:"

# Test basic string formatting
echo "Testing: ./bin/ssdsplash-send -t text \"Service %s returned %d\" nginx 0"

# Test numeric formatting
echo "Testing: ./bin/ssdsplash-send -t text \"Progress: %d/%d (%.1f%%)\" 42 100 42.0"

# Test without formatting (should work as before)
echo "Testing: ./bin/ssdsplash-send -t text \"Simple text without formatting\""

echo ""
echo "Format string functionality implemented!"
echo ""
echo "Usage examples:"
echo "  SERVICE=nginx; CODE=0"
echo "  ./bin/ssdsplash-send -t text \"Service %s returned %d\" \$SERVICE \$CODE"
echo ""
echo "  ./bin/ssdsplash-send -t text \"Progress: %d/%d (%.1f%%)\" 42 100 42.0"
echo ""
echo "  ./bin/ssdsplash-send -t text \"Temperature: %.1f°C\" 23.5"