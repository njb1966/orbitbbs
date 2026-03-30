#!/usr/bin/env python3
"""
OrbitBBS Feed Fetcher
Fetches RSS/Atom/Gemini feeds and generates ANSI-colored text files
for display in the BBS General Files / Feeds section.

Cron example (every 6 hours):
  0 */6 * * * /home/nick/Projects/Code_Projects/orbitbbs/scripts/fetch_feeds.py

Output goes to STAGING_DIR. Copy to DOS via mtools (see deploy_feeds.sh).
"""

import urllib.request
import urllib.error
import xml.etree.ElementTree as ET
import struct
import time
import os
import sys
from datetime import datetime, timezone

# Unicode → ASCII transliteration table for DOS latin-1 output
_UNICODE_MAP = str.maketrans({
    "\u2013": "-",   "\u2014": "--",  # en-dash, em-dash
    "\u2018": "'",   "\u2019": "'",   # left/right single quote
    "\u201c": '"',   "\u201d": '"',   # left/right double quote
    "\u2026": "...", "\u2022": "*",   # ellipsis, bullet
    "\u00e9": "e",   "\u00e8": "e",   "\u00ea": "e",  # e-accents
    "\u00e0": "a",   "\u00e1": "a",   "\u00e2": "a",  # a-accents
    "\u00f3": "o",   "\u00f6": "o",   "\u00fc": "u",  # o/u-accents
    "\u00df": "ss",  "\u00b0": "deg", "\u00b7": ".",  # misc
})

def to_dos(text):
    """Transliterate common Unicode chars to ASCII, drop the rest."""
    return text.translate(_UNICODE_MAP).encode("latin-1", errors="replace").decode("latin-1")

# ─── Configuration ────────────────────────────────────────────────────────────

STAGING_DIR = os.path.join(
    os.path.dirname(os.path.dirname(os.path.abspath(__file__))),
    "feeds"
)

# Feed definitions. Adjust URLs and max_items as needed.
FEEDS = [
    {
        "filename": "HN.ANS",
        "description": "Hacker News - Top Stories",
        "url": "https://news.ycombinator.com/rss",
        "type": "rss",
        "title": "HACKER NEWS",
        "subtitle": "top stories",
        "max_items": 20,
    },
    {
        "filename": "LOBSTERS.ANS",
        "description": "Lobste.rs - Tech Stories",
        "url": "https://lobste.rs/rss",
        "type": "rss",
        "title": "LOBSTE.RS",
        "subtitle": "technology & programming",
        "max_items": 20,
    },
    {
        "filename": "TILDES.ANS",
        "description": "Tildes.net - Community",
        # Tildes groups have per-group RSS: https://tildes.net/~GROUPNAME.rss
        # The front page aggregate requires login. Use a public group instead.
        "url": "https://tildes.net/~tech.rss",
        "type": "rss",
        "title": "TILDES.NET",
        "subtitle": "community discussions",
        "max_items": 20,
    },
    {
        "filename": "GEMINI.ANS",
        "description": "Geminispace - Feed Aggregator",
        # Gemini content via HTTP proxy.
        # geminispace.info/search is their main search/aggregator page.
        # The /new path lists recently updated capsules.
        "url": "https://portal.mozz.us/gemini/geminispace.info/new",
        "type": "gemini",
        "title": "GEMINISPACE",
        "subtitle": "via portal.mozz.us  |  native: gemini://geminispace.info/feeds",
        "max_items": 20,
    },
    {
        "filename": "512KB.ANS",
        "description": "512KB Club - Small Web",
        # 512KB Club does not publish a traditional RSS feed.
        # Using their GitHub releases Atom feed as a proxy for site news.
        "url": "https://github.com/kevquirk/512kb.club/releases.atom",
        "type": "atom",
        "type": "rss",
        "title": "512KB CLUB",
        "subtitle": "small web news & hall of fame",
        "max_items": 15,
    },
]

SCREEN_WIDTH = 79
FETCH_TIMEOUT = 20   # seconds per feed

# ─── ANSI Colour Codes ────────────────────────────────────────────────────────

E        = "\x1b["
RESET    = f"{E}0m"
BORDER   = f"{E}1;36m"    # bright cyan  — section dividers
TITLE    = f"{E}1;33m"    # bright yellow — feed name
SUBTITLE = f"{E}0;36m"    # dark cyan     — subtitle / gemini note
HEADLINE = f"{E}1;37m"    # bright white  — story title text
NUM      = f"{E}1;37m"    # bright white  — item number
META     = f"{E}0;34m"    # dark blue     — score / comments (subdued)
URL_CLR  = f"{E}0;32m"    # dark green    — URLs (subdued)
FOOTER   = f"{E}0;36m"    # dark cyan     — timestamp line

