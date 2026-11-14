#!/usr/bin/env python3
"""Choose which Tools/Testing/Run-*.ps1 scripts a change needs.

Fail-safe: a changed path that no rule recognises selects every script. Paths that cannot
affect the packaged game (documentation, repository validation) select nothing.

Usage:
  python Tools/Testing/select_tests.py --base origin/main          # compare HEAD with a ref
  python Tools/Testing/select_tests.py Source/FiskGame/Private/Fishing/FiskFightModel.cpp
Prints one script name per line (without .ps1), or ALL when everything must run.
"""
from __future__ import annotations

import argparse
import fnmatch
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
TESTING = ROOT / 'Tools' / 'Testing'
ALL = 'ALL'

# Paths that never change the packaged game or the graphical test runners.
IGNORED = (
    '*.md', 'Docs/*', 'Tools/Validation/*', 'Tools/Coordination/*', '.github/ISSUE_TEMPLATE/*',
    '.github/CODEOWNERS', '.github/pull_request_template.md', '.github/project-blueprint.json',
    'Tools/Testing/select_tests.py', 'Tools/Testing/test_select_tests.py', 'Tools/Testing/Test-RunAllLib.ps1', 'Tools/Build/*',
    'Tools/Testing/report.py', 'Tools/Testing/test_report.py', 'Tools/Testing/known_failures.json',
    'Tools/Testing/run_cache.py', 'Tools/Testing/test_run_cache.py', 'Tools/Testing/Test-PR.ps1',
    '*.docx', '.gitignore',
    'Grafikk/*',  # SpilLan card image: shown by the launcher, never loaded by the game
)

# Automation test sources (*Test.cpp) only add automation tests, which run at every level except Build.
# They never change what a graphical script sees, so they select no script, even in shared folders.
AUTOMATION_TESTS = ('Source/*Test.cpp',)

# FiskCore headers used by one area only. Checked before EVERYTHING; any other FiskCore file still
# selects every script.
CORE_RULES: tuple[tuple[str, list[str]], ...] = (
    ('Source/FiskCore/Public/FiskPier*', ['HarbourGuide', 'MarinaRelease']),
    ('Source/FiskCore/Public/FiskGreetingClock.h', ['HarbourGuide', 'MarinaRelease']),
    ('Source/FiskCore/Public/FiskHarbourRules.h', ['HarbourGuide', 'MarinaRelease']),
    ('Source/FiskCore/*/FiskRodStick*', ['FishCombat', 'RepeatedFishing', 'RigSetup', 'FishingRelease', 'ContentRelease', 'AnglerRelease']),
    ('Source/FiskCore/Public/FiskVoyageRules.h', ['SeaChart', 'NavigationRelease', 'CaptainTransfer', 'SkipperRelease',
                                                  'FullDay', 'HarbourGuide', 'ExplorationRelease']),
    ('Source/FiskCore/Public/FiskHelm.h', ['SeaChart', 'NavigationRelease', 'CaptainTransfer', 'SkipperRelease']),
)

# Shared foundations: a change here can affect any scenario.
EVERYTHING = (
    'Source/FiskCore/*', '*.Build.cs', '*.Target.cs', '*.uproject', 'Config/*',
    'Source/FiskGame/Public/*', 'Source/FiskGame/Private/FiskGameModule.cpp',
    'Source/FiskGame/Private/Bootstrap/*', 'Source/FiskGame/Private/DayCycle/*',
    'Source/FiskGame/Private/Boats/FiskBoardingController*', 'Source/FiskGame/Private/UI/FiskGameInstance*',
    'Tools/Testing/FiskTestCommon.ps1', 'Tools/Testing/Run-All.ps1', 'Tools/Testing/FiskRunAllLib.ps1', '.gitattributes',
)

FISHING = ['FishCombat', 'RepeatedFishing', 'RigSetup', 'FishingRelease', 'ContentRelease', 'AnglerRelease']
NAVIGATION = ['SeaChart', 'NavigationRelease', 'CaptainTransfer', 'SkipperRelease']
OFFSHORE = ['ExplorationRelease', 'MangroveRelease', 'ExpeditionRelease', 'SkipperRelease', 'AnglerRelease']
CAREER = ['CareerRelease', 'JournalRelease', 'MarinaRelease', 'ExpeditionRelease']
DAY = ['FullDay', 'SessionRelease', 'ContentRelease', 'FishingRelease']
MENU = ['MainMenu', 'SoloPlay']

