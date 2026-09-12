"""Four-player Shared/Pitch composition, purpose isolation and provenance gates."""
import copy, hashlib, json, os, shutil, tempfile, unittest
from pathlib import Path
from unittest.mock import patch
from GenerateSharedPortraitRuntimeDerivatives import encode_runtime_derivative, generate_canonical_selected
from SharedPortraitImportCatalog import (load_catalog, is_canonical, expand_runtime_entries, master_path,
    runtime_derivative_path, runtime_size, resolved_crop, pitch_composition, validate_generated_source)
ROOT=Path(__file__).resolve().parent.parent
class PitchMiniPilotTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.players=[e for e in load_catalog(ROOT) if is_canonical(e) and e.get('pilotStage') == '8.2A']
    def test_four_shared_outputs_reproduce_independently_of_full(self):
        self.assertEqual(len(self.players),4)
        for e in expand_runtime_entries(self.players,('Shared',)):
            with self.subTest(player=e['playerKey']):
                record=validate_generated_source(ROOT,e)
                data=encode_runtime_derivative(master_path(ROOT,e),runtime_size(e),resolved_crop(e),pitch_composition(e))
                self.assertEqual(data,runtime_derivative_path(ROOT,e).read_bytes())
                self.assertEqual(record['dimensions'],[512,768])
                self.assertEqual(record['pitchCompositionProfile'],'QuietPitchBust_v2')
                self.assertEqual(record['visualStatus'],'USER PIE ACCEPTED')
                self.assertNotEqual(runtime_derivative_path(ROOT,e), runtime_derivative_path(ROOT,dict(e,runtimeRole='Full')))
    def test_shared_first_activation_preserves_accepted_hand(self):
        records=json.loads((ROOT/'ContentSource/UI/PlayerPortraitRuntime/PlayerArtProvenance.json').read_text(encoding='utf-8'))['entries']
        with tempfile.TemporaryDirectory() as folder, patch.dict(os.environ,{'FMCODEX_PLAYER_ART_RUNTIME_ROLES':'Shared'}):
            root=Path(folder)
            prior=[]
            for e in self.players:
                for path in (master_path(ROOT,e),runtime_derivative_path(ROOT,dict(e,runtimeRole='Hand'))):
                    target=root/path.relative_to(ROOT);target.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(path,target)
                record=copy.deepcopy(next(r for r in records if r['playerKey']==e['playerKey']))
                record['roles'].pop('Shared');record['roles'].pop('Full',None);prior.append(record)
            generator=root/'Scripts/GenerateSharedPortraitRuntimeDerivatives.py';generator.parent.mkdir(parents=True)
            shutil.copyfile(ROOT/'Scripts/GenerateSharedPortraitRuntimeDerivatives.py',generator)
            provenance=root/'ContentSource/UI/PlayerPortraitRuntime/PlayerArtProvenance.json'
            provenance.write_text(json.dumps({'schemaVersion':2,'entries':prior}),encoding='utf-8')
            after=generate_canonical_selected(root,self.players)
            for old,new in zip(prior,after):
                self.assertEqual(new['roles']['Hand'],old['roles']['Hand'])
                self.assertEqual(set(new['roles']),{'Hand','Shared'})
            self.assertFalse(list(root.rglob('Full.png')))
    def test_pitch_metadata_and_resource_tampering_block_import(self):
        e=dict(self.players[0],runtimeRole='Shared')
        for rect in ([0,.5,1,.64],[0,.05,1,.2],[0,0,float('nan'),.64]):
            bad=copy.deepcopy(e);bad['cropOverrides']['pitchCropRect']=rect
            with self.assertRaises(RuntimeError):resolved_crop(bad)
        bad=copy.deepcopy(e);bad['cropOverrides']['pitchCropRect'][1]+=.01
        with self.assertRaises(RuntimeError):validate_generated_source(ROOT,bad)
        with self.assertRaises(RuntimeError):pitch_composition(dict(e,pitchCompositionProfile='Unknown'))
        original=Path.read_bytes
        for target in (runtime_derivative_path(ROOT,e),ROOT/'Scripts/GenerateSharedPortraitRuntimeDerivatives.py'):
            def tampered(path):return b'tampered' if path==target else original(path)
            with patch.object(Path,'read_bytes',tampered):
                with self.assertRaises(RuntimeError):validate_generated_source(ROOT,e)
if __name__=='__main__':unittest.main(verbosity=2)
