#!/usr/bin/env python3
"""Remembers which graphical test scripts already passed on exactly the same build.

Run-All.ps1 asks before it starts (skip) and records the scripts that passed (record). A script is
skipped only when the packaged game binary, the script itself, the shared test harness
(FiskTestCommon.ps1, FiskRunAllLib.ps1, Run-All.ps1) and the clock scale are all unchanged.
Release runs never use the cache.

  python Tools/Testing/run_cache.py skip   --cache <logs>/passed-cache.jsonl --exe <SHA256> FishCombat:4 SeaChart:1
  python Tools/Testing/run_cache.py record --cache <logs>/passed-cache.jsonl --exe <SHA256> --evidence <run> FishCombat:4
skip prints one script name per line that does not need to run again.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import sys
from pathlib import Path

TESTING = Path(__file__).resolve().parent
HARNESS = ('FiskTestCommon.ps1', 'FiskRunAllLib.ps1', 'Run-All.ps1')


def script_hash(name: str, folder: Path | None = None) -> str:
    folder = folder or TESTING
    digest = hashlib.sha256()
    for file in (f'Run-{name}.ps1',) + HARNESS:
        path = folder / file
        digest.update(file.encode())
        digest.update(path.read_bytes() if path.is_file() else b'<missing>')
    return digest.hexdigest()


def parse(items: list[str]) -> list[tuple[str, int]]:
    result = []
    for item in items:
        name, _, scale = item.partition(':')
        if not name or not scale.isdigit():
            raise ValueError(f'expected Name:Scale, got {item!r}')
        result.append((name, int(scale)))
    return result


def key(exe: str, name: str, scale: int, folder: Path | None) -> tuple[str, str, str, int]:
    return exe.upper(), name, script_hash(name, folder), scale


def load(cache: Path) -> set[tuple[str, str, str, int]]:
    known = set()
    if not cache.is_file():
        return known
    for line in cache.read_text(encoding='utf-8').splitlines():
        try:
            row = json.loads(line)
            known.add((row['exe'].upper(), row['script'], row['script_sha'], int(row['scale'])))
        except (ValueError, KeyError, TypeError):
            continue  # A damaged line only means that script runs again.
    return known


def skip(cache: Path, exe: str, items: list[tuple[str, int]], folder: Path | None = None) -> list[str]:
    known = load(cache)
    return [name for name, scale in items if key(exe, name, scale, folder) in known]


def record(cache: Path, exe: str, items: list[tuple[str, int]], evidence: str, folder: Path | None = None) -> None:
    cache.parent.mkdir(parents=True, exist_ok=True)
    with cache.open('a', encoding='utf-8') as handle:
        for name, scale in items:
            exe_sha, script, script_sha, scale = key(exe, name, scale, folder)
            handle.write(json.dumps({'exe': exe_sha, 'script': script, 'script_sha': script_sha,
                                     'scale': scale, 'evidence': evidence}) + '\n')


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('command', choices=('skip', 'record'))
    parser.add_argument('--cache', required=True)
    parser.add_argument('--exe', required=True)
    parser.add_argument('--evidence', default='')
    parser.add_argument('items', nargs='*', help='Name:ClockScale')
    args = parser.parse_args(argv)
    items = parse(args.items)
    if len(args.exe) != 64 or any(c not in '0123456789abcdefABCDEF' for c in args.exe):
        parser.error('--exe must be a SHA256')
    if args.command == 'skip':
        for name in skip(Path(args.cache), args.exe, items):
            print(name)
    else:
        record(Path(args.cache), args.exe, items, args.evidence)
    return 0


if __name__ == '__main__':
    sys.exit(main())
