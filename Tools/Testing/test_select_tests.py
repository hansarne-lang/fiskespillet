import fnmatch
import unittest
from pathlib import Path

import select_tests as st


class SelectTests(unittest.TestCase):
    def test_docs_only_selects_nothing(self):
        self.assertEqual(st.select(['README.md', 'Docs/Development/FISH_FIGHT.md', 'Tools/Validation/check_repository.py']), [])

    def test_core_and_controller_select_everything(self):
        for path in ('Source/FiskCore/Public/FiskCareerProfile.h', 'Source/FiskGame/FiskGame.Build.cs',
                     'Source/FiskGame/Private/DayCycle/FiskDayState.cpp', 'Config/DefaultGame.ini',
                     'Source/FiskGame/Private/Boats/FiskBoardingController.Career.cpp',
                     'Tools/Testing/FiskTestCommon.ps1'):
            self.assertEqual(st.select([path]), st.ALL, path)

    def test_unknown_path_is_fail_safe(self):
        self.assertEqual(st.select(['Source/FiskGame/Private/NewArea/FiskThing.cpp']), st.ALL)
        self.assertEqual(st.select(['Content/FiskELaget/Fishing/Rod.uasset']), st.ALL)

    def test_fishing_change_selects_fishing_scripts(self):
        result = st.select(['Source/FiskGame/Private/Fishing/FiskFightModel.cpp'])
        self.assertIn('FishCombat', result)
        self.assertIn('RigSetup', result)
        self.assertNotIn('MainMenu', result)

    def test_specific_rule_wins_over_folder(self):
        self.assertEqual(st.select(['Source/FiskGame/Private/UI/FiskFrontEnd.cpp']), ['MainMenu', 'SoloPlay'])

    def test_changed_runner_selects_itself(self):
        self.assertEqual(st.select(['Tools/Testing/Run-SeaChart.ps1', 'README.md']), ['SeaChart'])

    def test_rules_only_name_existing_scripts(self):
        scripts = set(st.available_scripts())
        self.assertTrue(scripts, 'no Run-*.ps1 scripts found')
        for pattern, targets in st.RULES:
            for target in targets:
                self.assertIn(target, scripts, pattern)

    def test_missing_script_is_fail_safe(self):
        self.assertEqual(st.select(['Source/FiskGame/Private/Fishing/FiskFightModel.cpp'], scripts=['FishCombat']), st.ALL)

    def test_every_source_file_is_classified(self):
        root = Path(st.ROOT)
        unclassified = []
        for path in sorted((root / 'Source').rglob('*')):
            if path.suffix not in ('.cpp', '.h', '.cs'):
                continue
            rel = path.relative_to(root).as_posix()
            if st._match(rel, st.EVERYTHING):
                continue
            if not any(fnmatch.fnmatchcase(rel, pattern) for pattern, _ in st.RULES):
                unclassified.append(rel)
        # Unclassified files still run everything; this lists them so rules can be added on purpose.
        self.assertEqual([p for p in unclassified if '/UI/' not in p], [])

    def test_windows_separators(self):
        self.assertEqual(st.select(['Tools\\Testing\\Run-SeaChart.ps1']), ['SeaChart'])


if __name__ == '__main__':
    unittest.main()