DIVIDER  = BORDER + "\xcd" * SCREEN_WIDTH + RESET   # DOS box-drawing ═


# ─── Helpers ──────────────────────────────────────────────────────────────────

def cr(s=""):
    """Append CRLF — DOS line ending required for BBS display."""
    return s + "\r\n"


def wrap(text, first_width, cont_width, cont_indent):
    """Word-wrap text. Returns list of plain strings (no ANSI in input)."""
    words = text.split()
    lines = []
    current = ""
    width = first_width
    for word in words:
        if not current:
            current = word
        elif len(current) + 1 + len(word) <= width:
            current += " " + word
        else:
            lines.append(current)
            current = word
            width = cont_width
    if current:
        lines.append(current)
    if not lines:
        return [""]
    result = [lines[0]]
    for line in lines[1:]:
        result.append(cont_indent + line)
    return result


def fetch_url(url):
    """Fetch URL, return bytes or None on failure."""
    try:
        req = urllib.request.Request(
            url,
            headers={"User-Agent": "OrbitBBS-FeedFetcher/1.0 (+retro-bbs)"}
        )
        with urllib.request.urlopen(req, timeout=FETCH_TIMEOUT) as resp:
            return resp.read()
    except Exception as exc:
        print(f"    WARN: {exc}", file=sys.stderr)
        return None


# ─── Parsers ──────────────────────────────────────────────────────────────────

ATOM_NS = "http://www.w3.org/2005/Atom"
DC_NS   = "http://purl.org/dc/elements/1.1/"

def parse_rss(data):
    """
    Parse RSS 2.0 or Atom XML.
    Returns list of dicts: {title, link, meta}
    """
    items = []
    try:
        root = ET.fromstring(data)
    except ET.ParseError as exc:
        print(f"    WARN: XML parse error: {exc}", file=sys.stderr)
        return items

    tag = root.tag
    if tag == f"{{{ATOM_NS}}}feed" or tag == "feed":
        # Atom
        for entry in root.findall(f".//{{{ATOM_NS}}}entry"):
            t = entry.find(f"{{{ATOM_NS}}}title")
            l = entry.find(f"{{{ATOM_NS}}}link")
            a = entry.find(f"{{{ATOM_NS}}}author/{{{ATOM_NS}}}name")
            title = (t.text or "").strip() if t is not None else ""
            link  = l.get("href", "") if l is not None else ""
            author= (a.text or "").strip() if a is not None else ""
            meta  = f"by {author}" if author else ""
            if title:
                items.append({"title": title, "link": link, "meta": meta})
    else:
        # RSS 2.0
        for item in root.findall(".//item"):
            def txt(tag):
                el = item.find(tag)
                return (el.text or "").strip() if el is not None else ""

            title    = txt("title")
            link     = txt("link")
            creator  = txt(f"{{{DC_NS}}}creator")
            comments = txt("comments")

            parts = []
            if creator:
                parts.append(f"by {creator}")
            if comments:
                # Some feeds put a number in <comments>, others a URL
                if comments.isdigit():
                    parts.append(f"comments: {comments}")
            meta = " | ".join(parts)

            if title:
                items.append({"title": title, "link": link, "meta": meta})

    return items


def parse_gemini(data):
    """
    Parse text/gemini format returned by HTTP proxy.
    Returns list of dicts: {title, link, meta}
    """
    items = []
    try:
        text = data.decode("utf-8", errors="replace")
    except Exception:
        return items

    for line in text.splitlines():
        stripped = line.strip()
        if stripped.startswith("=>"):
            parts = stripped[2:].split(None, 1)
            if not parts:
                continue
            link  = parts[0]
            title = parts[1].strip() if len(parts) > 1 else link
            # Skip navigation links that are just gemini:// addresses as title
            if title and title != link:
                items.append({"title": title, "link": link, "meta": ""})

    return items


# ─── ANSI File Builder ────────────────────────────────────────────────────────

