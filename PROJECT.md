# OrbitBBS

A fork of WWIV BBS v4.24a targeting MS-DOS 6.22, designed for the small web and
Geminispace community. Single-instance, retro feel, no file transfers.

## What It Is

OrbitBBS is not WWIV. Like Telegard and Renegade before it, it uses WWIV 4.24a
as a codebase and diverges from there. All WWIV branding has been replaced.
The focus is message bases, doors/games, and a curated feed reader — not file
trading or large-scale multi-node operation.

**Target audience:** Small web enthusiasts, Gemini capsule operators, retro
computing hobbyists. Intimacy over scale.

## Runtime Architecture

```
Internet (Telnet)
       │
       ▼
 Debian 12 Linux host
 ├─ tcpser        — TCP-to-serial bridge (presents virtual COM ports)
 └─ cron jobs     — Feed fetcher, maintenance scripts
       │
       ▼ (virtual serial / FOSSIL)
 QEMU VM — MS-DOS 6.22
 ├─ BNU.COM       — FOSSIL driver (BNU v1.70)
 ├─ SHARE.EXE     — DOS file locking (loaded in CONFIG.SYS)
 └─ BBS.EXE       — OrbitBBS (single instance)
```

## What Was Removed from WWIV

| Feature           | Status    | Notes |
|-------------------|-----------|-------|
| File transfers    | Removed   | All xfer menus, upload/download, batch DL |
| AutoMessage       | Removed   | Login display and post option gone |
| Chat with Sysop   | Removed   | reqchat, chat_room, WWIVCHAT support gone |
| DIREDIT sysop cmd | Removed   | File directory editor gone with xfer |

## What Was Changed

| Item              | Change |
|-------------------|--------|
| Version string    | `OrbitBBS v1.0` |
| Env vars          | `WWIV_DIR` → `ORBIT_DIR`, `WWIV_INSTANCE` → `ORBIT_INSTANCE` |
| Function pointer  | `WWIV_FP` → `ORBIT_FP` (external program handshake) |
| Net DAT file      | `WWIV_NET.DAT` → `ORBIT_NET.DAT` |
| Contact screen    | Wayne Bell / WWIV Software Services → OrbitBBS attribution |
| Batch file hint   | `wwiv.bat` → `orbit.bat` |
| General Files     | Repurposed as feed reader section (see below) |

## What Was Added

**Feed Reader** — The General Files (G) section hosts pre-fetched ANSI-formatted
news feeds. A Linux-side cron script fetches content and writes it to a shared
DOS-accessible disk image.

Feed sources (configured in `scripts/fetch_feeds.py`):
- Hacker News top stories
- Lobste.rs
- Tildes.net `~tech` group (URL needs verification — see NEXT-STEPS.md)
- Geminispace aggregator via portal.mozz.us proxy (URL needs verification)
- 512KB Club (URL needs verification)

## How to Build

Requires Borland C++ 3.1 (already in `BCPP31/`) and DOSBox.

```bash
dosbox -conf build.conf
# Inside DOSBox:
make -f makefile.mak > build.log
exit
```

Output binaries: `wwivs424/exe/BBS.EXE`, `RETURN.EXE`, `MINIESM.EXE`, `FIX.EXE`

## Directory Layout

```
orbitbbs/
├─ wwivs424/        Source code (Borland C, DOS target)
│   ├─ exe/         Compiled output
│   └─ obj/         Object files
├─ BCPP31/          Borland C++ 3.1 compiler (DOS executables, run in DOSBox)
├─ BNU170/          BNU FOSSIL driver
├─ tcpser/          tcpser source/binary
├─ feeds/           Feed staging directory (ANSI files + GFL index)
├─ scripts/         Linux-side maintenance scripts
│   ├─ fetch_feeds.py       Fetches RSS/Gemini, generates ANSI files + GFL
│   ├─ deploy_feeds.sh      Copies staging files to DOS FAT image via mtools
│   ├─ create_feeds_img.sh  One-time: creates feeds.img FAT12 disk image
│   └─ mtoolsrc             mtools config
├─ base-dos.qcow2   MS-DOS 6.22 QEMU disk image (BBS not yet installed)
├─ feeds.img        (created by create_feeds_img.sh — does not exist yet)
├─ build.conf       DOSBox config for compilation
└─ doors/           Doors/games staging
```

## Important Notes

- `WWIV.INI` filename and INI section names (`[WWIV]`, `[WWIV-1]`) are kept
  as-is because `INIT.EXE` is a pre-compiled binary that writes those names.
  Renaming them requires replacing INIT.EXE with a custom version.
- The `wwiv_version`, `wwiv_date`, `wwiv_num_version` variable names in `.H`
  files are internal only and were not renamed (not user-visible).
- Single-instance deployment. Multi-node support was intentionally deferred —
  the target audience does not warrant the added complexity.
