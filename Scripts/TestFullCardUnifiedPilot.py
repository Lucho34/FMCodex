"""Full activation reproduces the Master and preserves independently accepted purposes."""
import copy
import hashlib
import io
import json
import os
from pathlib import Path
import shutil
import tempfile
import unittest
from unittest.mock import patch
from PIL import Image
from GenerateSharedPortraitRuntimeDerivatives import encode_runtime_derivative, generate_canonical_selected
from SharedPortraitImportCatalog import (load_catalog, is_canonical, expand_runtime_entries,
    master_path, runtime_derivative_path, runtime_size, resolved_crop, validate_generated_source)

ROOT = Path(__file__).resolve().parent.parent

class FullCardUnifiedPilotTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.players = [e for e in load_catalog(ROOT) if is_canonical(e)]
        cls.records = json.loads((ROOT/'ContentSource/UI/PlayerPortraitRuntime/PlayerArtProvenance.json').read_text(encoding='utf-8'))['entries']

    def test_full_reproduces_directly_from_four_masters(self):
        self.assertEqual(len(self.players),4)
        for e in expand_runtime_entries(self.players,('Full',)):
            with self.subTest(player=e['playerKey']):
                record=validate_generated_source(ROOT,e)
                data=encode_runtime_derivative(master_path(ROOT,e),runtime_size(e),resolved_crop(e))
                self.assertEqual(data,runtime_derivative_path(ROOT,e).read_bytes())
                with Image.open(io.BytesIO(data)) as image:
                    self.assertEqual(image.size,(768,1152));self.assertEqual(image.mode,'RGB')
                self.assertEqual(record['visualStatus'],'PENDING USER PIE')
                self.assertEqual(record['cropRect'],[0,0,1,1])
                self.assertIn('/Canonical/',record['runtimeAssetPath'])

    def test_full_first_activation_preserves_existing_hand_and_pitch_records_and_bytes(self):
        with tempfile.TemporaryDirectory() as folder, patch.dict(os.environ,{'FMCODEX_PLAYER_ART_RUNTIME_ROLES':'Full'}):
            root=Path(folder);prior=[];protected={}
            for e in self.players:
                paths=[master_path(ROOT,e)]+[runtime_derivative_path(ROOT,dict(e,runtimeRole=r)) for r in ('Hand','Shared')]
                for path in paths:
                    dest=root/path.relative_to(ROOT);dest.parent.mkdir(parents=True,exist_ok=True)
                    shutil.copyfile(path,dest);protected[dest]=dest.read_bytes()
                record=copy.deepcopy(next(r for r in self.records if r['playerKey']==e['playerKey']))
                record['roles'].pop('Full');prior.append(record)
            generator=root/'Scripts/GenerateSharedPortraitRuntimeDerivatives.py';generator.parent.mkdir(parents=True)
            shutil.copyfile(ROOT/'Scripts/GenerateSharedPortraitRuntimeDerivatives.py',generator)
            provenance=root/'ContentSource/UI/PlayerPortraitRuntime/PlayerArtProvenance.json'
            provenance.write_text(json.dumps({'schemaVersion':2,'entries':prior}),encoding='utf-8')
            after=generate_canonical_selected(root,self.players)
            for old,new in zip(prior,after):
                for role in ('Hand','Shared'):self.assertEqual(old['roles'][role],new['roles'][role])
                self.assertEqual(set(new['roles']),{'Hand','Shared','Full'})
            for path,data in protected.items():self.assertEqual(path.read_bytes(),data)
            for e in expand_runtime_entries(self.players,('Full',)):
                self.assertEqual(runtime_derivative_path(root,e).read_bytes(),runtime_derivative_path(ROOT,e).read_bytes())
                validate_generated_source(root,e)

    def test_full_crop_master_and_output_tampering_block_import(self):
        e=dict(self.players[0],runtimeRole='Full')
        for rect in ([0,0,1,.5],[0,.2,1,1],[0,0,float('nan'),1]):
            bad=copy.deepcopy(e);bad.setdefault('cropOverrides',{})['fullCropRect']=rect
            with self.assertRaises(RuntimeError):resolved_crop(bad)
        bad=copy.deepcopy(e);bad['cropOverrides']['fullCropRect']=[0,0,.9,.9]
        with self.assertRaises(RuntimeError):validate_generated_source(ROOT,bad)
        original=Path.read_bytes
        for target in (master_path(ROOT,e),runtime_derivative_path(ROOT,e)):
            def tampered(path):return b'tampered' if path==target else original(path)
            with patch.object(Path,'read_bytes',tampered):
                with self.assertRaises(RuntimeError):validate_generated_source(ROOT,e)

if __name__=='__main__':unittest.main(verbosity=2)
