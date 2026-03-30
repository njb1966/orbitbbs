# OrbitBBS — Session Handoff
**Date:** 2026-03-29
**Session ended:** ~19:30 CDT

---

## 🟢 What Was Accomplished This Session

### Phase 5.5 — Custom Main Menu (`orbit_menu`) ✅
See previous session notes — completed before this session.

### Phase 6 — QEMU VM Setup ✅ (completed this session)

**6a — `run_bbs.sh`**
- Launches QEMU headless with `-serial pty`
- Auto-starts tcpser after detecting PTY
- `start` / `stop` / `status` / `setup` subcommands
- `setup` mode opens a GTK window with VGA display for sysop work

**6b — DOS CONFIG.SYS / AUTOEXEC.BAT**
- `CONFIG.SYS`: HIMEM.SYS, DOS=HIGH, FILES=40, BUFFERS=20, SHELL
- `AUTOEXEC.BAT`: SHARE → BNU.COM → `BBS.EXE /N1` → restart loop
- D: drive feed copy lines REM'd out (re-enable in Phase 8)

**6c — OrbitBBS installed on DOS VM**
- Files transferred via `qemu-nbd` mount (no floppy needed)
- Script: `scripts/make_install_img.sh` (creates FAT install disk — backup method)
- Directories created by INIT.EXE: `C:\ORBIT\DATA\`, `MSGS\`, `GFILES\`
- Sysop account created (Nick B, SL=255)

**6d — tcpser + telnet working**
- tcpser patched to not FATAL on PTY `TIOCMGET` (virtual devices have no modem lines)
  - `tcpser/src/serial.c`: FATAL → WARN, returns -1
  - `tcpser/src/bridge.c`: `ctrl_thread` exits gracefully instead of `exit(-1)`
- Key bug fixed: `BBS.EXE /N1 /M` — `/M` = **no modem** flag (opposite of intended)
  - Fixed to `BBS.EXE /N1` in AUTOEXEC.BAT
- tcpser runs with `-i "s0=1"` (auto-answer after 1 RING)
- `telnet localhost 2323` → OrbitBBS login screen ✅

**WELCOME.MSG**
- Block-letter ORBITBBS logo in yellow ANSI
- Cyan borders, green welcome text
- Stored as UTF-8 (works with modern telnet clients)
- Location: `C:\ORBIT\GFILES\WELCOME.MSG`

---

## 🔧 Key Technical Details

### tcpser PTY Patch
PTYs (`/dev/pts/N`) don't support `TIOCMGET`/`TIOCMSET` hardware modem ioctls.
Original tcpser called `exit(-1)` on failure. Patched to:
1. `serial.c` `ser_get_control_lines()`: WARN + return -1 (not FATAL)
2. `bridge.c` `ctrl_thread()`: return NULL instead of `exit(-1)`

DTR monitoring disabled (acceptable — tcpser still handles RING/CONNECT).

### `/M` Flag
In WWIV 4.24 BBS.EXE: `/M` = `ok_modem_stuff=0` (disables ALL modem/COM processing).
NOT "modem mode" as assumed. Remove it. BBS must run as just `BBS.EXE /N1`.

### QEMU Serial → tcpser → telnet Chain
```
BBS.EXE → BNU FOSSIL → COM1 UART → QEMU virtual UART → /dev/pts/N → tcpser → TCP:2323 → telnet
```
tcpser emulates Hayes modem (RING → ATA → CONNECT 38400) with `s0=1` auto-answer.

### File Transfer Method (Linux → DOS VM)
```bash
sudo qemu-nbd --connect=/dev/nbd0 base-dos.qcow2
sudo mount /dev/nbd0p1 /mnt/dos
# copy files
sudo umount /mnt/dos
sudo qemu-nbd --disconnect /dev/nbd0
```
VM must be STOPPED before mounting.

---

## ✅ Build & Runtime Status

| Artifact | Status |
|----------|--------|
| `wwivs424/exe/BBS.EXE` | Mar 29 16:41, 558,016 bytes — orbit_menu compiled in |
| `base-dos.qcow2` | OrbitBBS installed, sysop account created |
| `tcpser/tcpser` | Patched + rebuilt for PTY support |
| `run_bbs.sh` | Integrated QEMU + tcpser launch |
| `WELCOME.MSG` | Block-letter logo, ANSI colors |

**Test:** `./run_bbs.sh start && telnet localhost 2323` → login screen ✅

---

## 📍 Where We Are in the Project Roadmap

| Phase | Status | Description |
|-------|--------|-------------|
| 0 | ✅ Complete | DOSBox build environment |
| 1 | ✅ Complete | Branding: WWIV → OrbitBBS |
| 2 | ✅ Complete | AutoMessage removed |
| 3 | ✅ Complete | Chat with Sysop removed |
| 4 | ✅ Complete | File transfers removed |
| 5 | ✅ Complete | Feed reader (Linux scripts, ANSI formatter) |
| 5.5 | ✅ Complete | orbit_menu() custom main menu |
| **6** | **✅ Complete THIS SESSION** | **QEMU VM setup, OrbitBBS install, tcpser, telnet live** |
| 7 | 🔲 **NEXT** | Feed URL verification (3 broken URLs) |
| 8 | 🔲 Next | feeds.img creation + cron activation |
| 9 | 🔲 Future | Doors and games |

---

## 🚀 What to Do Next (Phase 7 + 8)

### Phase 7 — Fix Broken Feed URLs
Three feeds need new URLs (see `NEXT-STEPS.md` for details):
1. **Tildes.net** — `https://tildes.net/~tech.rss` returning 404
2. **Geminispace** — `https://portal.mozz.us/gemini/geminispace.info/new` returning 500
3. **512KB Club** — no GitHub releases feed; needs replacement

Script to update: `scripts/fetch_feeds.py`

### Phase 8 — Feeds Infrastructure
1. Create `feeds.img` (one-time): `scripts/create_feeds_img.sh`
2. Re-enable D: drive lines in `AUTOEXEC.BAT` (currently REM'd out)
3. Activate cron (see `NEXT-STEPS.md` for exact crontab entries)

---

## 📂 Key File Locations

| File | Purpose |
|------|---------|
| `run_bbs.sh` | Start/stop/status/setup — integrated QEMU + tcpser |
| `tcpser/tcpser` | Patched tcpser binary |
| `tcpser/src/serial.c` | PTY patch: TIOCMGET FATAL → WARN |
| `tcpser/src/bridge.c` | PTY patch: ctrl_thread graceful exit |
| `wwivs424/exe/BBS.EXE` | Current BBS binary (orbit_menu, no file xfer, no chat) |
| `wwivs424/MAKEFILE.MAK` | Build: `dosbox -conf build.conf` then `make -f makefile.mak` |
| `WELCOME.MSG` | Login welcome screen source (copy to `C:\ORBIT\GFILES\`) |
| `base-dos.qcow2` | MS-DOS 6.22 QEMU disk — OrbitBBS installed |
| `scripts/fetch_feeds.py` | RSS/Gemini feed fetcher |
| `scripts/deploy_feeds.sh` | Copies feeds to DOS FAT image |
| `scripts/make_install_img.sh` | Creates FAT install disk (backup transfer method) |
| `NEXT-STEPS.md` | Full Phase 7–9 specs |
| `PROJECT.md` | Architecture overview |

---

## 💬 Resuming Next Session

Say: **"Continuing OrbitBBS. Phase 6 complete and live. See SESSION-HANDOFF.md."**
