"""run_cache.py: a script is skipped only when build, script, harness and clock scale are unchanged."""
import io
import tempfile
import unittest
from contextlib import redirect_stdout
from pathlib import Path

import run_cache

EXE = 'A' * 64
OTHER = 'B' * 64


class Cache(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.folder = Path(self.temp.name)
        for name in ('Run-FishCombat.ps1', 'Run-SeaChart.ps1') + run_cache.HARNESS:
            (self.folder / name).write_text(f'# {name}\n', encoding='utf-8')
        self.cache = self.folder / 'logs' / 'passed-cache.jsonl'

    def tearDown(self):
        self.temp.cleanup()

    def skip(self, exe=EXE, items=(('FishCombat', 4), ('SeaChart', 1))):
        return run_cache.skip(self.cache, exe, list(items), self.folder)

    def test_empty_cache_runs_everything(self):
        self.assertEqual([], self.skip())

    def test_passed_script_is_skipped_on_same_build(self):
        run_cache.record(self.cache, EXE, [('FishCombat', 4)], 'all-1', self.folder)
        self.assertEqual(['FishCombat'], self.skip())

    def test_new_build_runs_again(self):
        run_cache.record(self.cache, EXE, [('FishCombat', 4)], 'all-1', self.folder)
        self.assertEqual([], self.skip(exe=OTHER))

    def test_exe_hash_case_does_not_matter(self):
        run_cache.record(self.cache, EXE.lower(), [('FishCombat', 4)], 'all-1', self.folder)
        self.assertEqual(['FishCombat'], self.skip())

    def test_other_clock_scale_runs_again(self):
        run_cache.record(self.cache, EXE, [('FishCombat', 4)], 'all-1', self.folder)
        self.assertEqual([], self.skip(items=[('FishCombat', 1)]))

    def test_changed_script_runs_again(self):
        run_cache.record(self.cache, EXE, [('FishCombat', 4)], 'all-1', self.folder)
        (self.folder / 'Run-FishCombat.ps1').write_text('# changed\n', encoding='utf-8')
        self.assertEqual([], self.skip())

    def test_changed_harness_runs_everything_again(self):
        run_cache.record(self.cache, EXE, [('FishCombat', 4), ('SeaChart', 1)], 'all-1', self.folder)
        (self.folder / 'FiskTestCommon.ps1').write_text('# changed\n', encoding='utf-8')
        self.assertEqual([], self.skip())

    def test_damaged_line_is_ignored(self):
        run_cache.record(self.cache, EXE, [('FishCombat', 4)], 'all-1', self.folder)
        with self.cache.open('a', encoding='utf-8') as handle:
            handle.write('{not json\n')
        self.assertEqual(['FishCombat'], self.skip())

    def test_command_line(self):
        run_cache.record(self.cache, EXE, [('SeaChart', 1)], 'all-1', self.folder)
        out = io.StringIO()
        original = run_cache.TESTING
        run_cache.TESTING = self.folder
        try:
            with redirect_stdout(out):
                self.assertEqual(0, run_cache.main(['skip', '--cache', str(self.cache), '--exe', EXE, 'SeaChart:1', 'FishCombat:4']))
        finally:
            run_cache.TESTING = original
        self.assertEqual('SeaChart\n', out.getvalue())

    def test_bad_arguments_are_refused(self):
        with self.assertRaises(ValueError):
            run_cache.parse(['FishCombat'])
        with self.assertRaises(SystemExit):
            run_cache.main(['skip', '--cache', str(self.cache), '--exe', 'not-a-hash', 'FishCombat:4'])


if __name__ == '__main__':
    unittest.main()
