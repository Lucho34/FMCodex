"""Stage 8.3C: source-gated Batch 2 under frozen Player Card Family v1.1."""
import copy
import hashlib
import json
from pathlib import Path
import re
import unittest
from unittest.mock import patch

from PIL import Image
from GenerateSharedPortraitRuntimeDerivatives import encode_runtime_derivative
from SharedPortraitImportCatalog import (
    load_catalog, is_canonical, master_path, runtime_derivative_path,
    runtime_size, expand_runtime_entries, resolved_crop, hand_composition,
    pitch_composition, validate_generated_source, select_entries,
)
from TestCanonicalArtBatch1 import PILOTS, SOURCES as BATCH1

ROOT = Path(__file__).resolve().parent.parent
SOURCES = {
    'Prototype.Arsenal.WilliamSaliba': ('FullCardHeroBust_01', '0E8FA62FB66CD5D28888134F1F74312A72E2BBCA81C72A2F0C93C84005797524', 3),
    'Prototype.Arsenal.JurrienTimber': ('01', '8DE585EB55109BBC11AE4BF983242E9314399850EEEA1CFB345423DB71223812', 6),
    'Prototype.Arsenal.MartinOdegaard': ('FullCardHeroBust_01', '49652417914A8276C39E39E01B05578F1D10CBD498B844BC423784156DDA74F3', 10),
    'Prototype.Arsenal.DeclanRice': ('FullCardHeroBust_01', 'F2934D9874465B114FA02C0DC78948285B485B61484A5EB7EFF0DE5739E9871D', 14),
    'Prototype.ManchesterCity.RubenDias': ('FullCardHeroBust_01', 'A0B5135A6B6572B53E938C26A23163475B61F07F52E0A31D77CB17E9874F255F', 2),
    'Prototype.ManchesterCity.BernardoSilva': ('01', '2FB44F7DD391357B17C62654AA2DA0870DC1099E7902A148306F70B1E14909E2', 10),
    'Prototype.ManchesterCity.PhilFoden': ('FullCardHeroBust_01', '8A313131711792634B1AA19AE6DB5EA4DA7F9D11E259387D6C449D27EA089D61', 11),
}
MISSING = 'Prototype.ManchesterCity.JohnStones'
OTHER_MISSING = 'Prototype.ManchesterCity.NicoGonzalez'
COMPLETION = {'Prototype.ManchesterCity.NathanAke', 'Prototype.ManchesterCity.RayanCherki'}
TIMBER = 'Prototype.Arsenal.JurrienTimber'


