"""Hand-only generation, provenance integrity, and retained legacy pipeline contracts."""
import copy
import hashlib
import json
import os
import shutil
import tempfile
from pathlib import Path
import unittest
from unittest.mock import patch
from GenerateSharedPortraitRuntimeDerivatives import encode_runtime_derivative, generate_canonical_selected
from SharedPortraitImportCatalog import (load_catalog, is_canonical, expand_runtime_entries,
    master_path, runtime_derivative_path, runtime_size, runtime_asset_name, asset_path,
    resolved_crop, hand_composition, validate_generated_source, selected_runtime_roles, select_entries)

ROOT = Path(__file__).resolve().parent.parent
ROLE_ENV = {"FMCODEX_PLAYER_ART_RUNTIME_ROLES": "Hand"}

class HandCompactPilotTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.catalog = load_catalog(ROOT)
        cls.players = [e for e in cls.catalog if is_canonical(e)]

    def fixture(self, root, players):
        for e in players:
            dest = master_path(root, e)
            dest.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(master_path(ROOT, e), dest)
        generator = root / 'Scripts/GenerateSharedPortraitRuntimeDerivatives.py'
        generator.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(ROOT / 'Scripts/GenerateSharedPortraitRuntimeDerivatives.py', generator)

    def test_four_hand_outputs_reproduce_with_rgb_dimensions_and_provenance(self):
        self.assertEqual({e['playerKey'] for e in self.players}, {
            'Prototype.Arsenal.BukayoSaka', 'Prototype.Arsenal.DavidRaya',
            'Prototype.ManchesterCity.Rodri', 'Prototype.ManchesterCity.ErlingHaaland'})
        for e in expand_runtime_entries(self.players, ('Hand',)):
            with self.subTest(player=e['playerKey']):
                record = validate_generated_source(ROOT, e)
                output = encode_runtime_derivative(master_path(ROOT,e), runtime_size(e), resolved_crop(e), hand_composition(e))
                self.assertEqual(output, runtime_derivative_path(ROOT,e).read_bytes())
                self.assertEqual(output, encode_runtime_derivative(master_path(ROOT,e), runtime_size(e), resolved_crop(e), hand_composition(e)))
                self.assertEqual(record['dimensions'], [192,128])
                self.assertEqual(record['visualStatus'], 'USER PIE ACCEPTED')

    def test_first_generation_requires_no_future_role_and_publishes_only_hand(self):
        with tempfile.TemporaryDirectory() as folder, patch.dict(os.environ, ROLE_ENV):
            root = Path(folder)
            self.fixture(root, self.players)
            records = generate_canonical_selected(root, self.players)
            self.assertEqual(len(records), 4)
            for e, record in zip(sorted(self.players,key=lambda e:e['playerKey']), records):
                self.assertEqual(set(record['roles']), {'Hand'})
                self.assertEqual(record['roleStatus'], {'Hand':'PENDING USER PIE','Shared':'DEFERRED','Full':'DEFERRED'})
                hand = dict(e,runtimeRole='Hand')
                self.assertEqual(runtime_derivative_path(root,hand).read_bytes(), runtime_derivative_path(ROOT,hand).read_bytes())
                validate_generated_source(root, hand)
            outputs = list((root/'ContentSource/UI/PlayerPortraitRuntime').rglob('*.png'))
            self.assertEqual(len(outputs), 4)
            self.assertTrue(all(p.name == 'Hand.png' for p in outputs))

    def test_existing_unselected_role_is_preserved_and_cannot_be_silently_invalidated(self):
        # A temporary future-role fixture exercises the guard without requiring
        # any deferred production PNG, package or backup to be present.
        with tempfile.TemporaryDirectory() as folder, patch.dict(os.environ, ROLE_ENV):
            root = Path(folder); e = self.players[0]
            self.fixture(root, [e])
            record = generate_canonical_selected(root, [e])[0]
            other = dict(e,runtimeRole='Full'); dest = runtime_derivative_path(root,other)
            data = encode_runtime_derivative(master_path(root,e),runtime_size(other),resolved_crop(other))
            dest.write_bytes(data)
            frozen = {'cropRect':resolved_crop(other),'dimensions':list(runtime_size(other)),
                'runtimeDerivativePath':dest.relative_to(root).as_posix(),
                'runtimeDerivativeSha256':hashlib.sha256(data).hexdigest().upper(),
                'runtimeAssetPath':asset_path(other)+'.'+runtime_asset_name(other),
                'importRecipe':'DesktopBC7OpaqueSharpen1_v1','visualStatus':'PENDING USER PIE'}
            record['roles']['Full'] = frozen
            provenance = root/'ContentSource/UI/PlayerPortraitRuntime/PlayerArtProvenance.json'
            provenance.write_text(json.dumps({'schemaVersion':2,'entries':[record]}),encoding='utf-8')
            result = generate_canonical_selected(root,[e])[0]
            self.assertEqual(result['roles']['Full'],frozen)
            self.assertEqual(dest.read_bytes(),data)
            changed = copy.deepcopy(e); changed.setdefault('cropOverrides',{})['fullCropRect']=[0,0,.8,.8]
            for bad in (changed,dict(e,masterRevision=99)):
                with patch('GenerateSharedPortraitRuntimeDerivatives.write_if_changed') as write:
                    with self.assertRaisesRegex(RuntimeError,'stale frozen Full'):
                        generate_canonical_selected(root,[bad])
                    write.assert_not_called()
            with patch('GenerateSharedPortraitRuntimeDerivatives.write_if_changed') as write:
                dest.write_bytes(b'tampered')
                with self.assertRaises(RuntimeError): generate_canonical_selected(root,[e])
                write.assert_not_called()
                dest.unlink()
                with self.assertRaises((RuntimeError,FileNotFoundError)): generate_canonical_selected(root,[e])
                write.assert_not_called()

    def test_crop_bounds_master_revision_and_recipe_block_import(self):
        e=dict(self.players[0],runtimeRole='Hand')
        for rect in ([0,0,1,1],[0,.7,1,.6],[0,.1,float('nan'),.6]):
            with self.assertRaises(RuntimeError): resolved_crop(dict(e,cropOverrides={'handCropRect':rect}))
        for bad, message in ((dict(e,masterSha256='0'*64),'Stale master'),
            (dict(e,masterRevision=99),'Stale crop/revision'),
            (dict(e,cropOverrides={'handCropRect':[0,.08,1,.6]}),'Stale crop')):
            with self.assertRaisesRegex(RuntimeError,message):validate_generated_source(ROOT,bad)
        with patch('SharedPortraitImportCatalog.hand_composition',return_value='CropOnly_v1'):
            with self.assertRaises(RuntimeError):validate_generated_source(ROOT,e)
        with self.assertRaises(RuntimeError):hand_composition(dict(e,handCompositionProfile='UpperTorsoNavy_v1'))

    def test_missing_or_tampered_master_generator_or_output_blocks_import(self):
        e=dict(self.players[0],runtimeRole='Hand'); original=Path.read_bytes
        for target in (master_path(ROOT,e),runtime_derivative_path(ROOT,e),ROOT/'Scripts/GenerateSharedPortraitRuntimeDerivatives.py'):
            with self.subTest(path=str(target)):
                def tampered(path):return b'tampered' if path==target else original(path)
                with patch.object(Path,'read_bytes',tampered):
                    with self.assertRaises(RuntimeError):validate_generated_source(ROOT,e)
                def missing(path):
                    if path==target:raise FileNotFoundError(str(path))
                    return original(path)
                with patch.object(Path,'read_bytes',missing):
                    with self.assertRaises(FileNotFoundError):validate_generated_source(ROOT,e)

    def test_explicit_selection_preserves_legacy_and_does_not_expand_future_roles(self):
        with patch.dict(os.environ,{},clear=True):
            with self.assertRaises(RuntimeError):select_entries(self.catalog)
            with self.assertRaises(RuntimeError):expand_runtime_entries(self.players)
            legacy=next(e for e in self.catalog if not is_canonical(e))
            self.assertEqual(len(expand_runtime_entries([legacy])),1)
            self.assertNotIn('PlayerMaster',str(master_path(ROOT,legacy)))
        key=self.players[0]['playerKey']
        with patch.dict(os.environ,{**ROLE_ENV,'FMCODEX_SHARED_PORTRAIT_PLAYER_KEYS':key}):
            entries=expand_runtime_entries(select_entries(self.catalog))
            self.assertEqual([(e['playerKey'],e['runtimeRole']) for e in entries],[(key,'Hand')])
        for raw in ('Hand;Unknown','Hand;Hand',';'):
            with patch.dict(os.environ,{'FMCODEX_PLAYER_ART_RUNTIME_ROLES':raw}):
                with self.assertRaises(RuntimeError):selected_runtime_roles()

if __name__ == '__main__':unittest.main(verbosity=2)
