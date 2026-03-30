# OrbitBBS — Next Steps

Phases 0–5 of initial development are complete. This document tracks what
remains before the BBS is live.

---

## Phase 6 — QEMU VM Setup (BLOCKED: do this next)

The `base-dos.qcow2` has MS-DOS 6.22 but OrbitBBS is not installed yet.

### 6a. QEMU Launch Configuration

Create a launch script (`run_bbs.sh`) with:

```bash
qemu-system-i386 \
  -hda /home/nick/Projects/Code_Projects/orbitbbs/base-dos.qcow2 \
  -hdb /home/nick/Projects/Code_Projects/orbitbbs/feeds.img \
  -m 16 \
  -serial pty \
  -display none \
  -daemonize
```

- `-hdb feeds.img` — second drive (D:) for feed files
- `-serial pty` — creates a virtual serial port for tcpser to attach to
- `-m 16` — 16MB RAM (DOS can't use more anyway without EMS/XMS)
- Adjust serial port as needed for tcpser integration

### 6b. DOS CONFIG.SYS and AUTOEXEC.BAT

**CONFIG.SYS:**
```
FILES=40
BUFFERS=20
DEVICE=C:\DOS\HIMEM.SYS
DOS=HIGH
SHELL=C:\COMMAND.COM /P /E:512
```

**AUTOEXEC.BAT:**
```batch
@ECHO OFF
SET ORBIT_INSTANCE=1
SET ORBIT_DIR=C:\ORBIT\
C:\DOS\SHARE.EXE /F:4096 /L:40
C:\BNU\BNU.COM
IF EXIST D:\FEEDS\HN.ANS COPY D:\FEEDS\*.ANS C:\ORBIT\GFILES\FEEDS\ /Y
IF EXIST D:\FEEDS.GFL    COPY D:\FEEDS.GFL C:\ORBIT\DATA\ /Y
CD C:\ORBIT
:TOP
BBS.EXE /N1
IF ERRORLEVEL 1 GOTO TOP
```

### 6c. OrbitBBS Installation on DOS

1. Copy `exe\BBS.EXE`, `RETURN.EXE`, `MINIESM.EXE`, `FIX.EXE` to `C:\ORBIT\`
2. Copy all support files (`.MSG`, `.STR`, `.MDM`, `WWIV.INI`, `REGIONS.DAT`) to `C:\ORBIT\`
3. Copy `BNU170\BNU.COM` to `C:\BNU\`
4. Run `INIT.EXE` to configure OrbitBBS:
   - Set data directory: `C:\ORBIT\DATA\`
   - Set gfiles directory: `C:\ORBIT\GFILES\`
   - Create a "Feeds" section in General Files pointing at `C:\ORBIT\GFILES\FEEDS\`
   - Register the 5 feed files in GFLEDIT (HN.ANS, LOBSTERS.ANS, etc.)
   - Configure modem: FOSSIL driver (BNU), not a real modem init string
5. Create directory structure:
   ```
   C:\ORBIT\DATA\
   C:\ORBIT\GFILES\FEEDS\
   C:\ORBIT\MSGS\
   ```

### 6d. tcpser Configuration

tcpser bridges TCP connections to the DOS virtual serial port.

```bash
# Example — adjust device path from QEMU -serial pty output
tcpser -v 25 -s 38400 -d /dev/pts/N -l 7
```

- Port 25 is telnet; consider 2323 if you don't want to run as root
- Test with: `telnet localhost 25` from the Linux host

---

## Phase 7 — Feed URL Verification

Three of five feed URLs need manual verification:

### Tildes.net
- Current URL: `https://tildes.net/~tech.rss` (returning 404)
- Try: `https://tildes.net/~tech/.rss` (with trailing slash before .rss)
- Try: Logging into Tildes and checking the RSS icon URL in the sidebar
- Alternative: Replace with another feed (e.g., `https://lobste.rs/t/programming.rss`)

### Geminispace via portal.mozz.us
- Current URL: `https://portal.mozz.us/gemini/geminispace.info/new` (returning 500)
- Try: `https://portal.mozz.us/gemini/geminispace.info/` to see available paths
- Alternative proxy: `https://proxy.vulpes.one/gemini/geminispace.info/`
- Alternative: Host your own Gemini proxy (`kineto`, `gmni`)

### 512KB Club
- GitHub has no releases (0 items returned, correct behavior — no releases exist)
- Options:
  a. Check `https://512kb.club/` page source for an RSS link
  b. Replace with Low-tech Magazine: `https://solar.lowtechmagazine.com/feeds/all-en.atom.xml`
  c. Replace with The Old Net: check `theoldnet.com` for RSS

Once URLs are fixed, update `scripts/fetch_feeds.py` and re-run.

---

## Phase 8 — Cron Setup

After feeds are verified and VM is live:

```bash
# Edit crontab: crontab -e
# Fetch every 6 hours; deploy 5 minutes later
0 */6 * * * /home/nick/Projects/Code_Projects/orbitbbs/scripts/fetch_feeds.py >> /var/log/orbitbbs-feeds.log 2>&1
5 */6 * * * /home/nick/Projects/Code_Projects/orbitbbs/scripts/deploy_feeds.sh >> /var/log/orbitbbs-feeds.log 2>&1
```

Also create the FAT image first (one time):
```bash
sudo apt install mtools dosfstools
/home/nick/Projects/Code_Projects/orbitbbs/scripts/create_feeds_img.sh
```

---

## Phase 9 — Doors / Games

The `doors/` directory is a staging area. WWIV supports external programs
via chains (CHAINS.DAT). Classic doors to consider:

- **Legend of the Red Dragon (LORD)** — Seth Robinson's classic RPG door
- **TradeWars 2002** — space trading, works well with small player counts
- **Usurper** — dungeon RPG, very popular on small BBSs

Each door needs:
1. A chain entry configured in INIT.EXE
2. A FOSSIL-aware version or a door driver that bridges FOSSIL to standard serial
3. Testing that the door can access the chain file (DOOR.SYS or CHAIN.TXT)

WWIV generates `CHAIN.TXT` for doors. BNU FOSSIL handles the COM port bridging.

---

## Deferred / Future

| Item | Notes |
|------|-------|
| Replace INIT.EXE | Pre-compiled binary still uses WWIV.INI section names. A custom INIT would rename to ORBIT.INI and clean up remaining WWIV strings in the sysop configuration tool. Low priority. |
| Second instance | If concurrent callers ever become a real need, WWIV's multi-instance works by running a second BBS.EXE with ORBIT_INSTANCE=2 on a second virtual serial port. SHARE.EXE handles file locking. The code already supports this — it's a runtime config, not a code change. |
| Custom logon art | HELLO.RIP / WELCOME.RIP need custom ANSI art for OrbitBBS branding. |
| Message conferences | Kept from WWIV. Consider sub-board layout: General, Tech, Small Web, Gemini, Retro Computing, Sysop. |
| QWK support | QWK packet offline reader code is still in the binary. Consider removing in a future cleanup pass if no one uses it. |

---

## Current State Summary

| Phase | Status | Description |
|-------|--------|-------------|
| 0 | ✅ Complete | DOSBox build environment, clean baseline compile |
| 1 | ✅ Complete | Branding: WWIV → OrbitBBS, env vars, version strings |
| 2 | ✅ Complete | AutoMessage removed |
| 3 | ✅ Complete | Chat with Sysop removed |
| 4 | ✅ Complete | File transfers removed (user-facing; source files retained as dead code) |
| 5 | ✅ Complete | Feed reader: Linux scripts, ANSI formatter, GFL generator. 2/5 feeds live. |
| 6 | 🔲 Next | QEMU VM setup, OrbitBBS installation, tcpser integration |
| 7 | 🔲 Blocked on 6 | Feed URL verification and cron setup |
| 8 | 🔲 Blocked on 6 | Cron job activation |
| 9 | 🔲 Future | Doors and games |
