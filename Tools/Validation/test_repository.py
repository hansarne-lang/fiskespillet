"""Regression tests exercise failed contracts and publication mistakes."""
from pathlib import Path
import json
import shutil
import tempfile
import unittest
from check_repository import validate

class RepositoryChecks(unittest.TestCase):
    def setUp(self):
        self.temp=tempfile.TemporaryDirectory()
        self.root=Path(self.temp.name)/'repo'
        source=Path(__file__).resolve().parents[2]
        shutil.copytree(source,self.root,ignore=shutil.ignore_patterns('.git','__pycache__'))
    def tearDown(self): self.temp.cleanup()
    def test_baseline(self): self.assertEqual([],validate(self.root))
    def test_missing_event(self):
        p=self.root/'MODULE_CONTRACTS.md';p.write_text(p.read_text(encoding='utf-8').replace('### CatchSold','### RemovedEvent'),encoding='utf-8')
        self.assertTrue(any('Missing event: CatchSold' in e for e in validate(self.root)))
    def test_missing_authority(self):
        p=self.root/'MODULE_CONTRACTS.md';p.write_text(p.read_text(encoding='utf-8').replace('Autoritet/feil:','Rule:',1),encoding='utf-8')
        self.assertTrue(any('missing Autoritet/feil' in e for e in validate(self.root)))
    def test_invalid_status(self):
        p=self.root/'.github/project-blueprint.json';data=json.loads(p.read_text(encoding='utf-8'));data['fields']['Status'].append('Almost Done');p.write_text(json.dumps(data),encoding='utf-8')
        self.assertIn('Invalid task status contract',validate(self.root))
    def test_broken_link(self):
        p=self.root/'README.md';p.write_text(p.read_text(encoding='utf-8')+'\n[bad](missing-document.md)\n',encoding='utf-8')
        self.assertTrue(any('broken local link' in e for e in validate(self.root)))
    def test_secret_file(self):
        (self.root/'.env').write_text('EXAMPLE=value',encoding='utf-8')
        self.assertIn('Sensitive/local file: .env',validate(self.root))
    def test_missing_lfs_lock(self):
        p=self.root/'.gitattributes';p.write_text(p.read_text(encoding='utf-8').replace(' lockable',''),encoding='utf-8')
        self.assertIn('Missing lockable LFS rule: umap',validate(self.root))
    def test_unsafe_done(self):
        p=self.root/'.github/project-blueprint.json';data=json.loads(p.read_text(encoding='utf-8'));data['automatic_closed_issue_to_done']=True;p.write_text(json.dumps(data),encoding='utf-8')
        self.assertIn('Unsafe automatic Done transition',validate(self.root))

if __name__=='__main__':unittest.main()
