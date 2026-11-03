"""Guard against corrupt project setup and reverse module dependencies."""
import json
from pathlib import Path
import shutil
import tempfile
import unittest
from check_bootstrap import validate


class BootstrapStructure(unittest.TestCase):
    def setUp(self):
        self.temp=tempfile.TemporaryDirectory()
        self.root=Path(self.temp.name)
        source=Path(__file__).resolve().parents[2]
        for name in ['FiskELaget.uproject','Source','Config']:
            if (source/name).is_dir():shutil.copytree(source/name,self.root/name)
            else:shutil.copyfile(source/name,self.root/name)

    def tearDown(self):
        self.temp.cleanup()

    def test_baseline_configuration(self):
        self.assertEqual([],validate(self.root))

    def test_duplicate_module_rejected(self):
        p=self.root/'FiskELaget.uproject'
        data=json.loads(p.read_text(encoding='utf-8'));data['Modules'].append(data['Modules'][0])
        p.write_text(json.dumps(data),encoding='utf-8')
        self.assertIn('Expected exactly FiskCore and FiskGame',validate(self.root))

    def test_core_cannot_depend_on_game_or_engine(self):
        p=self.root/'Source/FiskCore/FiskCore.Build.cs'
        p.write_text(p.read_text(encoding='utf-8')+'\nPrivateDependencyModuleNames.Add("Engine");',encoding='utf-8')
        self.assertIn('Invalid dependency direction/set: FiskCore',validate(self.root))

    def test_game_target_cannot_be_editor_target(self):
        p=self.root/'Source/FiskELaget.Target.cs'
        p.write_text(p.read_text(encoding='utf-8').replace('TargetType.Game','TargetType.Editor'),encoding='utf-8')
        self.assertTrue(any('Type = TargetType.Game' in e for e in validate(self.root)))

    def test_missing_source_or_wrong_startup_mode_rejected(self):
        (self.root/'Source/FiskCore/Private/FiskCoreModule.cpp').unlink()
        self.assertTrue(any('Missing source:' in e for e in validate(self.root)))
        p=self.root/'Config/DefaultEngine.ini'
        p.write_text(p.read_text(encoding='utf-8').replace('/Script/FiskGame.FiskBootstrapGameMode','/Script/Engine.GameModeBase'),encoding='utf-8')
        self.assertIn('Missing bootstrap GameMode configuration',validate(self.root))


if __name__=='__main__':unittest.main()
