"""Batch 3: exact content membership, immutable recipes and visual lifecycle."""
import hashlib
import json
from pathlib import Path
import re
import unittest
from TestPlayerArtFinal40 import FINAL12, CHANGED5, expected_status
from PIL import Image
from SharedPortraitImportCatalog import (
    load_catalog, is_canonical, expand_runtime_entries, validate_generated_source,
    master_path, runtime_derivative_path, runtime_size, hand_composition, pitch_composition,
)
from TestCanonicalArtBatch1 import PILOTS, SOURCES as BATCH1
from TestCanonicalArtBatch2 import SOURCES as BATCH2, COMPLETION

ROOT = Path(__file__).resolve().parent.parent
KEYS = (
    'Prototype.Arsenal.ViktorGyokeres', 'Prototype.ManchesterCity.RayanAitNouri',
    'Prototype.Arsenal.KaiHavertz', 'Prototype.ManchesterCity.MarcGuehi',
    'Prototype.Arsenal.EberechiEze', 'Prototype.ManchesterCity.OmarMarmoush',
    'Prototype.Arsenal.MartinZubimendi', 'Prototype.ManchesterCity.TijjaniReijnders',
)
FROZEN = PILOTS | set(BATCH1) | set(BATCH2) | COMPLETION

class CanonicalArtBatch3Test(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.catalog = load_catalog(ROOT)
        cls.selected = [e for e in cls.catalog if e.get('migrationStage') == '8.4']
        cls.provenance = json.loads((ROOT/'ContentSource/UI/PlayerPortraitRuntime/PlayerArtProvenance.json').read_text(encoding='utf-8'))['entries']

    def test_exact_membership_and_lifecycle(self):
        self.assertEqual({e['playerKey'] for e in self.selected}, set(KEYS))
        expected = FROZEN | set(KEYS) | FINAL12
        self.assertEqual(len(expected), 40)
        self.assertEqual({e['playerKey'] for e in self.catalog if is_canonical(e)}, expected)
        code = (ROOT/'Source/FMCodex/LocalPlay/FMCodexPlayerUIAssetReferences.cpp').read_text(encoding='utf-8')
        block = code.split('static const TSet<FName> CanonicalPlayers = {', 1)[1].split('};', 1)[0]
        self.assertEqual(set(re.findall(r'TEXT\("([^"]+)"\)', block)), expected)
        self.assertEqual({e['playerKey'] for e in self.provenance}, expected)
        for e in self.provenance:
            for role in ('Hand', 'Shared', 'Full'):
                status = expected_status(e['playerKey'], role)
                self.assertEqual(e['roleStatus'][role], status)
                self.assertEqual(e['roles'][role]['visualStatus'], status)
        self.assertIn('Prototype.ManchesterCity.JohnStones', expected)
        self.assertNotIn('Test.FuturePlayer', expected)

    def test_master_provenance_and_three_purposes(self):
        generation = json.loads((ROOT/'ArtSource/UI/PlayerMaster/Stage8_4_Generation.json').read_text(encoding='utf-8'))['entries']
        self.assertEqual(tuple(e['playerKey'] for e in generation), KEYS)
        records = {e['playerKey']:e for e in generation}
        digests = set()
        for e in self.selected:
            key = e['playerKey']
            with self.subTest(player=key):
                digest = hashlib.sha256(master_path(ROOT,e).read_bytes()).hexdigest().upper()
                digests.add(digest)
                self.assertEqual(e['masterSha256'], digest)
                self.assertEqual(records[key]['masterSha256'], digest)
                self.assertEqual(e['sourceProvenance']['originIdentity'], key)
                self.assertEqual(e['canonicalVisualStatus'], expected_status(e['playerKey'], 'Hand'))
                self.assertEqual(records[key]['runtimeVisualStatus'], 'USER PIE ACCEPTED')
                self.assertEqual(e['familyRevision'], '1.3')
                self.assertEqual(e['handCompositionProfile'], 'BalancedBust_v3')
                self.assertEqual(pitch_composition(e), 'QuietPitchBust_v3')
                if records[key]['route'] != 'accepted legacy source':
                    self.assertTrue(records[key]['prompt'])
                with Image.open(master_path(ROOT,e)) as im:
                    self.assertEqual(im.size,(1024,1536)); self.assertEqual(im.mode,'RGB')
                self.assertEqual(records[key]['sourceGate'], e['sourceGate'])
        self.assertEqual(len(digests),8)
        self.assertFalse(digests & {e['masterSha256'] for e in self.provenance if e['playerKey'] in FROZEN})
        for e in expand_runtime_entries(self.selected, ('Hand','Shared','Full')):
            with self.subTest(player=e['playerKey'],role=e['runtimeRole']):
                r = validate_generated_source(ROOT,e)
                self.assertEqual(r['visualStatus'],expected_status(e['playerKey'],e['runtimeRole']))
                self.assertEqual(r['importRecipe'],'DesktopBC7OpaqueSharpen1_v1')
                with Image.open(runtime_derivative_path(ROOT,e)) as im:
                    self.assertEqual(im.size,runtime_size(e)); self.assertEqual(im.mode,'RGB')
                self.assertIn('/Canonical/'+e['playerKey'].replace('.','_')+'/',r['runtimeAssetPath'])
                package = ROOT/('Content/'+r['runtimeAssetPath'].split('.')[0].removeprefix('/Game/')+'.uasset')
                self.assertTrue(package.is_file(), package)

if __name__ == '__main__':
    unittest.main(verbosity=2)
