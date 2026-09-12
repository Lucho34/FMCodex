"""Batch 1: explicit fictional identity, source integrity and three-purpose pipeline."""
import copy
import hashlib
import json
from pathlib import Path
import re
import unittest
from unittest.mock import patch
from PIL import Image
from GenerateSharedPortraitRuntimeDerivatives import encode_runtime_derivative
from SharedPortraitImportCatalog import (load_catalog, is_canonical, master_path,
    runtime_derivative_path, runtime_size, expand_runtime_entries, resolved_crop,
    hand_composition, pitch_composition, validate_generated_source, select_entries)

ROOT = Path(__file__).resolve().parent.parent
SOURCES = {
    'Prototype.ManchesterCity.GianluigiDonnarumma': 'FullCardPilot_02',
    'Prototype.Arsenal.GabrielMagalhaes': '01',
    'Prototype.Arsenal.MylesLewisSkelly': '01',
    'Prototype.Arsenal.RiccardoCalafiori': '01',
    'Prototype.ManchesterCity.JoskoGvardiol': '01',
    'Prototype.ManchesterCity.JeremyDoku': '01',
    'Prototype.Arsenal.GabrielMartinelli': '01',
}
MISSING = 'Prototype.ManchesterCity.NathanAke'
PILOTS = {'Prototype.Arsenal.BukayoSaka', 'Prototype.Arsenal.DavidRaya',
    'Prototype.ManchesterCity.Rodri', 'Prototype.ManchesterCity.ErlingHaaland'}


class CanonicalArtBatch1Test(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.catalog = load_catalog(ROOT)
        cls.players = [e for e in cls.catalog if e.get('migrationStage') == '8.3A']
        cls.roster = {p['playerKey']: p for p in json.loads(
            (ROOT/'Content/Data/CanonicalPlayerContent.json').read_text(encoding='utf-8'))['players']}

    def test_exact_batch_source_identity_and_master_integrity(self):
        self.assertEqual({e['playerKey'] for e in self.players}, set(SOURCES))
        self.assertEqual({e['playerKey'] for e in self.catalog if is_canonical(e)}, PILOTS | set(SOURCES))
        hashes = set()
        for e in self.players:
            with self.subTest(player=e['playerKey']):
                key = e['playerKey']; team = key.split('.')[1]
                self.assertEqual(self.roster[key]['team'].replace(' ', ''), team)
                expected = f'ArtSource/UI/PrototypeTeams/{team}/Portraits/T_{key.replace(".","_")}_{SOURCES[key]}.png'
                self.assertEqual(e['sourceProvenance']['sourcePath'], expected)
                self.assertEqual(e['sourceProvenance']['originIdentity'], key)
                self.assertIn(e['sourceGate']['classification'], ('SOURCE_READY','SOURCE_USABLE_WITH_METADATA_CROP'))
                source = (ROOT/expected).read_bytes(); master = master_path(ROOT,e).read_bytes()
                self.assertEqual(source, master)
                digest = hashlib.sha256(master).hexdigest().upper(); hashes.add(digest)
                self.assertEqual(e['masterSha256'], digest)
                self.assertEqual(e['sourceProvenance']['sourceSha256'], digest)
                self.assertEqual(e['masterRevision'], 1)
                with Image.open(master_path(ROOT,e)) as im:
                    self.assertEqual(im.size,(1024,1536)); self.assertEqual(im.mode,'RGB')
        self.assertEqual(len(hashes),7, 'No cross-player Master reuse')
        self.assertFalse((ROOT/'ArtSource/UI/PlayerMaster'/MISSING/'Master.png').exists())

    def test_all_roles_reproduce_directly_from_same_master(self):
        for e in expand_runtime_entries(self.players,('Hand','Shared','Full')):
            with self.subTest(player=e['playerKey'], role=e['runtimeRole']):
                record = validate_generated_source(ROOT,e)
                role = e['runtimeRole']
                composition = hand_composition(e) if role=='Hand' else pitch_composition(e) if role=='Shared' else 'CropOnly_v1'
                data = encode_runtime_derivative(master_path(ROOT,e),runtime_size(e),resolved_crop(e),composition)
                self.assertEqual(data, runtime_derivative_path(ROOT,e).read_bytes())
                self.assertEqual(record['visualStatus'],'USER PIE ACCEPTED')
                self.assertEqual(record['importRecipe'],'DesktopBC7OpaqueSharpen1_v1')
                with Image.open(runtime_derivative_path(ROOT,e)) as im:
                    self.assertEqual(im.size,runtime_size(e));self.assertEqual(im.mode,'RGB')
                self.assertIn('/Canonical/'+e['playerKey'].replace('.','_')+'/',record['runtimeAssetPath'])

    def test_explicit_enablement_and_source_missing_exclusion(self):
        source=(ROOT/'Source/FMCodex/LocalPlay/FMCodexPlayerUIAssetReferences.cpp').read_text(encoding='utf-8')
        block=source.split('static const TSet<FName> CanonicalPlayers = {',1)[1].split('};',1)[0]
        self.assertEqual(set(re.findall(r'TEXT\("([^\"]+)"\)',block)),PILOTS|set(SOURCES))
        with patch.dict('os.environ', {'FMCODEX_SHARED_PORTRAIT_PLAYER_KEYS':MISSING}):
            with self.assertRaisesRegex(RuntimeError,'Unknown Shared Portrait PlayerKey'):select_entries(self.catalog)
        with patch.dict('os.environ', {'FMCODEX_SHARED_PORTRAIT_PLAYER_KEYS':next(iter(SOURCES))}):
            self.assertEqual(len(select_entries(self.catalog)),1)
        self.assertFalse(next(e for e in self.catalog if e['playerKey']=='Prototype.Arsenal.MikelMerino').get('masterSourcePath'))

    def test_missing_tampered_master_output_and_role_binding_reject(self):
        entry = dict(self.players[0],runtimeRole='Shared'); original=Path.read_bytes
        for target in (master_path(ROOT,entry),runtime_derivative_path(ROOT,entry)):
            with self.subTest(path=str(target)):
                def tampered(p):return b'tampered' if p==target else original(p)
                with patch.object(Path,'read_bytes',tampered):
                    with self.assertRaises(RuntimeError):validate_generated_source(ROOT,entry)
                def missing(p):
                    if p==target:raise FileNotFoundError(str(p))
                    return original(p)
                with patch.object(Path,'read_bytes',missing):
                    with self.assertRaises(FileNotFoundError):validate_generated_source(ROOT,entry)
        path=ROOT/'ContentSource/UI/PlayerPortraitRuntime/PlayerArtProvenance.json'
        document=json.loads(path.read_text(encoding='utf-8')); old_read=Path.read_text
        record=next(e for e in document['entries'] if e['playerKey']==entry['playerKey'])
        record['roles']['Shared']['runtimeAssetPath']=record['roles']['Full']['runtimeAssetPath']
        def wrong_role(p,*args,**kwargs):return json.dumps(document) if p==path else old_read(p,*args,**kwargs)
        with patch.object(Path,'read_text',wrong_role):
            with self.assertRaisesRegex(RuntimeError,'Incorrect derived role binding'):validate_generated_source(ROOT,entry)
        bad=copy.deepcopy(entry);bad['cropOverrides']['pitchCropRect']=[0,.06,1,.55]
        with self.assertRaisesRegex(RuntimeError,'Stale crop'):validate_generated_source(ROOT,bad)


if __name__=='__main__':unittest.main(verbosity=2)
