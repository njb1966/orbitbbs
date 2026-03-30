#!/usr/bin/env bash
# deploy_feeds.sh — Copy feed files to DOS FAT image using mtools.
#
# Uses 'mtools' to write directly to the FAT image without mounting it,
# so this is safe to run while the QEMU VM is running.
#
# Prerequisites:
#   sudo apt install mtools
#   Run create_feeds_img.sh once first to create the image.
#
# Cron example (every 6 hours, 5 min after fetch):
#   5 */6 * * * /path/to/scripts/fetch_feeds.sh

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
STAGING_DIR="$PROJECT_DIR/feeds"
FEEDS_IMG="$PROJECT_DIR/feeds.img"
MTOOLSRC="$SCRIPT_DIR/mtoolsrc"

if [ ! -f "$FEEDS_IMG" ]; then
    echo "ERROR: $FEEDS_IMG not found. Run create_feeds_img.sh first."
    exit 1
fi

if ! command -v mcopy &>/dev/null; then
    echo "ERROR: mtools not installed. Run: sudo apt install mtools"
    exit 1
fi

export MTOOLSRC="$MTOOLSRC"

echo "Deploying feeds to $FEEDS_IMG"
echo

# Copy ANSI feed files to D:\FEEDS\ in the FAT image
for f in "$STAGING_DIR"/*.ANS; do
    fname="$(basename "$f")"
    echo "  mcopy $fname -> d:feeds/$fname"
    mcopy -o -i "$FEEDS_IMG" "$f" "::FEEDS/$fname"
done

# Copy GFL index to D:\ root (to be placed in DATA\ on DOS manually, or via batch)
if [ -f "$STAGING_DIR/FEEDS.GFL" ]; then
    echo "  mcopy FEEDS.GFL -> d:/"
    mcopy -o -i "$FEEDS_IMG" "$STAGING_DIR/FEEDS.GFL" "::"
fi

echo
echo "Done. DOS drive D: updated."
