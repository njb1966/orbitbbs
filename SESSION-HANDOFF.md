# OrbitBBS — Session Handoff
**Date:** 2026-03-29
**Session ended:** ~21:00 CDT

---

## 🟢 What Was Accomplished This Session

### Phase 6 — QEMU VM Setup ✅ (completed previous session)
See previous session notes.

### Registration Cleanup ✅ (completed this session)

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
| **7** | 🔲 **NEXT** | Feed URL verification (3 broken URLs) |
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

## 💬 Resuming Next Session

Say: **"Continuing OrbitBBS. Phase 6.5 complete. See SESSION-HANDOFF.md."**
