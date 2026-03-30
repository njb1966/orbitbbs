#!/usr/bin/env bash
# deploy_feeds.sh — Inject feed files directly into the DOS VM disk image.
#
# Stops the running QEMU VM, injects ANS files and FEEDS.GFL into the
# qcow2 image via guestfish, then restarts the VM.  Brief downtime
# (~30s) is acceptable for a 6-hour cron cycle.
#
# Prerequisites:
#   sudo apt install guestfish
#
# Cron example (every 6 hours, 5 min after fetch):
#   5 */6 * * * /path/to/orbitbbs/scripts/deploy_feeds.sh

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
STAGING_DIR="$PROJECT_DIR/feeds"
QCOW2="$PROJECT_DIR/base-dos.qcow2"
RUN_BBS="$PROJECT_DIR/run_bbs.sh"

if [ ! -f "$QCOW2" ]; then
    echo "ERROR: $QCOW2 not found."
    exit 1
fi

if ! command -v guestfish &>/dev/null; then
    echo "ERROR: guestfish not installed. Run: sudo apt install guestfish"
    exit 1
fi

echo "Deploying feeds to $QCOW2"
echo

# Stop VM if running
if "$RUN_BBS" status 2>/dev/null | grep -q RUNNING; then
    echo "Stopping OrbitBBS VM..."
    "$RUN_BBS" stop
    sleep 2
    WAS_RUNNING=1
else
    WAS_RUNNING=0
fi

# Inject files via guestfish
echo "Injecting feed files..."
guestfish -a "$QCOW2" <<'GFEOF'
run
mount /dev/sda1 /
GFEOF

# Build guestfish commands for each ANS file
GF_CMDS="run\nmount /dev/sda1 /\n"
for f in "$STAGING_DIR"/*.ANS; do
    fname="$(basename "$f")"
    echo "  $fname -> /ORBIT/GFILES/FEEDS/$fname"
    GF_CMDS+="copy-in $f /ORBIT/GFILES/FEEDS/\n"
done

if [ -f "$STAGING_DIR/FEEDS.GFL" ]; then
    echo "  FEEDS.GFL -> /ORBIT/DATA/FEEDS.GFL"
    GF_CMDS+="copy-in $STAGING_DIR/FEEDS.GFL /ORBIT/DATA/\n"
fi

printf "%b" "$GF_CMDS" | guestfish -a "$QCOW2"

echo
echo "Injection complete."

# Restart VM if it was running
if [ "$WAS_RUNNING" -eq 1 ]; then
    echo "Restarting OrbitBBS VM..."
    "$RUN_BBS" start
fi

echo "Done."
