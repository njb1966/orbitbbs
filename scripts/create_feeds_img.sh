#!/usr/bin/env bash
# create_feeds_img.sh — One-time setup: create the feeds FAT12 disk image.
#
# Creates a 1.44MB FAT12 image that QEMU will present to DOS as drive D:.
# The FEEDS\ directory inside it holds the ANSI feed files.
#
# Prerequisites:
#   sudo apt install mtools dosfstools
#
# Run this ONCE before starting the QEMU VM for the first time.
# After this, use deploy_feeds.sh to update content.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
FEEDS_IMG="$PROJECT_DIR/feeds.img"
MTOOLSRC="$SCRIPT_DIR/mtoolsrc"

if [ -f "$FEEDS_IMG" ]; then
    echo "WARNING: $FEEDS_IMG already exists. Delete it first to recreate."
    exit 1
fi

MKFSFAT=$(command -v mkfs.fat 2>/dev/null || command -v /usr/sbin/mkfs.fat 2>/dev/null || echo "")
if [ -z "$MKFSFAT" ]; then
    echo "ERROR: dosfstools not installed. Run: sudo apt install dosfstools mtools"
    exit 1
fi

echo "Creating FAT12 image: $FEEDS_IMG (1.44MB)"

# Create a 1.44MB blank image
dd if=/dev/zero of="$FEEDS_IMG" bs=512 count=2880 status=none

# Format as FAT12
"$MKFSFAT" -F 12 -n "ORBITFEEDS" "$FEEDS_IMG"

echo "Creating FEEDS\ directory in image..."

export MTOOLSRC="$MTOOLSRC"
mmd -i "$FEEDS_IMG" "::FEEDS"

echo
echo "Image created: $FEEDS_IMG"
echo
echo "Add to your QEMU command line:"
echo "  -drive file=$FEEDS_IMG,format=raw,if=ide,index=1"
echo
echo "DOS will see it as D:. Add to AUTOEXEC.BAT on the DOS side:"
echo "  IF EXIST D:\\FEEDS\\HN.ANS COPY D:\\FEEDS\\*.ANS C:\\GFILES\\FEEDS\\"
echo "  IF EXIST D:\\FEEDS.GFL COPY D:\\FEEDS.GFL C:\\WWIV\\DATA\\"
echo
echo "Run fetch_feeds.py then deploy_feeds.sh to populate the image."