def build_ansi(feed_cfg, items):
    """
    Build ANSI-coloured feed display as a string with CRLF line endings.
    Format:
      ═══ header ═══
      1. Headline text
         meta info (subdued)
         url (subdued)
      ...
      ═══ footer + timestamp ═══
    """
    out = []
    now_str = datetime.now(timezone.utc).strftime("%Y-%m-%d %H:%M UTC")
    max_items = feed_cfg.get("max_items", 20)

    out.append(cr())
    out.append(cr(DIVIDER))
    out.append(cr(
        f" {TITLE}{feed_cfg['title']}{RESET}"
        f"  {SUBTITLE}{feed_cfg['subtitle']}{RESET}"
    ))
    out.append(cr(DIVIDER))
    out.append(cr())

    if not items:
        out.append(cr(f"  {SUBTITLE}No items retrieved.{RESET}"))
        out.append(cr())
    else:
        for idx, item in enumerate(items[:max_items], 1):
            title = to_dos(item["title"])
            link  = to_dos(item["link"])
            meta  = to_dos(item["meta"])

            # Number + headline (word-wrapped)
            prefix     = f" {NUM}{idx:2d}.{RESET} "
            first_w    = SCREEN_WIDTH - 5
            cont_w     = SCREEN_WIDTH - 5
            cont_indent = "      "
            lines = wrap(title, first_w, cont_w, cont_indent)

            out.append(cr(f"{prefix}{HEADLINE}{lines[0]}{RESET}"))
            for cont_line in lines[1:]:
                out.append(cr(f"{HEADLINE}{cont_line}{RESET}"))

            # Meta (score / author) — subdued
            if meta:
                out.append(cr(f"     {META}{meta}{RESET}"))

            # URL — subdued, truncated to fit screen
            if link:
                max_url = SCREEN_WIDTH - 6
                display = link if len(link) <= max_url else link[:max_url - 3] + "..."
                out.append(cr(f"     {URL_CLR}{display}{RESET}"))

            out.append(cr())

    out.append(cr(DIVIDER))
    out.append(cr(f" {FOOTER}Updated: {now_str}{RESET}"))
    out.append(cr(DIVIDER))
    out.append(cr())

    return "".join(out)


def build_error_ansi(feed_cfg, reason):
    """Placeholder file written when a feed fails to fetch."""
    out = []
    out.append(cr())
    out.append(cr(DIVIDER))
    out.append(cr(
        f" {TITLE}{feed_cfg['title']}{RESET}"
        f"  {SUBTITLE}{feed_cfg['subtitle']}{RESET}"
    ))
    out.append(cr(DIVIDER))
    out.append(cr())
    out.append(cr(f"  {META}Feed temporarily unavailable.{RESET}"))
    out.append(cr(f"  {SUBTITLE}{reason[:SCREEN_WIDTH - 4]}{RESET}"))
    out.append(cr())
    out.append(cr(DIVIDER))
    out.append(cr())
    return "".join(out)


# ─── GFL Binary Index ─────────────────────────────────────────────────────────

def write_gfl(feeds, staging_dir):
    """
    Write FEEDS.GFL binary file for WWIV/OrbitBBS.

    gfilerec struct (VARDEC.H):
      char description[81]   — display name shown in menu
      char filename[13]      — 8.3 filename within section directory
      long daten             — Unix timestamp (little-endian signed 32-bit)

    Total: 98 bytes per record.
    Place this file in the BBS DATA\ directory on DOS.
    """
    now = int(time.time())
    data = b""
    for feed in feeds:
        desc  = feed["description"].encode("latin-1", errors="replace")
        fname = feed["filename"].encode("latin-1", errors="replace")
        desc  = desc[:80].ljust(81, b"\x00")
        fname = fname[:12].ljust(13, b"\x00")
        daten = struct.pack("<l", now)
        data += desc + fname + daten

    path = os.path.join(staging_dir, "FEEDS.GFL")
    with open(path, "wb") as f:
        f.write(data)
    print(f"  Wrote {path}  ({len(feeds)} records, {len(data)} bytes)")


# ─── Main ─────────────────────────────────────────────────────────────────────

def main():
    os.makedirs(STAGING_DIR, exist_ok=True)

    print(f"OrbitBBS Feed Fetcher  {datetime.now().strftime('%Y-%m-%d %H:%M')}")
    print(f"Staging: {STAGING_DIR}")
    print()

    for feed in FEEDS:
        print(f"[{feed['title']}]  {feed['url']}")
        raw = fetch_url(feed["url"])

        if raw is None:
            content = build_error_ansi(feed, f"Fetch failed: {feed['url']}")
            print(f"  -> error placeholder")
        else:
            if feed["type"] == "gemini":
                items = parse_gemini(raw)
            else:
                items = parse_rss(raw)
            print(f"  -> {len(items)} items")
            content = build_ansi(feed, items)

        out_path = os.path.join(STAGING_DIR, feed["filename"])
        with open(out_path, "w", newline="", encoding="latin-1") as f:
            f.write(content)
        print(f"  -> {out_path}")
        print()

    print("[GFL Index]")
    write_gfl(FEEDS, STAGING_DIR)
    print()
    print("Done. Run deploy_feeds.sh to copy to DOS image.")


if __name__ == "__main__":
    main()
