# OrbitBBS — Operations Guide

Day-to-day administration reference. Everything you need to manage the BBS
without touching the C source code.

---

## 1. Start / Stop / Status

```bash
cd /home/nick/Projects/Code_Projects/orbitbbs

./run_bbs.sh start    # Start QEMU VM + tcpser
./run_bbs.sh stop     # Graceful stop
./run_bbs.sh status   # Show PIDs, PTY, port
```

**Test connection:**
```bash
telnet localhost 2323
```

**Logs:**
```bash
/tmp/orbitbbs-qemu.log      # QEMU startup output
/tmp/orbitbbs-tcpser.log    # tcpser connection log
logs/feeds.log              # Feed fetch/deploy log (cron)
```

---

## 2. Injecting Files into the DOS VM

The VM disk (`base-dos.qcow2`) is a standard FAT filesystem. Use `guestfish`
to read or write files without booting the VM interactively.

> **ALWAYS stop the VM before using guestfish. Writing to a running qcow2
> risks corruption.**

### Stop → inject → start pattern

```bash
./run_bbs.sh stop

guestfish -a base-dos.qcow2 <<'EOF'
run
mount /dev/sda1 /
copy-in /path/to/local/file /ORBIT/GFILES/
EOF

./run_bbs.sh start
```

### Common target paths inside the VM

| What | DOS path | guestfish path |
|------|----------|----------------|
| BBS executable | `C:\ORBIT\BBS.EXE` | `/ORBIT/BBS.EXE` |
| Screen/menu files | `C:\ORBIT\GFILES\` | `/ORBIT/GFILES/` |
| Feed ANSI files | `C:\ORBIT\GFILES\FEEDS\` | `/ORBIT/GFILES/FEEDS/` |
| BBS config | `C:\ORBIT\WWIV.INI` | `/ORBIT/WWIV.INI` |
| AUTOEXEC.BAT | `C:\AUTOEXEC.BAT` | `/AUTOEXEC.BAT` |
| CONFIG.SYS | `C:\CONFIG.SYS` | `/CONFIG.SYS` |

### Read a file from the VM

```bash
guestfish -a base-dos.qcow2 --ro <<'EOF'
run
mount /dev/sda1 /
cat /AUTOEXEC.BAT
EOF
```

### List a directory

```bash
guestfish -a base-dos.qcow2 --ro <<'EOF'
run
mount /dev/sda1 /
ls /ORBIT/GFILES/
EOF
```

---

## 3. Updating ANSI Screen Files

Screens are plain files in `C:\ORBIT\GFILES\`. Drop in a new version and it
takes effect immediately on next display — no restart, no recompile.

### Workflow

1. Create or edit the file on Linux using **Moebius** or **PabloDraw**
2. Save as **CP437** encoding (not UTF-8 — SyncTerm expects CP437)
3. Inject into the VM:

```bash
./run_bbs.sh stop

guestfish -a base-dos.qcow2 <<'EOF'
run
mount /dev/sda1 /
copy-in MAINMENU.ANS /ORBIT/GFILES/
EOF

./run_bbs.sh start
```

### Screen file reference

| Filename | Shown when | ANSI version | Plain fallback |
|----------|-----------|--------------|----------------|
| `MAINMENU.ANS` | Main menu (non-expert mode) + `?` | `.ANS` | `.MSG` |
| `WELCOME.ANS` | Before login prompt | `.ANS` | `.MSG` |
| `LOGON.ANS` | After successful login | `.ANS` | `.MSG` |
| `LOGOFF.ANS` | On logout | `.ANS` | `.MSG` |
| `NEWUSER.ANS` | New user registration | `.ANS` | `.MSG` |
| `FEEDBACK.ANS` | Feedback/email screen | `.ANS` | `.MSG` |

**How auto-selection works:** The BBS checks for `.ANS` first (ANSI+color
users), then `.B&W` (ANSI without color), then `.MSG` (plain terminal).
You only need to provide the versions you want — missing variants fall back
gracefully.

### CP437 encoding check (Linux)

```bash
python3 -c "
with open('MYFILE.ANS', 'r', encoding='utf-8') as f: text = f.read()
try:
    text.encode('cp437')
    print('OK')
except UnicodeEncodeError as e:
    print(f'Bad char at {e.start}: U+{ord(text[e.start]):04X}')
"
```

If the check fails, fix the offending character in your ANSI editor
(use CP437 block chars, not Unicode equivalents).

---

## 4. DOS-Level Changes (AUTOEXEC.BAT, CONFIG.SYS)

Edit on Linux, inject with guestfish. Files must have **CRLF** line endings.

### Edit AUTOEXEC.BAT

```bash
# Pull current version out
guestfish -a base-dos.qcow2 --ro <<'EOF'
run
mount /dev/sda1 /
download /AUTOEXEC.BAT /tmp/AUTOEXEC.BAT
EOF

# Edit it
nano /tmp/AUTOEXEC.BAT

# Ensure CRLF endings
sed -i 's/\r\{0,1\}$/\r/' /tmp/AUTOEXEC.BAT

