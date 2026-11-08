#!/usr/bin/env python3
"""Enforce include boundaries between the logical areas in Source/FiskGame (ARCHITECTURE.md).

An area is a folder directly under Source/FiskGame/Private. A file in one area may include:
  - its own area,
  - FiskCore and Source/FiskGame/Public headers,
  - Unreal Engine headers,
  - other areas only where ALLOWED says so (the integration layer may include everything).

Existing violations are listed in include_exceptions.json. That list may only shrink: a new
violation fails, and so does an exception that is no longer used.

  python Tools/Validation/check_includes.py          # check
  python Tools/Validation/check_includes.py --prune  # drop exceptions that are no longer used
"""
from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path

PRIVATE = Path('Source/FiskGame/Private')
PUBLIC = Path('Source/FiskGame/Public')
EXCEPTIONS = Path('Tools/Validation/include_exceptions.json')
INTEGRATION = '(integration)'  # Bootstrap/ and FiskGameModule.cpp

# Allowed area -> area dependencies beyond the default rules. Bootstrap is the integration layer
# in ARCHITECTURE.md and may assemble every area. All other areas talk through FiskCore.
ALLOWED: dict[str, set[str] | str] = {
    'Bootstrap': '*',
    INTEGRATION: '*',
}

INCLUDE = re.compile(r'^\s*#\s*include\s*"([^"]+)"', re.MULTILINE)
COMMENTS = re.compile(r'//[^\n]*|/\*.*?\*/', re.DOTALL)


def areas(root: Path) -> set[str]:
    base = root / PRIVATE
    return {p.name for p in base.iterdir() if p.is_dir()} if base.is_dir() else set()


def area_of(root: Path, path: Path) -> str:
    rel = path.relative_to(root)
    if rel.parts[:3] == PUBLIC.parts:
        return 'Public'
    if rel.parts[:3] == PRIVATE.parts:
        return rel.parts[3] if len(rel.parts) > 4 else INTEGRATION
    raise ValueError(f'not in FiskGame: {rel}')


def target_area(include: str, known: set[str]) -> str | None:
    """Area named by an include, or None for own-folder, FiskCore, Public and engine headers."""
    first = include.replace('\\', '/').split('/', 1)
    return first[0] if len(first) == 2 and first[0] in known else None


def allowed(source: str, target: str) -> bool:
    if source == target:
        return True
    rule = ALLOWED.get(source, set())
    return rule == '*' or target in rule


def violations(root: Path) -> dict[str, list[str]]:
    known = areas(root)
    found: dict[str, list[str]] = {}
    for folder in (root / PRIVATE, root / PUBLIC):
        if not folder.is_dir():
            continue
        for path in sorted(folder.rglob('*')):
            if path.suffix.lower() not in ('.h', '.cpp', '.inl'):
                continue
            source = area_of(root, path)
            text = COMMENTS.sub('', path.read_text(encoding='utf-8', errors='replace'))
            bad = sorted({inc for inc in INCLUDE.findall(text)
                          if (target := target_area(inc, known)) and not allowed(source, target)})
            if bad:
                found[path.relative_to(root).as_posix()] = bad
    return found


def load_exceptions(root: Path) -> dict[str, list[str]]:
    path = root / EXCEPTIONS
    if not path.is_file():
        return {}
    data = json.loads(path.read_text(encoding='utf-8'))
    return {file: list(includes) for file, includes in data.get('exceptions', {}).items()}


def validate(root: Path) -> list[str]:
    errors: list[str] = []
    try:
        exceptions = load_exceptions(root)
    except (OSError, ValueError, AttributeError) as error:
        return [f'Unreadable {EXCEPTIONS.as_posix()}: {error}']
    current = violations(root)
    for file, includes in current.items():
        for include in includes:
            if include not in exceptions.get(file, []):
                errors.append(f'Forbidden include across areas: {file} includes "{include}"')
    for file, includes in exceptions.items():
        for include in includes:
            if include not in current.get(file, []):
                errors.append(f'Stale include exception (remove it with --prune): {file} -> "{include}"')
    return errors


def write_exceptions(root: Path, data: dict[str, list[str]]) -> None:
    payload = {
        '_comment': 'Existing cross-area includes from before the boundary check (issue #114). '
                    'This list may only shrink. Remove entries with: python Tools/Validation/check_includes.py --prune',
        'exceptions': {file: sorted(includes) for file, includes in sorted(data.items())},
    }
    (root / EXCEPTIONS).write_text(json.dumps(payload, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')


def prune(root: Path) -> int:
    """Remove exceptions that are no longer used. Never adds new ones."""
    exceptions = load_exceptions(root)
    current = violations(root)
    kept = {file: [inc for inc in incs if inc in current.get(file, [])] for file, incs in exceptions.items()}
    kept = {file: incs for file, incs in kept.items() if incs}
    removed = sum(map(len, exceptions.values())) - sum(map(len, kept.values()))
    write_exceptions(root, kept)
    return removed


def summary(root: Path) -> dict[str, int]:
    counts: dict[str, int] = {}
    for file, includes in violations(root).items():
        source = area_of(root, root / file)
        for include in includes:
            key = f'{source} -> {target_area(include, areas(root))}'
            counts[key] = counts.get(key, 0) + 1
    return dict(sorted(counts.items(), key=lambda item: (-item[1], item[0])))


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('--prune', action='store_true', help='remove exceptions that are no longer used')
    parser.add_argument('--summary', action='store_true', help='print violations per area pair')
    args = parser.parse_args(argv)
    root = Path(__file__).resolve().parents[2]
    if args.prune:
        print(f'Removed {prune(root)} unused exception(s).')
    if args.summary:
        print(json.dumps(summary(root), indent=2))
        return 0
    errors = validate(root)
    total = sum(map(len, load_exceptions(root).values()))
    print(json.dumps({'check': 'include-boundaries', 'status': 'failed' if errors else 'passed',
                      'remaining_exceptions': total, 'errors': errors}, ensure_ascii=False, indent=2))
    return 1 if errors else 0


if __name__ == '__main__':
    sys.exit(main())
