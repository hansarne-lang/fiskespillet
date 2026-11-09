"""report.py: short reports, known versus new failures, personal paths never shown."""
import json
import tempfile
import unittest
from pathlib import Path

import report

SUMMARY = 'Script,Result,ExitCode,Seconds,Slot,ClockScale\n'


class Report(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.root = Path(self.temp.name)
        self.run = self.root / 'all-1'
        self.run.mkdir()
        self.known = self.root / 'known.json'
        self.known.write_text(json.dumps({'known': [
            {'issue': 135, 'scripts': ['SoloPlay'], 'match': 'solo-home'},
            {'issue': 136, 'scripts': [], 'match': 'atlas_not_open'}]}), encoding='utf-8')

    def tearDown(self):
        self.temp.cleanup()

    def write(self, relative, text):
        path = self.run / relative
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(text, encoding='utf-8')

    def build(self, rows):
        self.write('summary.csv', SUMMARY + ''.join(rows))
        return report.main(['--run', str(self.run), '--known', str(self.known), '--commit', 'a' * 40,
                            '--out', str(self.root / 'report.md')])

    def text(self):
        return (self.root / 'report.md').read_text(encoding='utf-8')

    def test_all_passed(self):
        self.assertEqual(0, self.build(['FishCombat,Passed,0,104,1,4\n', 'MainMenu,Passed,0,81,0,4\n']))
        self.assertIn('**BESTÅTT**', self.text())
        self.assertIn('| 2 | 0 | 0 | 185 s |', self.text())

    def test_known_failure_does_not_fail_report(self):
        self.write('SoloPlay.err.txt', 'Missing screenshot: solo-home\n')
        self.assertEqual(0, self.build(['SoloPlay,Failed,1,50,0,4\n']))
        self.assertIn('#135', self.text())
        self.assertIn('Ingen nye feil', self.text())

    def test_new_failure_shows_first_marker_with_file_and_line(self):
        self.write('RigSetup/20261109/host.log', 'noise\nLogTemp: FISKELAGET_RIG_FAILED player=256 stage=9 fight_lost\nmore\n')
        self.assertEqual(1, self.build(['RigSetup,Failed,1,181,1,4\n']))
        text = self.text()
        self.assertIn('**FEILET: 1 ny(e) feil**', text)
        self.assertIn('`FISKELAGET_RIG_FAILED stage=9 fight_lost`', text)
        self.assertIn('RigSetup/20261109/host.log:2', text)
        self.assertIn('| NY |', text)

    def test_markers_are_capped_and_deduplicated(self):
        lines = ''.join(f'FISKELAGET_X_FAILED player={p} stage={s}\n' for p in (256, 257) for s in range(6))
        self.write('AnglerRelease/1/a.log', lines)
        self.build(['AnglerRelease,Failed,1,27,1,4\n'])
        self.assertEqual(report.MAX_LINES, self.text().count('FISKELAGET_X_FAILED'))

    def test_known_issue_in_any_script(self):
        self.write('MangroveRelease/1/solo.log', 'FISKELAGET_EXPLORATION_FAILED mode=full stage=0 atlas_not_open\n')
        self.assertEqual(0, self.build(['MangroveRelease,Failed,1,37,1,4\n']))
        self.assertIn('#136', self.text())

    def test_known_issue_limited_to_its_scripts(self):
        self.write('MainMenu.err.txt', 'Missing screenshot: solo-home\n')
        self.assertEqual(1, self.build(['MainMenu,Failed,1,79,0,4\n']))

    def test_failure_without_any_line_is_new(self):
        self.assertEqual(1, self.build(['FullDay,Failed,1,10,1,4\n']))
        self.assertIn('ingen feillinje funnet', self.text())

    def test_progress_clixml_is_not_an_error_line(self):
        self.write('FullDay.err.txt', '#< CLIXML\n<Objs><Obj S="progress"/></Objs>\n')
        self.build(['FullDay,Failed,1,10,1,4\n'])
        self.assertNotIn('CLIXML', self.text())

    def test_cached_scripts_are_listed(self):
        self.build(['FishCombat,Cached,0,0,0,4\n', 'SeaChart,Passed,0,475,1,1\n'])
        self.assertIn('Ikke kjørt på nytt (bestod før med samme bygg): FishCombat.', self.text())
        self.assertIn('klokkefaktor 1, 4', self.text())

    def test_personal_paths_are_redacted(self):
        home = 'C:' + '\\'.join(['', 'Users', 'ola', 'AppData', 'x.png'])  # Built here so the repository check sees no real path
        self.write('SoloPlay.err.txt', f'Missing {home}\n')
        self.build(['SoloPlay,Failed,1,50,0,4\n'])
        self.assertNotIn('ola', self.text())
        self.assertIn('%USERPROFILE%', self.text())

    def test_automation_failures(self):
        index = self.root / 'index.json'
        index.write_text(json.dumps({'succeeded': 170, 'succeededWithWarnings': 3, 'failed': 1, 'notRun': 0, 'tests': [
            {'fullTestPath': 'Fiskelaget.Fishing.RodStickInputs', 'state': 'Fail',
             'entries': [{'event': {'type': 'Error', 'message': 'Moving up lifts'}}]},
            {'fullTestPath': 'Fiskelaget.Menu.Navigation', 'state': 'Success', 'entries': []}]}), encoding='utf-8')
        code = report.main(['--automation', str(index), '--known', str(self.known), '--commit', 'b' * 40, '--out', str(self.root / 'report.md')])
        self.assertEqual(1, code)
        self.assertIn('| Automasjon | 173 | 1 | 0 | |', self.text())
        self.assertIn('Fiskelaget.Fishing.RodStickInputs', self.text())

    def test_exe_hash_in_header(self):
        exe = self.root / 'FiskELaget.exe'
        exe.write_bytes(b'abc')
        self.write('summary.csv', SUMMARY)
        report.main(['--run', str(self.run), '--exe', str(exe), '--known', str(self.known), '--commit', 'c' * 40, '--out', str(self.root / 'r.md')])
        self.assertIn('BA7816BF8F01CFEA414140DE5DAE2223B00361A396177A9CB410FF61F20015AD', (self.root / 'r.md').read_text(encoding='utf-8'))

    def test_invalid_known_file_fails_closed(self):
        self.known.write_text(json.dumps({'known': [{'issue': 'x', 'match': 'a'}]}), encoding='utf-8')
        with self.assertRaises(ValueError):
            self.build(['FishCombat,Passed,0,1,0,4\n'])

    def test_repository_known_failures_are_valid(self):
        entries = report.load_known(report.KNOWN)
        self.assertTrue(entries)
        self.assertTrue(all(isinstance(e['issue'], int) for e in entries))


if __name__ == '__main__':
    unittest.main()
