# OrbitBBS — Session Handoff
**Date:** 2026-03-30
**Session ended:** 2026-03-30

---

## 🟢 What Was Accomplished This Session

### Phase 10 — Feature Validation ✅

All menu commands validated end-to-end. Two bugs found and fixed:

| # | Issue | Fix | File |
|---|-------|-----|------|
| 1 | Messages required validation even for sysop | SL >= 20 bypasses `restrict_validate` | MSGBASE1.C, QWK1.C |
| 2 | `checkpw()` silent on wrong password | Added "Incorrect password." feedback | BBSUTL1.C |

**Full validation checklist — all passing:**
- [x] Messages: P, S, N, Q, Z, R, E, M, K, F
- [x] Sub navigation: `]`, `[`, H, J
- [x] Settings: Y, I, X
- [x] Last callers: L
- [x] Logoff: O, `/O`
- [x] Sysop commands: `//UE`, `//BE`, `//CE`, `//GE`
- [x] New user registration end-to-end
- [x] VER command (no WWIV artifacts)
- [x] G → Feeds → pagination
- [x] U → user list ANSI alignment
- [x] D → Defaults (no WWIV Reg #)
- [x] B → BBS List (telnet address)

### Phase 11 — Migration to debian-mac2 ✅

OrbitBBS moved from debian12 to debian-mac2 (192.168.0.232). Now publicly accessible.

**Infrastructure:**
- OrbitBBS runs on **debian-mac2** (192.168.0.232 / Tailscale 100.118.30.47)
- **Relay** (157.230.152.152 / gamesrv-relay): nginx stream proxy, port 2324 → 100.118.30.47:2323
- **DNS:** `orbitbbs.njb1966.com` A record → 157.230.152.152 (Cloudflare, DNS-only / grey cloud)
- **Public address:** `orbitbbs.njb1966.com:2324`
- **Systemd:** `orbitbbs.service` on debian-mac2, enabled at boot

**Relay config** (`/etc/nginx/conf.d/orbitbbs.stream` on gamesrv-relay):
```nginx
server {
    listen 2324;
    proxy_pass 100.118.30.47:2323;
}
```

### Build System Discovery ✅

DOSBox (used for building BBS.EXE) requires a display. Build machine is debian12.

- **Automated build:** `DISPLAY=:1 dosbox -conf build-auto.conf -exit`
- **Interactive build:** `DISPLAY=:1 dosbox -conf build.conf` then `make -f makefile.mak`
- `DISPLAY=:1` uses TigerVNC server already running on debian12 (port 5901)
- `build-auto.conf` created this session — runs make and exits automatically

### VM Console Access (INIT.EXE etc.) ✅

To access the DOS console for INIT.EXE or manual config:
1. SSH tunnel: `ssh -L 5901:localhost:5901 nick@debian12`
2. `DISPLAY=:1 ./run_bbs.sh setup` (on debian12 — build machine, not production)
3. Remmina → `127.0.0.1:5901` with VNC password
4. Press **F5** at DOS boot to skip AUTOEXEC.BAT

---

## 📍 Phase Roadmap

| Phase | Status | Description |
|-------|--------|-------------|
| 0–11 | ✅ Complete | Build, branding, feature removals, feeds, QEMU, ANSI screens, validation, migration |
| **12** | 🔲 **Next** | FidoNet (binkd + HPT, apply for node number) |
| 13 | 🔲 Future | Doors/games (LORD, TradeWars, Usurper) |

---

## 🔧 Key Technical Details

### Current Binary
- `wwivs424/exe/BBS.EXE` — **Mar 30 13:27, 557,824 bytes**
- Deployed to debian-mac2 via guestfish

### BBS.EXE Build History (cumulative)
| Size | Changes |
|------|---------|
| 557,744 | DEFAULTS.C + MISCCMD.C (reg#, user list, bbslist) |
| 557,792 | GFILES.C (forced pause), MSGBASE.C (ESC reset fix) |
| 557,776 | BBSUTL1.C (checkpw feedback) |
| **557,824** | MSGBASE1.C + QWK1.C (SL>=20 bypasses restrict_validate) |

### Feed Deployment
```bash
python3 scripts/fetch_feeds.py && bash scripts/deploy_feeds.sh
```
Cron on debian-mac2: fetch `0 */6`, deploy `5 */6`. `deploy_feeds.sh` handles stop/inject/start via guestfish.

### Gfiles Section Config (set via `//GE`)
Section name: "Feeds", filename: "FEEDS", SL: 0
→ reads `DATA\FEEDS.GFL`, serves files from `GFILES\FEEDS\`

### AUTOEXEC.BAT (current)
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

### Infrastructure Map
```
User (telnet) → orbitbbs.njb1966.com:2324
    → 157.230.152.152:2324 (gamesrv-relay, nginx stream)
    → 100.118.30.47:2323 (debian-mac2, Tailscale)
    → tcpser → QEMU serial → MS-DOS 6.22 → BBS.EXE
```

---

## 🚀 What to Do Next

**Phase 12 — FidoNet**
- Install binkd + HPT on debian-mac2
- Apply for FidoNet node number (public IP available via relay: 157.230.152.152)
- BinkP port: 24554 — add nginx stream proxy on relay (same pattern as port 2324)

---

## 💬 Resuming Next Session

Say: **"Continuing OrbitBBS. Phases 10 and 11 complete. See SESSION-HANDOFF.md."**
