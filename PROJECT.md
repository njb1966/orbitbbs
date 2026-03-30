# OrbitBBS

A fork of WWIV BBS v4.24a targeting MS-DOS 6.22, designed for the small web
and Geminispace community. Single-instance, retro feel, no file transfers.

**Live at:** `telnet bbs.deadparrotbbs.com 2323` (after VPS migration)
**Dev:** `telnet localhost 2323`

---

## What It Is

OrbitBBS is not WWIV. Like Telegard and Renegade before it, it uses WWIV 4.24a
as a codebase and diverges from there. All WWIV branding has been replaced.
The focus is message bases, doors/games, and a curated feed reader — not file
trading or large-scale multi-node operation.

**Target audience:** Small web enthusiasts, Gemini capsule operators, retro
computing hobbyists. Intimacy over scale.

---

## Runtime Architecture

```
Internet (Telnet :2323)
       │
       ▼
 Debian 12 Linux host
 ├─ tcpser        — TCP-to-serial bridge (virtual COM port)
 ├─ cron          — Feed fetcher every 6h → feeds.img
 └─ run_bbs.sh    — QEMU + tcpser lifecycle management
       │
       ▼ (virtual serial / FOSSIL)
 QEMU VM — MS-DOS 6.22
 ├─ BNU.COM       — FOSSIL driver (BNU v1.70)
 ├─ SHARE.EXE     — DOS file locking
 └─ BBS.EXE       — OrbitBBS (C:\ORBIT\)

 Second drive (D:) = feeds.img (FAT12, 1.44MB)
 ├─ FEEDS\HN.ANS, LOBSTERS.ANS, etc.   — ANSI feed files
 └─ FEEDS.GFL                          — BBS file index
```

---

## What Was Removed from WWIV

| Feature | Status | Notes |
|---------|--------|-------|
| File transfers | Removed | All xfer menus, upload/download, batch DL |
| AutoMessage | Removed | Login display and post option gone |
| Chat with Sysop | Removed | reqchat, chat_room, WWIVCHAT support gone |
| QWK offline reader | Removed | Blocked at menu level |
| WWIV reg number display | Removed | Logon no longer shows (Unregistered) |

---

## What Was Changed

| Item | Change |
|------|--------|
| Version string | `OrbitBBS v1.0` |
| Env vars (BBS.C) | `WWIV_DIR` → `ORBIT_DIR`, `WWIV_INSTANCE` → `ORBIT_INSTANCE` |
| WWIV_FP / WWIV_NET.* | Kept — internal door IPC and network processing |
| WWIV.INI section names | Kept — pre-compiled INIT.EXE writes these |
| Main menu | File-driven: drop `MAINMENU.ANS` in GFILES, no recompile needed |
| Last Callers header | ANSI-colored, replaces garbled WWIV color codes |
| DOS prompt | `OrbitBBS: ` (shown on local console / DOS shell) |

---

## What Was Added

**Feed Reader** — The General Files (`G`) section hosts pre-fetched
ANSI-formatted news feeds from a Linux-side cron script.

| Feed file | Source |
|-----------|--------|
| `HN.ANS` | Hacker News top stories |
| `LOBSTERS.ANS` | Lobste.rs |
| `TILDES.ANS` | Drew DeVault's blog |
| `GEMINI.ANS` | Hundred Rabbits (100r.co) |
| `512KB.ANS` | Low-tech Magazine |

Configured in `scripts/fetch_feeds.py`. Runs every 6 hours via cron.

**File-Driven Screens** — All display screens (welcome, logon, menu, etc.)
are plain files in `C:\ORBIT\GFILES\`. Create in Moebius/PabloDraw (CP437),
inject via guestfish. No recompile needed.

---

## How to Build

Requires DOSBox and xvfb (headless display for CLI):

```bash
sudo apt install xvfb   # one-time
xvfb-run dosbox -conf build-auto.conf -exit
grep -c "Error" wwivs424/BUILD.LOG   # should be 0
```

Output: `wwivs424/exe/BBS.EXE`

Deploy to VM (stop first):
```bash
./run_bbs.sh stop
guestfish -a base-dos.qcow2 <<'EOF'
run
mount /dev/sda1 /
copy-in wwivs424/exe/BBS.EXE /ORBIT/
EOF
./run_bbs.sh start
```

See `OPERATIONS.md` for the full operations reference.

---

## Directory Layout

```
orbitbbs/
├─ run_bbs.sh              VM lifecycle (start/stop/status/setup)
├─ build-auto.conf         Unattended DOSBox build
├─ base-dos.qcow2          MS-DOS 6.22 QEMU disk (C: drive, OrbitBBS installed)
├─ feeds.img               FAT12 feed image (D: drive)
├─ logs/feeds.log          Cron feed log
├─ feeds/                  Feed staging — generated, gitignored
├─ scripts/
│   ├─ fetch_feeds.py      Fetch RSS feeds, generate ANSI + GFL
│   ├─ deploy_feeds.sh     Copy staging → feeds.img via mtools
│   └─ create_feeds_img.sh One-time: create feeds.img
├─ wwivs424/               Source code (Borland C, DOS target)
│   ├─ exe/BBS.EXE         Current compiled binary
│   └─ BUILD.LOG           Last build output
├─ BCPP31/                 Borland C++ 3.1 compiler (runs in DOSBox)
├─ BNU170/                 BNU FOSSIL driver
├─ tcpser/                 tcpser source + binary
└─ doors/                  Doors/games staging
```

---

## Important Notes

- `WWIV.INI` and its section names `[WWIV]` / `[WWIV-1]` are kept as-is.
  The pre-compiled `INIT.EXE` binary writes those names — renaming requires
  replacing INIT.EXE entirely.
- `WWIV_FP` env var and `WWIV_NET.*` temp files are kept — they are internal
  IPC used by door programs and network processing. Renaming breaks doors.
- Screen `.ANS` files use CP437 encoding, not UTF-8. SyncTerm and other BBS
  terminals expect CP437.
- Single-instance deployment. Multi-node deferred — target audience doesn't
  warrant the complexity.