# Inject back
./run_bbs.sh stop
guestfish -a base-dos.qcow2 <<'EOF'
run
mount /dev/sda1 /
copy-in /tmp/AUTOEXEC.BAT /
EOF
./run_bbs.sh start
```

### Current AUTOEXEC.BAT

```bat
@ECHO OFF
SET ORBIT_INSTANCE=1
SET ORBIT_DIR=C:\ORBIT
C:\DOS\SHARE.EXE /F:4096 /L:40
C:\BNU\BNU.COM
IF EXIST D:\FEEDS\HN.ANS COPY D:\FEEDS\*.ANS C:\ORBIT\GFILES\FEEDS\ /Y
IF EXIST D:\FEEDS.GFL    COPY D:\FEEDS.GFL C:\ORBIT\DATA\ /Y
CD C:\ORBIT
:TOP
BBS.EXE /N1
IF ERRORLEVEL 1 GOTO TOP
```

---

## 5. Rebuilding BBS.EXE (recompile)

Only needed if C source code changes. Screen files never require a recompile.

```bash
cd /home/nick/Projects/Code_Projects/orbitbbs

# Build (headless — xvfb-run required, install with: sudo apt install xvfb)
xvfb-run dosbox -conf build-auto.conf -exit

# Check for errors
grep -c "Error" wwivs424/BUILD.LOG   # should be 0
tail -5 wwivs424/BUILD.LOG

# Deploy
./run_bbs.sh stop
guestfish -a base-dos.qcow2 <<'EOF'
run
mount /dev/sda1 /
copy-in wwivs424/exe/BBS.EXE /ORBIT/
EOF
./run_bbs.sh start
```

---

## 6. Feed Management

### Manual refresh

```bash
# Fetch new content and deploy to feeds.img
python3 scripts/fetch_feeds.py && bash scripts/deploy_feeds.sh
```

### Cron schedule (active)

```
0 */6 * * *   fetch_feeds.py   — fetch every 6 hours
5 */6 * * *   deploy_feeds.sh  — deploy 5 min after fetch
```

Feeds copy from `feeds.img` (D:) to `C:\ORBIT\GFILES\FEEDS\` on each DOS boot
via the `IF EXIST` lines in AUTOEXEC.BAT.

### Adding or changing feeds

Edit `scripts/fetch_feeds.py` — the `FEEDS` list at the top. Each entry:

```python
{
    "filename": "MYFEED.ANS",       # output file in feeds/
    "description": "My Feed",       # shown in BBS menu
    "url": "https://example.com/rss",
    "type": "rss",                  # rss, atom, or gemini
    "title": "MY FEED",             # ANSI header title
    "subtitle": "description line",
    "max_items": 20,
}
```

After editing, run `python3 scripts/fetch_feeds.py` to test, then
`bash scripts/deploy_feeds.sh` to push to the image.

---

## 7. Sysop Commands (via Telnet)

Log in as sysop (SL=255), then type `//` followed by the command at the
main menu prompt.

| Command | Function |
|---------|----------|
| `//UE` or `//UEDIT` | User editor — set SL, validate accounts, edit flags |
| `//CU` or `//CHUSER` | Switch active user context |
| `//BE` | Board (sub) editor |
| `//CE` | Chain (door/game) editor |
| `//YLOG` | Yesterday's activity log |
| `//DOS` | Drop to DOS shell (requires sysop password) |

**Single-char commands** (type directly at menu prompt, no `//`):

| Key | Function |
|-----|----------|
| `U` | User list |
| `L` | Last callers |
| `I` | System info + version |
| `VER` | Version string (typed in full) |
| `WHO` | Multi-instance status |

---

## 8. WWIV.INI — BBS Configuration

Located at `C:\ORBIT\WWIV.INI`. Pull, edit, reinject with guestfish.

> Section names `[WWIV]` and `[WWIV-1]` are intentional — the pre-compiled
> `INIT.EXE` writes those names. Do not rename them.

Key settings you may want to adjust:

```ini
[WWIV]
CALLOUT_BETWEEN_NODE_NUMBER=   ; node number (set when joining FidoNet/WWIVnet)
MAX_USERS=                     ; maximum user accounts
NEWUSER_SL=                    ; security level for new users
VALIDATED_SL=                  ; security level after sysop validation
```

To edit interactively, use setup mode (requires desktop/VNC):
```bash
./run_bbs.sh setup   # boots VM with display — runs INIT.EXE
```

---

## 9. Directory Quick Reference

```
orbitbbs/
├─ run_bbs.sh              Start/stop/status/setup
├─ build-auto.conf         Unattended DOSBox build config
├─ base-dos.qcow2          MS-DOS 6.22 QEMU disk (C: drive)
├─ feeds.img               FAT12 feed image (D: drive)
├─ WELCOME.MSG             Pre-login screen (legacy, replace with WELCOME.ANS)
├─ logs/
│   └─ feeds.log           Cron feed log
├─ feeds/                  Feed staging (generated, gitignored)
│   ├─ HN.ANS
│   ├─ LOBSTERS.ANS
│   ├─ TILDES.ANS          (Drew DeVault's blog)
│   ├─ GEMINI.ANS          (Hundred Rabbits)
│   ├─ 512KB.ANS           (Low-tech Magazine)
│   └─ FEEDS.GFL           Binary index for BBS
├─ scripts/
│   ├─ fetch_feeds.py      Fetch + generate ANSI files
│   ├─ deploy_feeds.sh     Copy staging → feeds.img via mtools
│   └─ create_feeds_img.sh One-time: create feeds.img
└─ wwivs424/
    ├─ exe/BBS.EXE         Current compiled binary
    ├─ BUILD.LOG           Last build output
    └─ *.C / *.H           Source code
```
