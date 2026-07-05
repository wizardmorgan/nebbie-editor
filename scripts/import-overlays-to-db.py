#!/usr/bin/env python3
"""
Import Nebbie overlay directories (rooms/, objects/, mobiles/, zones/) into SQLite.

Each entity type is stored in a user-named table. The overlay file body is kept
verbatim so the mud can reconstruct the entity from the stored text.

Interactive use (default):
  ./scripts/import-overlays-to-db.sh

Non-interactive example:
  ./scripts/import-overlays-to-db.py \\
    --lib /path/to/mudroot/lib \\
    --db /path/to/nebbie-world.sqlite3 \\
    --rooms nebbie-room --objects nebbie-object \\
    --mobiles nebbie-mob --zone-resets nebbie-zone
"""

from __future__ import annotations

import argparse
import re
import sqlite3
import subprocess
import sys
from dataclasses import dataclass
from datetime import datetime, timezone
from pathlib import Path
from typing import Dict, Optional

TABLE_NAME_RE = re.compile(r"^[A-Za-z_][A-Za-z0-9_-]*$")

OVERLAY_DIRS = {
    "rooms": "rooms",
    "objects": "objects",
    "mobiles": "mobiles",
    "zone_resets": "zones",
}

DEFAULT_TABLES = {
    "rooms": "nebbie-room",
    "objects": "nebbie-object",
    "mobiles": "nebbie-mob",
    "zone_resets": "nebbie-zone",
}


@dataclass
class ImportCounts:
    rooms: int = 0
    objects: int = 0
    mobiles: int = 0
    zone_resets: int = 0


def eprint(*args: object) -> None:
    print(*args, file=sys.stderr)


def prompt(message: str, default: str = "") -> str:
    suffix = f" [{default}]" if default else ""
    value = input(f"{message}{suffix}: ").strip()
    return value if value else default


def validate_table_name(name: str) -> str:
    if not name:
        raise ValueError("table name cannot be empty")
    if not TABLE_NAME_RE.match(name):
        raise ValueError(
            f"invalid table name '{name}' "
            "(use letters, digits, underscore, hyphen; must start with letter or _)"
        )
    return name


def quote_ident(name: str) -> str:
    return '"' + name.replace('"', '""') + '"'


def resolve_lib_directory(path: Path) -> Path:
    path = path.resolve()
    if path.is_file():
        return path.parent
    if (path / "myst.zon").exists() or (path / "myst.wld").exists():
        return path
    nested = path / "lib"
    if nested.is_dir() and ((nested / "myst.zon").exists() or (nested / "myst.wld").exists()):
        return nested.resolve()
    return path


def find_nebbiedit() -> Optional[Path]:
    script_root = Path(__file__).resolve().parent.parent
    candidates = [
        script_root / "build" / "nebbiedit" / "nebbiedit",
        script_root / "build" / "nebbiedit",
    ]
    for candidate in candidates:
        if candidate.is_file():
            return candidate
    return None


def run_overlay_export(lib_root: Path) -> None:
    nebbiedit = find_nebbiedit()
    if nebbiedit is None:
        raise RuntimeError(
            "nebbiedit not found; build the project or export overlays manually before import"
        )
    eprint(f"Exporting overlays via {nebbiedit} ...")
    subprocess.run(
        [str(nebbiedit), "overlay", "export", str(lib_root)],
        check=True,
    )


def parse_vnum_filename(path: Path) -> Optional[int]:
    stem = path.name
    if not stem or not stem[0].isdigit():
        return None
    try:
        value = int(stem)
        return value if value > 0 else None
    except ValueError:
        return None


def parse_zone_filename(path: Path) -> Optional[int]:
    stem = path.stem
    if not stem or not stem[0].isdigit():
        return None
    try:
        value = int(stem)
        return value if value > 0 else None
    except ValueError:
        return None


def ensure_table(conn: sqlite3.Connection, table: str, kind: str) -> None:
    sql = f"""
    CREATE TABLE IF NOT EXISTS {quote_ident(table)} (
        id INTEGER PRIMARY KEY,
        kind TEXT NOT NULL,
        body TEXT NOT NULL,
        source_relpath TEXT NOT NULL,
        imported_at TEXT NOT NULL
    )
    """
    conn.execute(sql)
    conn.execute(
        f"CREATE INDEX IF NOT EXISTS {quote_ident(table + '_kind_idx')} "
        f"ON {quote_ident(table)} (kind)"
    )


def upsert_row(
    conn: sqlite3.Connection,
    table: str,
    entity_id: int,
    kind: str,
    body: str,
    source_relpath: str,
) -> None:
    imported_at = datetime.now(timezone.utc).replace(microsecond=0).isoformat()
    conn.execute(
        f"""
        INSERT INTO {quote_ident(table)} (id, kind, body, source_relpath, imported_at)
        VALUES (?, ?, ?, ?, ?)
        ON CONFLICT(id) DO UPDATE SET
            kind = excluded.kind,
            body = excluded.body,
            source_relpath = excluded.source_relpath,
            imported_at = excluded.imported_at
        """,
        (entity_id, kind, body, source_relpath, imported_at),
    )


def import_directory(
    conn: sqlite3.Connection,
    lib_root: Path,
    subdir: str,
    table: str,
    kind: str,
    id_parser,
) -> int:
    directory = lib_root / subdir
    if not directory.is_dir():
        eprint(f"Skipping {kind}: directory not found ({directory})")
        return 0

    ensure_table(conn, table, kind)
    count = 0
    for entry in sorted(directory.iterdir()):
        if not entry.is_file():
            continue
        entity_id = id_parser(entry)
        if entity_id is None:
            continue
        body = entry.read_text(encoding="utf-8", errors="surrogateescape")
        rel = f"{subdir}/{entry.name}"
        upsert_row(conn, table, entity_id, kind, body, rel)
        count += 1
    return count


