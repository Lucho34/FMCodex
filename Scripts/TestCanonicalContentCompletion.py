"""Stage 8.3E authored bio and two new canonical source families."""
import hashlib
import json
from pathlib import Path
import re
import unittest
from urllib.parse import urlparse

from PIL import Image
from GenerateSharedPortraitRuntimeDerivatives import encode_runtime_derivative
from SharedPortraitImportCatalog import (
    load_catalog, expand_runtime_entries, validate_generated_source,
    master_path, runtime_derivative_path, runtime_size, resolved_crop,
    hand_composition, pitch_composition,
)

ROOT = Path(__file__).resolve().parent.parent
NEW = {'Prototype.ManchesterCity.NathanAke', 'Prototype.ManchesterCity.RayanCherki'}
BIO = ('birthDate', 'heightCm', 'weightKg', 'nationality')

def read(path):
    return json.loads((ROOT/path).read_text(encoding='utf-8-sig'))

class CanonicalContentCompletionTest(unittest.TestCase):
    def test_bio_provenance_matches_authored_and_generated_values(self):
        source = {p['playerKey']: p for p in read('ContentSource/PlayerContent/CanonicalPlayerImportConfig.json')['players']}
        runtime = {p['playerKey']: p for p in read('Content/Data/CanonicalPlayerContent.json')['players']}
        evidence = read('ContentSource/PlayerContent/PlayerBioProvenance.json')['players']
        self.assertEqual(len(evidence), 40)
        self.assertEqual({e['playerKey'] for e in evidence}, set(source))
        self.assertEqual(set(source), set(runtime))
        recovered = preserved = 0
        missing = []
        for entry in evidence:
            key = entry['playerKey']
            self.assertEqual(set(entry['fields']), set(BIO))
            for field, fact in entry['fields'].items():
                with self.subTest(player=key, field=field):
                    authored = source[key]['presentation'].get(field)
                    self.assertEqual(authored, fact['value'])
                    self.assertEqual(runtime[key]['presentation'][field], authored if authored is not None else (0 if field in ('heightCm','weightKg') else ''))
                    if fact['status'] == 'RECOVERED':
                        recovered += 1
                        self.assertTrue(fact['value'])
                        self.assertEqual(urlparse(fact['sourceUrl']).scheme, 'https')
                        self.assertIn(fact['sourceClass'], ('official_club_archive','official_competition_profile','official_federation_profile','ESPN_fallback'))
                    elif fact['status'] == 'PRESERVED_AUTHORED_BASELINE':
                        preserved += 1
                        self.assertTrue(fact['repositorySource'])
                    else:
                        missing.append((key, field))
                        self.assertIsNone(fact['value'])
                        self.assertEqual(fact['status'], 'MISSING_UNRESOLVED_CONFLICT')
                        self.assertGreaterEqual(len(entry['conflictingSources']), 2)
        self.assertEqual((recovered, preserved), (96, 64))
        self.assertEqual(missing, [])
        for player in runtime.values():
            self.assertNotIn('sourceUrl', player['presentation'])
            self.assertNotIn('—', player['presentation'].values())

    def test_new_sources_have_distinct_identity_and_explicit_routes(self):
        catalog = load_catalog(ROOT)
        new = [e for e in catalog if e.get('migrationStage') == '8.3E']
        self.assertEqual({e['playerKey'] for e in new}, NEW)
        canonical = [e for e in catalog if e.get('masterSourcePath')]
        self.assertEqual(len(canonical), 28)
        self.assertEqual(len({e['masterSha256'] for e in canonical}), 28)
        records = {e['playerKey']:e for e in read('ArtSource/UI/PlayerMaster/Stage8_3E_Generation.json')['entries']}
        code = (ROOT/'Source/FMCodex/LocalPlay/FMCodexPlayerUIAssetReferences.cpp').read_text(encoding='utf-8')
        block = code.split('static const TSet<FName> CanonicalPlayers = {',1)[1].split('};',1)[0]
        self.assertEqual(set(re.findall(r'TEXT\("([^"]+)"\)', block)), {e['playerKey'] for e in canonical})
        for e in new:
            key = e['playerKey']
            self.assertEqual(e['team'], 'ManchesterCity')
            self.assertEqual(e['sourceProvenance']['originIdentity'], key)
            self.assertEqual(e['sourceProvenance']['sourcePath'], e['masterSourcePath'])
            self.assertEqual(records[key]['masterSha256'], e['masterSha256'])
            self.assertIn(key, records[key]['prompt'])
            self.assertEqual(hashlib.sha256(master_path(ROOT,e).read_bytes()).hexdigest().upper(), e['masterSha256'])
            with Image.open(master_path(ROOT,e)) as im:
                self.assertEqual((im.size,im.mode), ((1024,1536),'RGB'))
            expected = {'handCropRect':[0,.025,1,.53], 'pitchCropRect':[0,.025,1,.58]} if key.endswith('.NathanAke') else {}
            self.assertEqual(e.get('cropOverrides',{}), expected)
            self.assertEqual(e['canonicalVisualStatus'], 'USER PIE ACCEPTED')

    def test_closeout_weight_and_accepted_role_metadata(self):
        bio = read('ContentSource/PlayerContent/PlayerBioProvenance.json')
        item = next(p for p in bio['players'] if p['playerKey'].endswith('.RayanAitNouri'))
        weight = item['fields']['weightKg']
        self.assertEqual((weight['value'], weight['sourceTier'], weight['sourceFamily']), (70, 1, 'FFF'))
        self.assertEqual(weight['sourceUrl'], 'https://www.fff.fr/equipe-nationale/joueur/8926-ait-nouri-rayan/fiche.html')
        self.assertEqual(sum(e['verdict'] == 'SELECTED' for e in item['weightResearch']), 1)
        self.assertEqual(sum(e['sourceFamily'] == 'ESPN' for e in item['weightResearch']), 1)
        self.assertEqual(bio['closeout']['completeFieldsAfter'], 160)
        provenance = read('ContentSource/UI/PlayerPortraitRuntime/PlayerArtProvenance.json')['entries']
        self.assertEqual(len(provenance), 28)
        for p in provenance:
            self.assertEqual(set(p['roleStatus'].values()), {'USER PIE ACCEPTED'})
            self.assertEqual({v['visualStatus'] for v in p['roles'].values()}, {'USER PIE ACCEPTED'})
        generated = read('ArtSource/UI/PlayerMaster/Stage8_3E_Generation.json')['entries']
        self.assertEqual({p['runtimeVisualStatus'] for p in generated}, {'USER PIE ACCEPTED'})
        for e in load_catalog(ROOT):
            if e['playerKey'] in NEW:
                self.assertEqual(e['canonicalVisualStatus'], 'USER PIE ACCEPTED')
                self.assertEqual(e['pitchVisualStatus'], 'USER PIE ACCEPTED')
                for role in ('Hand', 'Shared', 'Full'):
                    record = validate_generated_source(ROOT, dict(e, runtimeRole=role))
                    self.assertEqual(record['visualStatus'], 'USER PIE ACCEPTED')

    def test_new_derivatives_reproduce_and_keep_role_isolation(self):
        players = [e for e in load_catalog(ROOT) if e['playerKey'] in NEW]
        for e in expand_runtime_entries(players, ('Hand','Shared','Full')):
            with self.subTest(player=e['playerKey'], role=e['runtimeRole']):
                record = validate_generated_source(ROOT,e)
                role = e['runtimeRole']
                profile = hand_composition(e) if role == 'Hand' else pitch_composition(e) if role == 'Shared' else 'CropOnly_v1'
                self.assertEqual(encode_runtime_derivative(master_path(ROOT,e),runtime_size(e),resolved_crop(e),profile),runtime_derivative_path(ROOT,e).read_bytes())
                self.assertEqual(record['visualStatus'], 'USER PIE ACCEPTED')
                self.assertTrue(record['runtimeAssetPath'].endswith('_'+role))
                self.assertEqual(record['importRecipe'], 'DesktopBC7OpaqueSharpen1_v1')

if __name__ == '__main__':
    unittest.main(verbosity=2)
