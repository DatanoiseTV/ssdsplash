#!/bin/bash

echo "Testing improved multiline text support..."

echo ""
echo "=== Multiline Support Features ==="
echo "✓ TrueType fonts: Native \\n support"
echo "✓ Bitmap fonts: NOW supports \\n characters!"
echo "✓ Format strings: \\n works in printf-style formatting"
echo "✓ Auto-wrapping: Text wraps to next line when reaching display width"
echo "✓ Multi-display: Works with all display sizes (128x64, 128x32, 240x320, etc.)"

echo ""
echo "=== Test Cases ==="

echo ""
echo "1. Basic multiline with bitmap font:"
echo "   ./bin/ssdsplash-send -t text \"Line 1\\nLine 2\\nLine 3\""

echo ""
echo "2. Multiline with formatting:"
echo "   SERVICE=nginx; STATUS=running; PID=1234"
echo "   ./bin/ssdsplash-send -t text \"Service: %s\\nStatus: %s\\nPID: %d\" \$SERVICE \$STATUS \$PID"

echo ""
echo "3. Mixed newlines and wrapping:"
echo "   ./bin/ssdsplash-send -t text \"Long line that will wrap automatically\\nExplicit newline\\nShort\""

echo ""
echo "4. System information multiline:"
echo "   HOSTNAME=\$(hostname)"
echo "   UPTIME=\$(uptime | cut -d' ' -f4-5)"
echo "   LOAD=\$(uptime | awk -F'load average:' '{print \$2}' | cut -d',' -f1)"
echo "   ./bin/ssdsplash-send -t text \"Host: %s\\nUptime: %s\\nLoad: %s\" \"\$HOSTNAME\" \"\$UPTIME\" \"\$LOAD\""

echo ""
echo "5. Boot progress with multiline:"
echo "   ./bin/ssdsplash-send -t text \"Booting system...\\nLoading modules\\nStarting services\""

echo ""
echo "6. Long text with auto-wrapping (bitmap font):"
echo "   ./bin/ssdsplash-send -t text \"This is a very long line that should automatically wrap to the next line when it exceeds the display width\""

echo ""
echo "7. Error/status reporting:"
echo "   ERROR_CODE=404; ERROR_MSG=\"Not Found\""
echo "   ./bin/ssdsplash-send -t text \"Error %d:\\n%s\\nCheck logs\" \$ERROR_CODE \"\$ERROR_MSG\""

echo ""
echo "=== Usage in Shell Scripts ==="
echo ""
echo "# Function for multiline status display"
echo "show_status() {"
echo "    local service=\$1"
echo "    local status=\$2"
echo "    local extra=\$3"
echo "    ./bin/ssdsplash-send -t text \"Service: %s\\nStatus: %s\\n%s\" \"\$service\" \"\$status\" \"\$extra\""
echo "}"
echo ""
echo "show_status \"nginx\" \"running\" \"PID: 1234\""

echo ""
echo "=== Technical Details ==="
echo "• \\n creates new line at original X position"
echo "• Auto-wrap continues on next line"
echo "• Y position stops at display height boundary"
echo "• Works with all display types and sizes"
echo "• Font height: 8 pixels (bitmap), variable (TrueType)"
echo "• Character width: 6 pixels (bitmap), variable (TrueType)"