def import_overlays(
    lib_root: Path,
    db_path: Path,
    table_map: Dict[str, str],
) -> ImportCounts:
    counts = ImportCounts()
    db_path.parent.mkdir(parents=True, exist_ok=True)

    with sqlite3.connect(db_path) as conn:
        conn.execute("PRAGMA foreign_keys = ON")

        if "rooms" in table_map:
            counts.rooms = import_directory(
                conn, lib_root, OVERLAY_DIRS["rooms"], table_map["rooms"], "room", parse_vnum_filename
            )
        if "objects" in table_map:
            counts.objects = import_directory(
                conn,
                lib_root,
                OVERLAY_DIRS["objects"],
                table_map["objects"],
                "object",
                parse_vnum_filename,
            )
        if "mobiles" in table_map:
            counts.mobiles = import_directory(
                conn,
                lib_root,
                OVERLAY_DIRS["mobiles"],
                table_map["mobiles"],
                "mobile",
                parse_vnum_filename,
            )
        if "zone_resets" in table_map:
            counts.zone_resets = import_directory(
                conn,
                lib_root,
                OVERLAY_DIRS["zone_resets"],
                table_map["zone_resets"],
                "zone_reset",
                parse_zone_filename,
            )

        conn.commit()

    return counts


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Import Nebbie overlay files into SQLite tables.")
    parser.add_argument("--lib", type=Path, help="mudroot/lib directory (or parent containing lib/)")
    parser.add_argument("--db", type=Path, help="SQLite database file to create or update")
    parser.add_argument("--rooms", help="SQLite table for room overlays (skip if omitted in batch mode)")
    parser.add_argument("--objects", help="SQLite table for object overlays")
    parser.add_argument("--mobiles", help="SQLite table for mobile overlays")
    parser.add_argument("--zone-resets", dest="zone_resets", help="SQLite table for zone reset overlays")
    parser.add_argument(
        "--export-first",
        action="store_true",
        help="Run nebbiedit overlay export on --lib before importing",
    )
    parser.add_argument(
        "--skip-missing",
        action="store_true",
        help="Skip import kinds whose overlay directory is absent",
    )
    return parser.parse_args()


def interactive_table_map() -> Dict[str, str]:
    eprint("Table names for each overlay import type (leave empty to skip that type).")
    table_map: Dict[str, str] = {}
    for kind, default in DEFAULT_TABLES.items():
        while True:
            label = kind.replace("_", " ")
            value = prompt(f"Table for {label}", default)
            if not value:
                eprint(f"  -> skipping {label}")
                break
            try:
                table_map[kind] = validate_table_name(value)
                break
            except ValueError as ex:
                eprint(f"  ! {ex}")
    if not table_map:
        raise SystemExit("No import types selected.")
    return table_map


def batch_table_map(args: argparse.Namespace) -> Dict[str, str]:
    mapping = {
        "rooms": args.rooms,
        "objects": args.objects,
        "mobiles": args.mobiles,
        "zone_resets": args.zone_resets,
    }
    table_map: Dict[str, str] = {}
    for kind, name in mapping.items():
        if name:
            table_map[kind] = validate_table_name(name)
    if not table_map:
        raise SystemExit("Specify at least one of --rooms --objects --mobiles --zone-resets")
    return table_map


def main() -> int:
    args = parse_args()
    interactive = args.lib is None or args.db is None

    if interactive:
        eprint("Nebbie overlay -> SQLite import")
        eprint("Import overlay file bodies verbatim (rooms/, objects/, mobiles/, zones/).")
        lib_input = prompt("Library path (mudroot/lib)", "")
        if not lib_input:
            raise SystemExit("Library path is required.")
        db_input = prompt("SQLite database path", "nebbie-overlays.sqlite3")
        lib_root = resolve_lib_directory(Path(lib_input))
        db_path = Path(db_input).expanduser()
        export_first = prompt("Export overlays from myst.* first? [y/N]", "N").lower() in {"y", "yes", "s", "si"}
        table_map = interactive_table_map()
    else:
        lib_root = resolve_lib_directory(args.lib)
        db_path = args.db.expanduser()
        export_first = args.export_first
        table_map = batch_table_map(args)

    if not lib_root.is_dir():
        raise SystemExit(f"Library directory not found: {lib_root}")

    if export_first:
        run_overlay_export(lib_root)

    missing = [kind for kind in table_map if not (lib_root / OVERLAY_DIRS[kind]).is_dir()]
    if missing and not interactive and not args.skip_missing:
        names = ", ".join(OVERLAY_DIRS[k] + "/" for k in missing)
        raise SystemExit(f"Missing overlay directories under {lib_root}: {names}")

    counts = import_overlays(lib_root, db_path, table_map)

    print("Import completed.")
    print(f"  database: {db_path.resolve()}")
    for kind, table in table_map.items():
        imported = getattr(counts, kind)
        print(f"  {kind}: {imported} row(s) -> {table}")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except subprocess.CalledProcessError as ex:
        eprint(f"nebbiedit overlay export failed with exit code {ex.returncode}")
        raise SystemExit(ex.returncode)
    except KeyboardInterrupt:
        eprint("\nCancelled.")
        raise SystemExit(130)
