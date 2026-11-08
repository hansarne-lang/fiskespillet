"""Include boundaries between FiskGame areas: allowed, forbidden and stale exceptions."""
import json
import tempfile
import unittest
from pathlib import Path

import check_includes as ci


class IncludeBoundaries(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.root = Path(self.temp.name)
        for area in ('Fishing', 'Boats', 'Bootstrap'):
            (self.root / ci.PRIVATE / area).mkdir(parents=True)
        (self.root / ci.PUBLIC).mkdir(parents=True)
        (self.root / 'Tools/Validation').mkdir(parents=True)
        self.write('Source/FiskGame/Private/Boats/FiskBoat.h', '#pragma once\n')
        self.write('Source/FiskGame/Private/Fishing/FiskRod.h', '#pragma once\n')

    def tearDown(self):
        self.temp.cleanup()

    def write(self, relative, text):
        path = self.root / relative
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(text, encoding='utf-8')

    def exceptions(self, data):
        ci.write_exceptions(self.root, data)

    def test_allowed_includes(self):
        self.write('Source/FiskGame/Private/Fishing/FiskRod.cpp',
                   '#include "Fishing/FiskRod.h"\n#include "FiskRod.h"\n#include "FiskCareerProfile.h"\n'
                   '#include "FiskFishingGround.h"\n#include "Engine/World.h"\n#include "Misc/Parse.h"\n')
        self.write('Source/FiskGame/Private/Bootstrap/FiskMode.cpp', '#include "Boats/FiskBoat.h"\n#include "Fishing/FiskRod.h"\n')
        self.write('Source/FiskGame/Private/FiskGameModule.cpp', '#include "Boats/FiskBoat.h"\n')
        self.assertEqual([], ci.validate(self.root))

    def test_new_cross_area_include_fails(self):
        self.write('Source/FiskGame/Private/Fishing/FiskRod.cpp', '#include "Boats/FiskBoat.h"\n')
        errors = ci.validate(self.root)
        self.assertEqual(1, len(errors))
        self.assertIn('Fishing/FiskRod.cpp includes "Boats/FiskBoat.h"', errors[0])

    def test_feature_area_cannot_include_integration_layer(self):
        self.write('Source/FiskGame/Private/Fishing/FiskRod.cpp', '#include "Bootstrap/FiskMode.h"\n')
        self.assertEqual(1, len(ci.validate(self.root)))

    def test_public_header_cannot_include_private_area(self):
        self.write('Source/FiskGame/Public/FiskShared.h', '#include "Boats/FiskBoat.h"\n')
        self.assertEqual(1, len(ci.validate(self.root)))

    def test_listed_exception_passes(self):
        self.write('Source/FiskGame/Private/Fishing/FiskRod.cpp', '#include "Boats/FiskBoat.h"\n')
        self.exceptions({'Source/FiskGame/Private/Fishing/FiskRod.cpp': ['Boats/FiskBoat.h']})
        self.assertEqual([], ci.validate(self.root))

    def test_exception_does_not_cover_other_file_or_header(self):
        self.write('Source/FiskGame/Private/Fishing/FiskRod.cpp', '#include "Boats/FiskBoat.h"\n')
        self.write('Source/FiskGame/Private/Fishing/FiskReel.cpp', '#include "Boats/FiskBoat.h"\n')
        self.exceptions({'Source/FiskGame/Private/Fishing/FiskRod.cpp': ['Boats/FiskBoat.h']})
        errors = ci.validate(self.root)
        self.assertEqual(1, len(errors))
        self.assertIn('FiskReel.cpp', errors[0])

    def test_stale_exception_fails_and_prune_removes_only_it(self):
        self.write('Source/FiskGame/Private/Fishing/FiskRod.cpp', '#include "Boats/FiskBoat.h"\n')
        self.exceptions({'Source/FiskGame/Private/Fishing/FiskRod.cpp': ['Boats/FiskBoat.h'],
                         'Source/FiskGame/Private/Fishing/Gone.cpp': ['Boats/FiskBoat.h']})
        errors = ci.validate(self.root)
        self.assertEqual(1, len(errors))
        self.assertIn('Stale include exception', errors[0])
        self.assertEqual(1, ci.prune(self.root))
        self.assertEqual([], ci.validate(self.root))
        self.assertIn('Source/FiskGame/Private/Fishing/FiskRod.cpp', ci.load_exceptions(self.root))

    def test_prune_never_adds_new_violations(self):
        self.write('Source/FiskGame/Private/Fishing/FiskRod.cpp', '#include "Boats/FiskBoat.h"\n')
        self.exceptions({})
        self.assertEqual(0, ci.prune(self.root))
        self.assertEqual(1, len(ci.validate(self.root)))

    def test_commented_include_is_ignored(self):
        self.write('Source/FiskGame/Private/Fishing/FiskRod.cpp',
                   '// #include "Boats/FiskBoat.h"\n/* #include "Boats/FiskBoat.h" */\n')
        self.assertEqual([], ci.validate(self.root))

    def test_corrupt_exception_file_fails_closed(self):
        (self.root / ci.EXCEPTIONS).write_text('{not json', encoding='utf-8')
        self.assertIn('Unreadable', ci.validate(self.root)[0])

    def test_repository_is_clean(self):
        root = Path(__file__).resolve().parents[2]
        self.assertEqual([], ci.validate(root))
        data = json.loads((root / ci.EXCEPTIONS).read_text(encoding='utf-8'))
        self.assertEqual(sorted(data['exceptions']), list(data['exceptions']))


if __name__ == '__main__':
    unittest.main()
