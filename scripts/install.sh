#!/bin/bash
# Install Weird Drum module to Ableton Move via SSH
set -e

MOVE_IP="${1:-move.local}"
MODULE_DIR="$(cd "$(dirname "$0")/.." && pwd)/modules/weird_drum"

if [ ! -f "$MODULE_DIR/weird_drum.so" ]; then
    echo "Error: weird_drum.so not found. Run build.sh first."
    exit 1
fi

echo "Installing Weird Drum to $MOVE_IP..."
DEST="/data/UserData/move-anything/modules/audio_fx/weird_drum"
ssh root@$MOVE_IP "mkdir -p $DEST"
scp "$MODULE_DIR/weird_drum.so" "$MODULE_DIR/module.json" "$MODULE_DIR/ui_chain.js" root@$MOVE_IP:$DEST/
ssh root@$MOVE_IP "chown -R ableton:users $DEST"

echo "Done. Restart Move-Anything to load Weird Drum."
