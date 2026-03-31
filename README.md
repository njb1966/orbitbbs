# OrbitBBS

A fork of [WWIV BBS v4.24a](https://github.com/wwivbbs/wwiv) targeting MS-DOS 6.22,
built for the small web and Geminispace community. Single-instance, retro feel, no
file transfers.

**Live:** `telnet orbitbbs.njb1966.com 2324`

---

## What It Is

OrbitBBS is not WWIV. Like Telegard and Renegade before it, it takes WWIV 4.24a as a
codebase and diverges from there. All WWIV branding has been replaced. The focus is
message bases, doors/games, and a curated feed reader — not file trading or large-scale
multi-node operation.

**Target audience:** Small web enthusiasts, Gemini capsule operators, retro computing
hobbyists. Intimacy over scale.

---

## Architecture

```
Internet (Telnet :2324)
       │
       ▼
 Nginx stream proxy (relay VPS)
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
 └─ ANSI feed files, refreshed by cron
```

---

## What Was Changed from WWIV

| Item | Change |
|------|--------|
| Branding | All WWIV references replaced with OrbitBBS |
| Version string | `OrbitBBS v1.0` |
| Env vars | `WWIV_DIR` → `ORBIT_DIR`, `WWIV_INSTANCE` → `ORBIT_INSTANCE` |
| Main menu | File-driven: drop `MAINMENU.ANS` in GFILES, no recompile needed |
| Last Callers header | ANSI-colored, replaces garbled WWIV color codes |
| DOS prompt | `OrbitBBS: ` |

**Removed:** File transfers, AutoMessage, Chat with Sysop, QWK offline reader, WWIV
registration number display.

**Note:** `WWIV.INI`, `[WWIV]` section names, `WWIV_FP`, and `WWIV_NET.*` temp files
are kept as-is. The pre-compiled `INIT.EXE` writes those names and doors rely on the
env vars — renaming breaks things.

---

## Feed Reader

The General Files section hosts pre-fetched ANSI-formatted news feeds, updated every
6 hours by a Linux-side cron script:

| Feed | Source |
|------|--------|
| `HN.ANS` | Hacker News top stories |
| `LOBSTERS.ANS` | Lobste.rs |
| `TILDES.ANS` | Drew DeVault's blog |
| `GEMINI.ANS` | Hundred Rabbits (100r.co) |
| `512KB.ANS` | Low-tech Magazine |

Configured in `scripts/fetch_feeds.py`. Add or change feeds by editing the `FEEDS`
list at the top of that file.

---

## Quick Start

### Requirements

- Debian/Ubuntu Linux host
- `qemu-system-i386`, `tcpser`, `dosbox`, `guestfish`, `xvfb`, `mtools`

### Run the BBS

```bash
./run_bbs.sh start    # Start QEMU VM + tcpser
./run_bbs.sh stop     # Graceful stop
./run_bbs.sh status   # Show PIDs, PTY, port
telnet localhost 2323  # Test connection
```

### Build BBS.EXE (only needed after source changes)

Requires DOSBox and a display. `xvfb-run` provides a headless display:

```bash
sudo apt install xvfb   # one-time
xvfb-run dosbox -conf build-auto.conf -exit
grep -c "Error" wwivs424/BUILD.LOG   # should be 0
```

### Deploy a new BBS.EXE

```bash
./run_bbs.sh stop
guestfish -a base-dos.qcow2 <<'EOF'
run
mount /dev/sda1 /
copy-in wwivs424/exe/BBS.EXE /ORBIT/
EOF
./run_bbs.sh start
```

---

## Directory Layout

```
orbitbbs/
├─ run_bbs.sh              VM lifecycle (start/stop/status/setup)
├─ build-auto.conf         Unattended DOSBox build config
├─ base-dos.qcow2          MS-DOS 6.22 QEMU disk (C: drive, gitignored)
├─ feeds.img               FAT12 feed image (D: drive, gitignored)
├─ logs/feeds.log          Cron feed log
├─ scripts/
│   ├─ fetch_feeds.py      Fetch RSS feeds, generate ANSI + GFL
│   ├─ deploy_feeds.sh     Copy staging → feeds.img via guestfish
│   └─ create_feeds_img.sh One-time: create feeds.img
├─ wwivs424/               Source code (Borland C++ 3.1, DOS target)
│   ├─ exe/BBS.EXE         Compiled binary
│   └─ BUILD.LOG           Last build output
├─ BCPP31/                 Borland C++ 3.1 compiler (runs in DOSBox)
├─ BNU170/                 BNU FOSSIL driver
├─ tcpser/                 tcpser source + binary
└─ doors/                  Doors/games staging
```

---

## Documentation

| File | Contents |
|------|----------|
| `PROJECT.md` | Full feature list, what was changed/removed, build reference |
| `OPERATIONS.md` | Day-to-day administration, file injection, feed management, sysop commands |
| `NEXT-STEPS.md` | Upcoming work (FidoNet, doors/games) |

---

## Connecting

Use any telnet-capable BBS terminal. [SyncTerm](https://syncterm.bbsdev.net/) is
recommended — it handles CP437 encoding and ANSI/RIP correctly.

```
Host:  orbitbbs.njb1966.com
Port:  2324
```

---

## License

OrbitBBS source modifications are released under the same license as WWIV 4.24a.
WWIV 4.24a source is in the public domain.