# First matching rule wins, so specific files come before their folders.
RULES: tuple[tuple[str, list[str]], ...] = (
    ('Source/FiskGame/Private/UI/FiskMenu*', MENU),
    ('Source/FiskGame/Private/UI/FiskFrontEnd*', MENU),
    ('Source/FiskGame/Private/UI/FiskSoloSmoke*', ['SoloPlay']),
    ('Source/FiskGame/Private/UI/FiskMarina*', ['MarinaRelease']),
    ('Source/FiskGame/Private/UI/FiskRig*', ['RigSetup', 'FishingRelease', 'AnglerRelease']),
    ('Source/FiskGame/Private/UI/FiskSeaChart*', ['SeaChart', 'NavigationRelease']),
    ('Source/FiskGame/Private/UI/FiskNavigation*', ['NavigationRelease', 'SeaChart']),
    ('Source/FiskGame/Private/UI/FiskJournal*', ['JournalRelease', 'CareerRelease']),
    ('Source/FiskGame/Private/UI/FiskCareer*', CAREER),
    ('Source/FiskGame/Private/UI/FiskContent*', ['ContentRelease', 'SessionRelease']),
    ('Source/FiskGame/Private/UI/FiskSession*', ['SessionRelease', 'FullDay']),
    ('Source/FiskGame/Private/UI/FiskFishingRelease*', ['FishingRelease']),
    ('Source/FiskGame/Private/UI/FiskAngler*', ['AnglerRelease']),
    ('Source/FiskGame/Private/UI/FiskExpedition*', ['ExpeditionRelease']),
    ('Source/FiskGame/Private/UI/FiskSkipper*', ['SkipperRelease']),
    ('Source/FiskGame/Private/Boats/FiskCaptain*', ['CaptainTransfer', 'SeaChart']),
    ('Source/FiskGame/Private/Boats/FiskMarinaZone*', ['SeaChart', 'NavigationRelease', 'FullDay']),
    ('Source/FiskGame/Private/Boats/Controller/FiskMarinaSync*', ['MarinaRelease', 'HarbourGuide']),
    ('Source/FiskGame/Private/Boats/Controller/FiskRecoverySync*', CAREER),
    ('Source/FiskGame/Private/Boats/Controller/FiskCareerSync*', CAREER + ['FullDay', 'SessionRelease']),
    ('Source/FiskGame/Private/Boats/Controller/FiskFleetSync*', NAVIGATION + ['FullDay', 'HarbourGuide', 'MarinaRelease']),
    ('Source/FiskGame/Private/Boats/Controller/FiskExpeditionCommands*', ['ExpeditionRelease', 'ExplorationRelease', 'MangroveRelease']),
    ('Source/FiskGame/Private/Boats/Controller/FiskBoatCommands*', NAVIGATION + ['FullDay', 'SessionRelease', 'FishingRelease', 'ExplorationRelease', 'HarbourGuide']),
    ('Source/FiskGame/Private/Boats/Controller/FiskRodCommands*', FISHING + ['FullDay', 'CareerRelease']),
    ('Source/FiskGame/Private/Fishing/*', FISHING),
    ('Source/FiskGame/Private/Angler/*', ['AnglerRelease', 'MangroveRelease']),
    ('Source/FiskGame/Private/Boats/*', NAVIGATION + ['FullDay', 'HarbourGuide']),
    ('Source/FiskGame/Private/Voyage/*', NAVIGATION),
    ('Source/FiskGame/Private/World/FiskPier*', ['HarbourGuide']),
    ('Source/FiskGame/Private/World/*', ['SeaChart', 'ExplorationRelease', 'MangroveRelease', 'HarbourGuide']),
    ('Source/FiskGame/Private/Expedition/*', ['ExpeditionRelease', 'ExplorationRelease']),
    ('Source/FiskGame/Private/Economy/*', DAY),
    ('Source/FiskGame/Private/Career/*', CAREER),
    ('Source/FiskGame/Private/Collections/*', ['MangroveRelease', 'CareerRelease']),
)


def available_scripts() -> list[str]:
    return sorted(p.stem[len('Run-'):] for p in TESTING.glob('Run-*.ps1') if p.stem != 'Run-All')


def _match(path: str, patterns) -> bool:
    return any(fnmatch.fnmatchcase(path, pattern) for pattern in patterns)


def select(paths, scripts: list[str] | None = None) -> list[str] | str:
    """Return sorted script names, or ALL."""
    scripts = available_scripts() if scripts is None else scripts
    chosen: set[str] = set()
    for raw in paths:
        path = raw.strip().replace('\\', '/')
        if not path:
            continue
        if _match(path, AUTOMATION_TESTS):
            continue
        core = next((targets for pattern, targets in CORE_RULES if fnmatch.fnmatchcase(path, pattern)), None)
        if core is not None:
            chosen.update(core)
            continue
        if _match(path, EVERYTHING):
            return ALL
        if fnmatch.fnmatchcase(path, 'Tools/Testing/Run-*.ps1'):
            chosen.add(Path(path).stem[len('Run-'):])
            continue
        if _match(path, IGNORED):
            continue
        for pattern, targets in RULES:
            if fnmatch.fnmatchcase(path, pattern):
                chosen.update(targets)
                break
        else:
            return ALL  # Unknown path: run everything rather than guess.
    known = set(scripts)
    missing = sorted(chosen - known)
    if missing:
        # A rule points at a script that no longer exists: be safe.
        return ALL
    return sorted(chosen)


def changed_paths(base: str, committed_only: bool = False) -> list[str]:
    merge_base = subprocess.run(['git', 'merge-base', base, 'HEAD'], cwd=ROOT, check=True,
                                capture_output=True, text=True).stdout.strip()
    committed = subprocess.run(['git', 'diff', '--name-only', merge_base, 'HEAD'], cwd=ROOT, check=True,
                               capture_output=True, text=True).stdout.splitlines()
    if committed_only:
        return sorted(set(committed))
    working = subprocess.run(['git', 'diff', '--name-only', 'HEAD'], cwd=ROOT, check=True,
                             capture_output=True, text=True).stdout.splitlines()
    return sorted(set(committed) | set(working))


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('paths', nargs='*', help='Changed repository-relative paths')
    parser.add_argument('--base', help='Git ref to compare with (uses the merge base with HEAD)')
    parser.add_argument('--committed-only', action='store_true',
                        help='Ignore uncommitted changes (CI: Unreal rewrites some tracked files while building)')
    args = parser.parse_args(argv)
    if bool(args.paths) == bool(args.base):
        parser.error('give either paths or --base')
    paths = args.paths or changed_paths(args.base, args.committed_only)
    result = select(paths)
    if result == ALL:
        print(ALL)
    else:
        for name in result:
            print(name)
    return 0


if __name__ == '__main__':
    sys.exit(main())
