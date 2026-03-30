# OrbitBBS — Session Handoff
**Date:** 2026-03-30
**Session ended:** ~08:30 CDT

---

## 🟢 What Was Accomplished This Session

### Phase 7 — Feed URL Fixes ✅ (completed this session)

Three broken feed URLs replaced in `scripts/fetch_feeds.py`:

| File | Was | Now |
|------|-----|-----|
| TILDES.ANS | tildes.net/~tech.rss (404/422) | drewdevault.com/blog/index.xml |
| GEMINI.ANS | portal.mozz.us Gemini proxy (500 + HTML) | 100r.co/links/rss.xml (Hundred Rabbits) |
| 512KB.ANS | github.com/kevquirk/512kb.club/releases.atom (empty) | solar.lowtechmagazine.com/posts/index.xml |

Also fixed duplicate `"type"` key bug on the 512KB entry.

All 5 feeds confirmed live: HN (30), Lobsters (25), DeVault (383 total, capped at 20), Hundred Rabbits (84 total, capped at 20), Low-tech Mag (3 — slow publisher, normal).

Commit: `107931e`

---

### Registration Cleanup ✅ (completed previous session)

**NEWUSER.C changes (recompiled + deployed):**

1. **Birth year — 4-digit YYYY**
   - Was: `input(ag,2)` + `y=atoi(ag)+1900` (2-digit, 1900-based)
   - Now: `input(ag,4)` + `y=atoi(ag)` (full 4-digit year)
   - Prompt changed to `"Year you were born (YYYY): "`
   - Removed the 1919 sentinel check (artifact of 2-digit input)
   - Review screen now shows `month/day/year+1900` for 4-digit display
   - Storage unchanged: `u->year = (unsigned char)(y-1900)` — `years_old()` still works

2. **Computer type — removed**
   - Removed `input_comptype()` call from registration flow
   - Removed comp_type display from review screen
   - Removed `case '7': input_comptype()` from edit menu

3. **Default transfer protocol — removed**
   - Removed `get_protocol(xf_down)` block entirely
   - File transfers were removed in Phase 4; this question was irrelevant

4. **Callsign — marked optional**
   - Added `"(Optional -- press Enter to skip)"` line before input prompt
   - HAM operators can still use the field; others skip it

### Menu Cleanup ✅

**MMENU.C changes (recompiled + deployed):**
- Removed `CLS` and `VER` from the orbit_menu display (OTHER section)
- Fixed `CLS` handler: `\f` → `\x1b[2J\x1b[H` (proper ANSI clear screen)
- `CLS` and `VER` still work as hidden typed commands

### build-auto.conf ✅
- Added unattended DOSBox build config (`build-auto.conf`)
- Runs `make -f makefile.mak > BUILD.LOG` then exits automatically
- Use: `dosbox -conf build-auto.conf -exit &`

---

## 🔧 Key Technical Details

### Sysop Commands (via telnet)
Log in as sysop (SL=255), then at the main menu prompt type `//` followed by the command:

| Command | What it does |
|---------|-------------|
| `//UE` or `//UEDIT` | User editor — edit SL, name, flags, validate accounts |
| `//CU` or `//CHUSER` | Change user context |
| `//YLOG` | Yesterday's activity log |
| `//DOS` | DOS shell (requires sysop password) |
| `//BE` | Board (sub) editor |
| `//CE` | Chain (door) editor |

The `//` prefix works because `mmkey()` detects the second `/` and switches to reading a full 50-char line, bypassing the single-char command dispatch.

Single-char commands (like `U` = user list) execute immediately; `//` is required for multi-char sysop commands.

### Birth Year Storage
`thisuser.year` is stored as `year - 1900` (unsigned char).
- 1985 stored as 85
- 2001 stored as 101
`years_old()` uses `today.da_year - 1900 - y` arithmetic — works correctly for 2000+ years.
4-digit input is just parsed directly; storage is unchanged.

### BBS.EXE Sizes (history)
| Date | Size | Notes |
|------|------|-------|
| Mar 29 16:41 | 558,016 | Phase 6 complete, orbit_menu |
| Mar 29 19:38 | 557,856 | Registration cleanup |
| Mar 29 19:53 | 557,680 | Menu cleanup (CLS/VER) |

---

## ✅ Build & Runtime Status

