#!/usr/bin/env python3
"""Short Markdown test report, so nobody (human or AI) has to read raw logs.

Reads what the test tools already write:
  --automation  index.json from Tools/Build/Test-Automation.ps1
  --run         the all-<time> folder from Tools/Testing/Run-All.ps1 (summary.csv, *.err.txt, logs)
  --exe         the packaged game binary (its SHA256 goes in the header)
For each failed script only the first FISKELAGET_*_FAILED lines (at most three) are shown, with
file and line. Failures listed in known_failures.json are marked "kjent (#issue)"; everything else
is NY. Exit code 0 when there are no new failures, 1 when there are.

  python Tools/Testing/report.py --level PR --exe <pakke>/FiskELaget/Binaries/Win64/FiskELaget.exe \
      --automation <logs>/automation/<time>/index.json --run <logs>/all-<time> --out <logs>/report.md
"""
from __future__ import annotations

import argparse
import csv
import hashlib
import json
import re
import subprocess
import sys
from dataclasses import dataclass, field
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
KNOWN = Path(__file__).resolve().parent / 'known_failures.json'
MARKER = re.compile(r'FISKELAGET_[A-Z0-9_]*FAILED\b[^\r\n]*')
PERSONAL = re.compile(r'[A-Za-z]:[\\/]+Users[\\/]+[^\\/\s]+', re.IGNORECASE)
MAX_LINES = 3


@dataclass
class Failure:
    text: str
    where: str = ''
    issue: int | None = None


@dataclass
class Script:
    name: str
    result: str
    seconds: int
    scale: str
    failures: list[Failure] = field(default_factory=list)

    @property
    def new(self) -> bool:
        return self.result == 'Failed' and (not self.failures or any(f.issue is None for f in self.failures))


def redact(text: str) -> str:
    return PERSONAL.sub('%USERPROFILE%', text)


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open('rb') as handle:
        for block in iter(lambda: handle.read(1 << 20), b''):
            digest.update(block)
    return digest.hexdigest().upper()


def load_known(path: Path) -> list[dict]:
    if not path.is_file():
        return []
    data = json.loads(path.read_text(encoding='utf-8'))
    entries = data.get('known', [])
    for entry in entries:
        if not isinstance(entry.get('issue'), int) or not entry.get('match'):
            raise ValueError(f'known_failures.json: every entry needs an issue number and a match: {entry}')
        re.compile(entry['match'])
    return entries


def classify(script: str, text: str, known: list[dict]) -> int | None:
    for entry in known:
        scripts = entry.get('scripts') or []
        if scripts and script not in scripts:
            continue
        if re.search(entry['match'], text):
            return entry['issue']
    return None


def first_error_line(path: Path) -> str:
    """First meaningful line of an err.txt; PowerShell progress CLIXML is skipped."""
    if not path.is_file():
        return ''
    text = path.read_text(encoding='utf-8', errors='replace')
    if text.lstrip().startswith('#< CLIXML'):
        return ''
    for line in text.splitlines():
        line = line.strip()
        if line:
            return line[:300]
    return ''


def markers(folder: Path, run: Path) -> list[Failure]:
    found: list[Failure] = []
    seen: set[str] = set()
    if not folder.is_dir():
        return found
    for log in sorted(folder.rglob('*.log')):
        for number, line in enumerate(log.read_text(encoding='utf-8', errors='replace').splitlines(), 1):
            match = MARKER.search(line)
            if not match:
                continue
            text = re.sub(r'\bplayer=\d+\s*', '', match.group(0)).strip()
            if text in seen:
                continue
            seen.add(text)
            found.append(Failure(text, f'{log.relative_to(run).as_posix()}:{number}'))
            if len(found) >= MAX_LINES:
                return found
    return found


def read_run(run: Path, known: list[dict]) -> list[Script]:
    summary = run / 'summary.csv'
    if not summary.is_file():
        raise FileNotFoundError(f'summary.csv missing in {run}')
    scripts = []
    with summary.open(encoding='utf-8-sig', newline='') as handle:
        for row in csv.DictReader(handle):
            item = Script(row['Script'], row['Result'], int(float(row.get('Seconds') or 0)), row.get('ClockScale', ''))
            if item.result == 'Failed':
                found = markers(run / item.name, run)
                error = first_error_line(run / f'{item.name}.err.txt')
                if error and not any(error in f.text or f.text in error for f in found):
                    found.insert(0, Failure(error, f'{item.name}.err.txt:1'))
                for failure in found[:MAX_LINES]:
                    failure.issue = classify(item.name, failure.text, known)
                    item.failures.append(failure)
            scripts.append(item)
    return scripts


