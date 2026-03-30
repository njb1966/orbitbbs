#!/usr/bin/env python3
"""
Preview the OrbitBBS main menu as it will appear to telnet callers.
Run in any terminal to check rendering before QEMU VM is set up.

Usage:
  python3 scripts/preview_menu.py
"""

import sys

E  = "\x1b["
YW = f"{E}1;33m"   # bright yellow  — title + command keys
GN = f"{E}1;32m"   # bright green   — section headers
CN = f"{E}1;36m"   # bright cyan    — borders
RS = f"{E}0m"      # reset

color = sys.stdout.isatty()

def c(code):
    if color:
        sys.stdout.write(code)

def pl(s=""):
    print(s)

def row(left_key, left_desc, right_key=None, right_desc=None, pad=21):
    """Print one or two command entries on a line."""
    left = f"  {YW if color else ''}{left_key}{RS if color else ''}  {left_desc}"
    if right_key:
        # pad left side to fixed width (accounting for invisible ANSI bytes)
        visible_left = f"  {left_key}  {left_desc}"
        padding = " " * (pad - len(left_key) - len(left_desc))
        right = f"{YW if color else ''}{right_key}{RS if color else ''}  {right_desc}"
        print(left + padding + right)
    else:
        print(left)

def section(name):
    c(GN); print(f"  {name}"); c(RS)

pl()
c(CN); pl("==============================================================================")
c(YW); pl("  OrbitBBS  --  Main Menu")
c(CN); pl("==============================================================================")
c(RS); pl()

section("MESSAGES")
row("P", "Post a message",    "Q", "Quick scan (this sub)", pad=23)
row("S", "Scan messages",     "N", "New scan (all subs)",   pad=23)
row("Z", "Express scan",      "R", "Remove your post",      pad=23)
row("E", "Send email",        "M", "Read your mail",        pad=23)
row("K", "Delete old email",  "F", "Feedback to sysop",     pad=23)
pl()

section("COMMUNITY & ENTERTAINMENT")
row("G", "Feed reader",   ".", "Doors & games",  pad=23)
row("U", "User list",     "V", "Vote",           pad=23)
row("B", "BBS list",      "$", "Time bank",      pad=23)
row("*", "Sub-board list")
pl()

section("NAVIGATION")
row("] or +", "Next sub",        "[ or -", "Prev sub",         pad=25)
row("}",       "Next conf",       "{",       "Prev conf",       pad=25)
row("H",       "Hop to sub",      "J",       "Jump to conference", pad=25)
pl()

section("SETTINGS & INFO")
row("D", "Defaults",          "Y", "Your info",    pad=23)
row("I", "System info",       "L", "Last callers", pad=23)
row("X", "Toggle expert mode")
pl()

section("OTHER")
row("O",   "Logoff",       "/O",  "Instant logoff", pad=23)
row("CLS", "Clear screen", "VER", "Version info",   pad=23)
pl()

c(CN); pl("==============================================================================")
c(RS); pl()