| Artifact | Status |
|----------|--------|
| `wwivs424/exe/BBS.EXE` | Mar 29 19:53, 557,680 bytes — all session changes compiled in |
| `base-dos.qcow2` | Updated with latest BBS.EXE |
| `tcpser/tcpser` | Patched + rebuilt for PTY support |
| `run_bbs.sh` | Integrated QEMU + tcpser launch |
| `WELCOME.MSG` | Block-letter logo, ANSI colors |
| `build-auto.conf` | Unattended build config |

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
| 6 | ✅ Complete | QEMU VM setup, OrbitBBS install, tcpser, telnet live |
| 6.5 | ✅ Complete THIS SESSION | Registration cleanup, menu cleanup |
| **7** | ✅ **DONE** | Feed URL verification — all 5 feeds live |
| **8** | ✅ **DONE** | feeds.img, AUTOEXEC.BAT D: drive, cron active |
| 9 | 🔲 Future | Doors and games |
| **9.5** | 🔲 **NEXT** | Custom ANSI screens (welcome, last callers header) |

---

## 🚀 What to Do Next (Phase 7 + 8)

### Phase 8 — Feeds Infrastructure ✅ (completed this session)

- `feeds.img` created (1.44MB FAT12, populated with all 5 feeds)
- `AUTOEXEC.BAT` updated: D: drive COPY lines un-REM'd
- `C:\ORBIT\GFILES\FEEDS\` directory created in qcow2
- Cron active: fetch every 6h, deploy 5min later → `logs/feeds.log`
- VM restarted with D: drive attached — confirmed no warnings
- `xvfb-run` confirmed working for headless DOSBox builds (CLI-safe)
- guestfish confirmed working for file injection (CLI-safe, no GTK needed)

### Phase 8 — Original notes
1. Create `feeds.img` (one-time): `scripts/create_feeds_img.sh`
2. Re-enable D: drive lines in `AUTOEXEC.BAT` (currently REM'd out)
3. Activate cron (see `NEXT-STEPS.md` for exact crontab entries)

---

## 📂 Key File Locations

| File | Purpose |
|------|---------|
| `run_bbs.sh` | Start/stop/status/setup — integrated QEMU + tcpser |
| `build-auto.conf` | Unattended DOSBox build (runs make then exits) |
| `tcpser/tcpser` | Patched tcpser binary |
| `wwivs424/NEWUSER.C` | Registration flow (4-digit year, no comptype, no defprot, optional callsign) |
| `wwivs424/MMENU.C` | Main menu display + command dispatch; orbit_menu() |
| `wwivs424/exe/BBS.EXE` | Current BBS binary |
| `wwivs424/MAKEFILE.MAK` | Build: `dosbox -conf build-auto.conf -exit` |
| `WELCOME.MSG` | Login welcome screen source |
| `base-dos.qcow2` | MS-DOS 6.22 QEMU disk — OrbitBBS installed |
| `scripts/fetch_feeds.py` | RSS/Gemini feed fetcher |
| `scripts/deploy_feeds.sh` | Copies feeds to DOS FAT image |
| `NEXT-STEPS.md` | Full Phase 7–9 specs |
| `PROJECT.md` | Architecture overview |

---

## 🚀 Remaining Work (pre-VPS)

### Phase 9 — ANSI Screens
Design in Moebius or PabloDraw (CP437), inject via guestfish into `C:\ORBIT\GFILES\`.
No recompile needed.

| File | Notes |
|------|-------|
| `MAINMENU.ANS` | Main menu — replaces hardcoded orbit_menu() fallback |
| `MAINMENU.MSG` | Plain terminal fallback |
| `WELCOME.ANS` | Pre-login screen |
| `LOGON.ANS` | Post-login screen |
| `LOGOFF.ANS` | Logout screen |
| `NEWUSER.ANS` | New user registration screen |

### Phase 10 — Feature Validation
Test every menu command end-to-end via telnet:
- [ ] Messages: P, S, N, Q, Z, R, E, M, K, F
- [ ] Feed reader: G → browse feeds
- [ ] User list: U
- [ ] Sub navigation: ], [, H, J
- [ ] Settings: D, Y, I, X
- [ ] Last callers: L
- [ ] Logoff: O, /O
- [ ] Sysop commands: //UE, //BE, //CE, //DOS
- [ ] New user registration flow
- [ ] VER command (verify no WWIV artifacts)

### Phase 11 — VPS Migration
VPS: Debian 12, bbs.deadparrotbbs.com (ready)
- Copy qcow2, feeds.img, run_bbs.sh, scripts/, tcpser to VPS
- Update paths in run_bbs.sh (PROJ dir)
- Open ports: 2323 (telnet), 24554 (binkp/FidoNet future)
- Configure systemd service for auto-start
- Test telnet from outside

### Phase 12 — FidoNet (after VPS)
- Install binkd + HPT on VPS
- Apply for node number (bbs.deadparrotbbs.com, port 24554)
- Configure echomail areas

## 💬 Resuming Next Session

Say: **"Continuing OrbitBBS. Phase 8.5 complete. See SESSION-HANDOFF.md."**