def read_automation(index: Path, known: list[dict]) -> tuple[int, int, int, list[Script]]:
    data = json.loads(index.read_text(encoding='utf-8-sig'))
    passed = int(data.get('succeeded', 0)) + int(data.get('succeededWithWarnings', 0))
    failed, not_run = int(data.get('failed', 0)), int(data.get('notRun', 0))
    failures = []
    for test in data.get('tests', []):
        if test.get('state') != 'Fail':
            continue
        name = test.get('fullTestPath') or test.get('testDisplayName') or '?'
        message = next((e.get('event', {}).get('message', '') for e in test.get('entries', [])
                        if e.get('event', {}).get('type') == 'Error'), '') or 'Fail'
        item = Script(name, 'Failed', 0, '')
        item.failures.append(Failure(message.strip()[:300], 'index.json', classify(name, f'{name} {message}', known)))
        failures.append(item)
    return passed, failed, not_run, failures


def commit() -> str:
    try:
        return subprocess.run(['git', 'rev-parse', 'HEAD'], cwd=ROOT, capture_output=True, text=True, check=True).stdout.strip()
    except (OSError, subprocess.CalledProcessError):
        return 'ukjent'


def cell(text: str) -> str:
    return redact(text).replace('|', '\\|')


def render(level: str, source: str, exe_hash: str, automation, scripts: list[Script] | None, run: Path | None) -> tuple[str, int]:
    failed_items = [s for s in (scripts or []) if s.result == 'Failed'] + (automation[3] if automation else [])
    new = [s for s in failed_items if s.new]
    known = [s for s in failed_items if not s.new]
    if new:
        verdict = f'**FEILET: {len(new)} ny(e) feil**' + (f', {len(known)} kjent(e)' if known else '')
    elif known:
        verdict = f'**Ingen nye feil.** {len(known)} kjent(e) feil, se issue-lenkene'
    else:
        verdict = '**BESTÅTT**'
    lines = [f'## Testrapport – nivå {level}', '',
             f'Commit `{source[:12]}` · spillbinær SHA256 `{exe_hash}`', '', verdict, '',
             '| Del | Bestått | Feilet | Ikke kjørt / cache | Tid |', '|---|---|---|---|---|']
    if automation:
        passed, failed, not_run, _ = automation
        lines.append(f'| Automasjon | {passed} | {failed} | {not_run} | |')
    if scripts is not None:
        cached = [s for s in scripts if s.result == 'Cached']
        scales = sorted({s.scale for s in scripts if s.scale})
        lines.append(f'| Røyktester (klokkefaktor {", ".join(scales) or "?"}) | {sum(s.result == "Passed" for s in scripts)} '
                     f'| {sum(s.result == "Failed" for s in scripts)} | {len(cached)} | {sum(s.seconds for s in scripts)} s |')
    for title, items in (('Nye feil', new), ('Kjente feil', known)):
        if not items:
            continue
        lines += ['', f'### {title}', '', '| Test | Feil | Bevis | Issue |', '|---|---|---|---|']
        for item in items:
            for failure in item.failures or [Failure('ingen feillinje funnet; se loggen', '')]:
                issue = f'#{failure.issue}' if failure.issue else 'NY'
                lines.append(f'| {cell(item.name)} | `{cell(failure.text)}` | {cell(failure.where)} | {issue} |')
    if scripts:
        cached_names = [s.name for s in scripts if s.result == 'Cached']
        if cached_names:
            lines += ['', f'Ikke kjørt på nytt (bestod før med samme bygg): {", ".join(cached_names)}.']
    if run:
        lines += ['', f'Bevis: `{cell(str(run))}` (stier over er relative til denne mappa).']
    return '\n'.join(lines) + '\n', 1 if new else 0


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('--level', default='PR')
    parser.add_argument('--exe', help='Packaged game binary')
    parser.add_argument('--automation', help='index.json from Test-Automation.ps1')
    parser.add_argument('--run', help='all-<time> folder from Run-All.ps1')
    parser.add_argument('--known', default=str(KNOWN))
    parser.add_argument('--commit', default='')
    parser.add_argument('--out', help='Also write the report to this file')
    args = parser.parse_args(argv)
    known = load_known(Path(args.known))
    exe_hash = sha256(Path(args.exe)) if args.exe else 'ikke oppgitt'
    automation = read_automation(Path(args.automation), known) if args.automation else None
    run = Path(args.run) if args.run else None
    scripts = read_run(run, known) if run else None
    text, code = render(args.level, args.commit or commit(), exe_hash, automation, scripts, run)
    if args.out:
        Path(args.out).write_text(text, encoding='utf-8')
    sys.stdout.write(text)
    return code


if __name__ == '__main__':
    sys.exit(main())