class CanonicalArtBatch2Test(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.catalog = load_catalog(ROOT)
        cls.players = [e for e in cls.catalog if e.get('migrationStage') == '8.3C']
        cls.roster = {p['playerKey']: p for p in json.loads(
            (ROOT/'Content/Data/CanonicalPlayerContent.json').read_text(encoding='utf-8'))['players']}

    def test_exact_keys_sources_masters_and_explicit_enablement(self):
        self.assertEqual({e['playerKey'] for e in self.players}, set(SOURCES))
        expected = PILOTS | set(BATCH1) | set(SOURCES) | COMPLETION | {e['playerKey'] for e in self.catalog if e.get('migrationStage') == '8.4'}
        self.assertEqual({e['playerKey'] for e in self.catalog if is_canonical(e)}, expected)
        code = (ROOT/'Source/FMCodex/LocalPlay/FMCodexPlayerUIAssetReferences.cpp').read_text(encoding='utf-8')
        block = code.split('static const TSet<FName> CanonicalPlayers = {', 1)[1].split('};', 1)[0]
        self.assertEqual(set(re.findall(r'TEXT\("([^"]+)"\)', block)), expected)
        digests = set()
        for e in self.players:
            key = e['playerKey']; suffix, digest, number = SOURCES[key]
            with self.subTest(key=key):
                path = f'ArtSource/UI/PrototypeTeams/{key.split(".")[1]}/Portraits/T_{key.replace(".", "_")}_{suffix}.png'
                self.assertEqual(e['sourceProvenance']['sourcePath'], path)
                self.assertEqual(e['sourceProvenance']['originIdentity'], key)
                self.assertEqual(e['team'], self.roster[key]['team'].replace(' ', ''))
                data = (ROOT/path).read_bytes()
                self.assertEqual(master_path(ROOT, e).read_bytes(), data)
                self.assertEqual(hashlib.sha256(data).hexdigest().upper(), digest)
                self.assertEqual(e['masterSha256'], digest)
                self.assertEqual(e['sourceProvenance']['sourceSha256'], digest)
                self.assertEqual(e['masterRevision'], 1)
                self.assertEqual(e['familyRevision'], '1.2')
                self.assertEqual(e['sourceGate']['classification'], 'SOURCE_USABLE_WITH_METADATA_CROP' if key == TIMBER else 'SOURCE_READY')
                self.assertEqual(self.roster[key]['presentation']['defaultShirtNumber'], number)
                with Image.open(master_path(ROOT, e)) as im:
                    self.assertEqual(im.size, (1024, 1536)); self.assertEqual(im.mode, 'RGB')
                digests.add(digest)
        self.assertEqual(len(digests), 7, 'No cross-player source reuse')

    def test_three_roles_reproduce_with_frozen_profiles_and_bounded_metadata(self):
        for e in expand_runtime_entries(self.players, ('Hand', 'Shared', 'Full')):
            role = e['runtimeRole']; key = e['playerKey']
            with self.subTest(key=key, role=role):
                record = validate_generated_source(ROOT, e)
                profile = hand_composition(e) if role == 'Hand' else pitch_composition(e) if role == 'Shared' else 'CropOnly_v1'
                self.assertEqual(profile, {'Hand':'BalancedBust_v3', 'Shared':'QuietPitchBust_v3', 'Full':'CropOnly_v1'}[role])
                top = .115 if key == TIMBER else .055
                expected_crop = [0, top, 1, .5 if role == 'Hand' else .55] if role != 'Full' else [0, 0, 1, 1]
                self.assertEqual(resolved_crop(e), expected_crop)
                self.assertEqual(e.get('cropOverrides', {}), {'handCropRect':[0,.115,1,.5], 'pitchCropRect':[0,.115,1,.55]} if key == TIMBER else {})
                if key == TIMBER: self.assertTrue(e['cropExceptionReason'])
                data = encode_runtime_derivative(master_path(ROOT, e), runtime_size(e), resolved_crop(e), profile)
                self.assertEqual(data, runtime_derivative_path(ROOT, e).read_bytes())
                self.assertEqual(record['visualStatus'], 'USER PIE ACCEPTED')
                self.assertEqual(record['importRecipe'], 'DesktopBC7OpaqueSharpen1_v1')
                with Image.open(runtime_derivative_path(ROOT, e)) as im:
                    self.assertEqual(im.size, runtime_size(e)); self.assertEqual(im.mode, 'RGB')
        provenance = json.loads((ROOT/'ContentSource/UI/PlayerPortraitRuntime/PlayerArtProvenance.json').read_text(encoding='utf-8'))
        for p in provenance['entries']:
            for role in ('Hand', 'Shared', 'Full'):
                expected = 'USER PIE ACCEPTED'
                self.assertEqual(p['roleStatus'][role], expected)
                self.assertEqual(p['roles'][role]['visualStatus'], expected)

    def test_missing_sources_stay_excluded_and_nonmigrated_legacy_survives(self):
        for key in (MISSING, OTHER_MISSING):
            self.assertIn(key, self.roster)
            self.assertFalse((ROOT/'ArtSource/UI/PlayerMaster'/key).exists())
            self.assertFalse((ROOT/'ContentSource/UI/PlayerPortraitRuntime'/key).exists())
            self.assertFalse((ROOT/'Content/UI/Portraits/PrototypeTeams/Canonical'/key.replace('.', '_')).exists())
            with patch.dict('os.environ', {'FMCODEX_SHARED_PORTRAIT_PLAYER_KEYS':key}):
                with self.assertRaisesRegex(RuntimeError, 'Unknown Shared Portrait PlayerKey'): select_entries(self.catalog)
        legacy = next(e for e in self.catalog if e['playerKey'] == 'Prototype.Arsenal.MikelMerino')
        self.assertFalse(is_canonical(legacy))
        self.assertEqual(self.roster['Prototype.ManchesterCity.RayanCherki']['presentation']['defaultShirtNumber'], 12)
        # Stage 8.3E filled real biographies independently of art migration.
        for key in (TIMBER, MISSING):
            for field in ('birthDate', 'heightCm', 'weightKg', 'nationality'):
                self.assertTrue(self.roster[key]['presentation'][field], 'Researched biography reaches generated data')

    def test_missing_tampered_master_output_crop_and_role_reject(self):
        entry = dict(self.players[0], runtimeRole='Shared')
        read_bytes = Path.read_bytes
        for target in (master_path(ROOT, entry), runtime_derivative_path(ROOT, entry)):
            with self.subTest(path=str(target)):
                def tampered(p): return b'tampered' if p == target else read_bytes(p)
                with patch.object(Path, 'read_bytes', tampered):
                    with self.assertRaises(RuntimeError): validate_generated_source(ROOT, entry)
                def missing(p):
                    if p == target: raise FileNotFoundError(str(p))
                    return read_bytes(p)
                with patch.object(Path, 'read_bytes', missing):
                    with self.assertRaises(FileNotFoundError): validate_generated_source(ROOT, entry)
        bad = copy.deepcopy(entry); bad['cropOverrides'] = {'pitchCropRect':[0,.07,1,.55]}
        with self.assertRaisesRegex(RuntimeError, 'Stale crop'): validate_generated_source(ROOT, bad)
        path = ROOT/'ContentSource/UI/PlayerPortraitRuntime/PlayerArtProvenance.json'
        document = json.loads(path.read_text(encoding='utf-8')); read_text = Path.read_text
        record = next(e for e in document['entries'] if e['playerKey'] == entry['playerKey'])
        record['roles']['Shared']['runtimeAssetPath'] = record['roles']['Full']['runtimeAssetPath']
        def wrong_role(p, *args, **kwargs): return json.dumps(document) if p == path else read_text(p, *args, **kwargs)
        with patch.object(Path, 'read_text', wrong_role):
            with self.assertRaisesRegex(RuntimeError, 'Incorrect derived role binding'): validate_generated_source(ROOT, entry)


if __name__ == '__main__': unittest.main(verbosity=2)
