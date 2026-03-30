# OrbitBBS — Session Handoff
**Date:** 2026-03-30
**Session ended:** ongoing

---

## 🟢 What Was Accomplished This Session

### Feature Validation Fixes (Phase 10 partial) ✅

Four bugs found during validation, all fixed:

| # | Issue | Fix | File |
|---|-------|-----|------|
| 1 | G → "No Gfiles Section Available" | Config via `//GE` + feed deployment fix | GFILES.C, deploy_feeds.sh |
| 2 | User list ANSI misaligned | Replaced WWIV internal color codes with ANSI escapes | MISCCMD.C |
| 3 | Defaults screen shows WWIV Reg # | Removed display and edit (`case 'W'` / `enter_regnum`) | DEFAULTS.C |
| 4 | BBS List phone-only entry | Accepts any telnet address (50-char, no format check) | MISCCMD.C |

### Feed Deployment Rewrite ✅

`deploy_feeds.sh` rewritten: stops VM, injects directly into qcow2 via guestfish,
restarts (~30s downtime per 6h cron cycle). Floppy emulation approach abandoned
— MS-DOS 6.22 under QEMU does not reliably read raw FAT12 images as a hard disk,
and the floppy path (`-fda`) had unreliable DOS read behavior.

Files land at:
- `C:\ORBIT\GFILES\FEEDS\` — ANS display files
- `C:\ORBIT\DATA\FEEDS.GFL` — file index

### Feed Pagination Fix ✅

**Root cause:** `read_message1()` reset `lines_listed=0` on every ESC byte
(0x1B). Feed files use ANSI color codes on every line, so the page counter
never reached `screenlinest`. Removed the reset in MSGBASE.C.

Also added forced `sysstatus_pause_on_page` in `gfile_sec()` so feed content
always pages regardless of user's personal setting.

---

## 📍 Phase Roadmap

| Phase | Status | Description |
|-------|--------|-------------|
| 0–8.5 | ✅ Complete | Build env, branding, feature removal, feeds, QEMU, VM |
| 9 | ✅ Complete | ANSI screens (file-driven: drop files in GFILES\) |
| **10** | 🔄 **In Progress** | Feature validation |
| 11 | 🔲 Next | VPS migration (bbs.deadparrotbbs.com, Debian 12 ready) |
| 12 | 🔲 Future | FidoNet (after VPS — needs public IP for node application) |
| 13 | 🔲 Future | Doors/games (LORD, TradeWars, Usurper) |

---

## ✅ Phase 10 — Validation Checklist

### Tested and working
- [x] G → Feeds → file list displayed
- [x] G → Feeds → select feed → content displays with pagination
- [x] U → user list renders with correct ANSI alignment
- [x] D → Defaults screen (no WWIV Reg # field)
- [x] B → BBS List accepts telnet address

### Still to validate
- [ ] Messages: P, S, N, Q, Z, R, E, M, K, F
- [ ] Sub navigation: ], [, H, J
- [ ] Settings: Y, I, X
- [ ] Last callers: L
- [ ] Logoff: O, /O
- [ ] Sysop commands: //UE, //BE, //CE, //DOS, //GE
- [ ] New user registration flow end-to-end
- [ ] VER command (no WWIV artifacts)

---

## 🔧 Key Technical Details

### BBS.EXE History (this session)
| Size | Notes |
|------|-------|
| 557,744 | DEFAULTS.C + MISCCMD.C fixes (reg#, user list, bbslist) |
| 557,792 | GFILES.C: forced pause in gfile_sec() |
| (same build) | MSGBASE.C: removed lines_listed=0 reset on ESC |

Final: `557,792` bytes (last build includes all changes)

### Feed Deployment
```bash
python3 scripts/fetch_feeds.py && bash scripts/deploy_feeds.sh
```
Cron: fetch 0 */6, deploy 5 */6. deploy_feeds.sh handles stop/inject/start.

### Gfiles Section Config (done via //GE)
Section name: "Feeds", filename: "FEEDS", SL: 0
→ reads DATA\FEEDS.GFL, serves files from GFILES\FEEDS\

### AUTOEXEC.BAT (current — no floppy lines)
```bat
@ECHO OFF
SET ORBIT_INSTANCE=1
SET ORBIT_DIR=C:\ORBIT
C:\DOS\SHARE.EXE /F:4096 /L:40
C:\BNU\BNU.COM
CD C:\ORBIT
:TOP
BBS.EXE /N1
IF ERRORLEVEL 1 GOTO TOP
```

---

## 🚀 What to Do Next

**Phase 10 — Complete validation** (see checklist above)

**Phase 11 — VPS Migration**
- VPS: Debian 12, bbs.deadparrotbbs.com (ready)
- Copy: qcow2, run_bbs.sh, scripts/, tcpser, feeds.img
- Update paths in run_bbs.sh
- Open ports: 2323 (telnet), 24554 (binkp/FidoNet future)
- Configure systemd service for auto-start
- Test telnet from outside

**Phase 12 — FidoNet** (after VPS)
- Install binkd + HPT on VPS
- Apply for node number at bbs.deadparrotbbs.com:24554

## 💬 Resuming Next Session

Say: **"Continuing OrbitBBS. Phase 10 validation in progress. See SESSION-HANDOFF.md."**
