#!/bin/bash
# deploy.sh — Deploy OrbitBBS to the production VM
#
# Copies BBS.EXE and all gfiles/ content into the QEMU disk image
# in a single guestfish session (mounts disk once).
#
# Run from the project root on the build machine (debian12).
#
# Usage:
#   ./deploy.sh
#   ORBIT_HOST=192.168.0.231 ./deploy.sh   # override host
#
# Prerequisites:
#   - SSH key access to ORBIT_HOST
#   - guestfish installed on ORBIT_HOST
#   - orbitbbs.service managed by systemd on ORBIT_HOST

set -e

ORBIT_HOST="${ORBIT_HOST:-100.118.30.47}"
ORBIT_USER="${ORBIT_USER:-nick}"
DISK_IMAGE="/home/${ORBIT_USER}/Projects/Code_Projects/orbitbbs/base-dos.qcow2"
BBS_EXE="wwivs424/exe/BBS.EXE"
GFILES_DIR="gfiles"

SSH="ssh ${ORBIT_USER}@${ORBIT_HOST}"

echo "==> OrbitBBS deploy → ${ORBIT_USER}@${ORBIT_HOST}"

# Sanity checks
[ -f "$BBS_EXE" ]   || { echo "ERROR: $BBS_EXE not found. Build first."; exit 1; }
[ -d "$GFILES_DIR" ] || { echo "ERROR: $GFILES_DIR/ not found."; exit 1; }

# Stage files on the remote
echo "--> Uploading BBS.EXE and gfiles/..."
$SSH "rm -rf /tmp/orbit_deploy && mkdir -p /tmp/orbit_deploy/gfiles"
scp "$BBS_EXE"      "${ORBIT_USER}@${ORBIT_HOST}:/tmp/orbit_deploy/BBS.EXE"
scp ${GFILES_DIR}/* "${ORBIT_USER}@${ORBIT_HOST}:/tmp/orbit_deploy/gfiles/"

# Build guestfish command list
GF_SCRIPT="copy-in /tmp/orbit_deploy/BBS.EXE /ORBIT"
for f in ${GFILES_DIR}/*; do
  GF_SCRIPT="${GF_SCRIPT}"$'\n'"copy-in /tmp/orbit_deploy/gfiles/$(basename $f) /ORBIT/GFILES"
done

# Stop, inject, start — all via a single remote script
echo "--> Stopping orbitbbs..."
$SSH "sudo systemctl stop orbitbbs"

echo "--> Injecting into disk image..."
$SSH "bash -s" <<REMOTE
guestfish -a "${DISK_IMAGE}" -m /dev/sda1 <<'GF'
${GF_SCRIPT}
GF
REMOTE

echo "--> Starting orbitbbs..."
$SSH "sudo systemctl start orbitbbs && rm -rf /tmp/orbit_deploy"

echo
echo "==> Done. BBS live at orbitbbs.njb1966.com:2324"
