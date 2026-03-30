#!/bin/bash
# make_install_img.sh — Create a partitioned FAT16 disk image with all OrbitBBS install files
# Mount as -hdb in QEMU setup mode, then COPY D:\*.* from DOS.

set -e

PROJ=/home/nick/Projects/Code_Projects/orbitbbs
IMG="$PROJ/install.img"

# Partition starts at sector 2048 (1MB offset), sector size 512
PART_OFFSET=$((2048 * 512))

# 16MB image
dd if=/dev/zero of="$IMG" bs=1M count=16 status=none

# Write MBR + partition table: one primary FAT16 partition starting at sector 2048
echo "2048,,b" | sfdisk --no-reread "$IMG" 2>/dev/null

# Format the partition (FAT16) using mtools @@offset syntax
mformat -i "${IMG}@@${PART_OFFSET}" -F ::
echo "Created $IMG (16MB, FAT16 partition at offset $PART_OFFSET)"

mcopy_img() {
  MTOOLS_NO_VFAT=1 mcopy -i "${IMG}@@${PART_OFFSET}" "$1" ":/$2"
}

# EXEs
mcopy_img "$PROJ/wwivs424/exe/BBS.EXE"     "BBS.EXE"
mcopy_img "$PROJ/wwivs424/exe/RETURN.EXE"  "RETURN.EXE"
mcopy_img "$PROJ/wwivs424/exe/MINIESM.EXE" "MINIESM.EXE"
mcopy_img "$PROJ/wwivs424/exe/FIX.EXE"     "FIX.EXE"
mcopy_img "$PROJ/wwivs424/INIT.EXE"        "INIT.EXE"

# Support files
mcopy_img "$PROJ/wwivs424/ENGLISH.STR"     "ENGLISH.STR"
mcopy_img "$PROJ/wwivs424/INI.STR"         "INI.STR"
mcopy_img "$PROJ/wwivs424/SYSOPLOG.STR"    "SYSOPLOG.STR"
mcopy_img "$PROJ/wwivs424/HELP.MSG"        "HELP.MSG"
mcopy_img "$PROJ/wwivs424/MENUS.MSG"       "MENUS.MSG"
mcopy_img "$PROJ/wwivs424/MENUSANS.MSG"    "MENUSANS.MSG"
mcopy_img "$PROJ/wwivs424/MENUSLCL.MSG"    "MENUSLCL.MSG"
mcopy_img "$PROJ/wwivs424/MENUSSOF.MSG"    "MENUSSOF.MSG"
mcopy_img "$PROJ/wwivs424/QWK.MSG"         "QWK.MSG"
mcopy_img "$PROJ/wwivs424/MODEMS.MDM"      "MODEMS.MDM"
mcopy_img "$PROJ/wwivs424/REGIONS.DAT"     "REGIONS.DAT"
mcopy_img "$PROJ/wwivs424/WWIV.INI"        "WWIV.INI"

# BNU FOSSIL driver
mcopy_img "$PROJ/BNU170/BNU.COM"           "BNU.COM"

echo ""
echo "install.img contents:"
mdir -i "${IMG}@@${PART_OFFSET}" ::
echo ""
echo "Done. Boot with: ./run_bbs.sh setup"
echo "In DOS: COPY D:\\*.* C:\\ORBIT\\"
echo "        COPY D:\\BNU.COM C:\\BNU\\"
