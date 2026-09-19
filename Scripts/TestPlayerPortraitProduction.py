"""v1.3 production boundary, byte-driven lifecycle and fail-closed preflight."""
import copy
import json
from pathlib import Path
import tempfile
import unittest
from TestPlayerArtFinal40 import FINAL12, CHANGED5, expected_status
from unittest.mock import patch

from PIL import Image
from GenerateSharedPortraitRuntimeDerivatives import encode_runtime_derivative, prepare_family_source
from PlayerPortraitComposition import compose_portrait, HAND_PROFILE
from PlayerPortraitPreflight import (implementation_hashes, preflight_binding, validate_preflight,
                                    role_visual_status, REGIONS, IMPLEMENTATION_HASH_PROFILE,
                                    canonical_code_sha256)
from SharedPortraitImportCatalog import (load_catalog, is_canonical, expand_runtime_entries,
    master_path, runtime_size, resolved_crop, validate_generated_source, runtime_derivative_path)

ROOT = Path(__file__).resolve().parent.parent


def publish_in_test_workspace(root, entries):
    """Synthetic receipt only in a TemporaryDirectory, never the real project."""
    import shutil
    import GenerateSharedPortraitRuntimeDerivatives as generator
    from PlayerPortraitPreflight import IMPLEMENTATION_FILES
    from SharedPortraitImportCatalog import selected_runtime_roles
    if root.resolve() == ROOT.resolve() or (root/'FMCodex.uproject').exists():
        raise AssertionError('Synthetic review may never publish into the real project')
    for name in IMPLEMENTATION_FILES:
        destination = root/name
        destination.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(ROOT/name, destination)
    roles = selected_runtime_roles()
    records = generator.generate_canonical_selected(root, entries, preview_directory=root/'Review')
    receipt = {'schemaVersion':1, 'status':'PASS', 'implementationHashProfile':IMPLEMENTATION_HASH_PROFILE,
               'implementationSha256':implementation_hashes(root),
               'bindings':[preflight_binding(r, roles) for r in records],
               'inspections':{r['playerKey']:{'left':dict.fromkeys(REGIONS, 'PASS'),
                   'right':dict.fromkeys(REGIONS, 'PASS'), 'Hand':'PASS', 'Shared':'PASS', 'Full':'PASS'} for r in records}}
    path = root/'SyntheticTestReceipt.json'
    path.write_text(json.dumps(receipt), encoding='utf-8')
    return generator.generate_canonical_selected(root, entries, preflight_receipt=path)


class PlayerPortraitProductionTest(unittest.TestCase):
    def test_missing_receipt_never_writes_production(self):
        import GenerateSharedPortraitRuntimeDerivatives as generator
        entry = next(e for e in load_catalog(ROOT) if is_canonical(e))
        entry = dict(entry, familyRevision='1.3', foregroundExtractionProfile='SourceSpaceForeground_v3',
                     handCompositionProfile='BalancedBust_v3', pitchCompositionProfile='QuietPitchBust_v3')
        with patch.dict('os.environ', {'FMCODEX_PLAYER_ART_RUNTIME_ROLES':'Hand;Shared',
                                     'FMCODEX_PLAYER_ART_PREFLIGHT_RECEIPT':''}), \
                patch.object(generator, 'prepare_family_source', return_value=None), \
                patch.object(generator, 'encode_runtime_derivative', return_value=b'fixture'), \
                patch.object(generator, 'write_if_changed') as writes:
            with self.assertRaisesRegex(RuntimeError, 'receipt required'):
                generator.generate_canonical_selected(ROOT, [entry])
            writes.assert_not_called()

    def test_all_40_explicit_profiles_and_120_current_bindings(self):
        entries = [e for e in load_catalog(ROOT) if is_canonical(e)]
        self.assertEqual(len(entries), 40)
        for entry in entries:
            self.assertEqual(entry['familyRevision'], '1.3')
            self.assertEqual(entry['canonicalVisualStatus'], expected_status(entry['playerKey'],'Hand'))
            self.assertEqual(entry['roleStatus'], {role:expected_status(entry['playerKey'],role) for role in ('Hand','Shared','Full')})
            self.assertEqual(entry['foregroundExtractionProfile'], 'SourceSpaceForeground_v3')
            self.assertEqual(entry['handCompositionProfile'], 'BalancedBust_v3')
            self.assertEqual(entry['pitchCompositionProfile'], 'QuietPitchBust_v3')
        for entry in expand_runtime_entries(entries, ('Hand', 'Shared', 'Full')):
            with self.subTest(key=entry['playerKey'], role=entry['runtimeRole']):
                record = validate_generated_source(ROOT, entry)
                self.assertEqual(record['visualStatus'], expected_status(entry['playerKey'], entry['runtimeRole']))
                with Image.open(runtime_derivative_path(ROOT, entry)) as image:
                    self.assertEqual((image.mode, image.size), ('RGB', runtime_size(entry)))
                if entry['runtimeRole'] == 'Full':
                    self.assertNotIn('foregroundExtractionProfile', record)
                else:
                    self.assertEqual(record['foregroundExtractionProfile'], 'SourceSpaceForeground_v3')
                    self.assertEqual(record['framingAndBackgroundProfile'], 'SourceSpaceForeground_v1')

    def test_status_is_per_role_and_byte_driven(self):
        for old_status in ('USER PIE ACCEPTED', 'PENDING USER PIE'):
            old = {'visualStatus': old_status, 'runtimeDerivativeSha256': 'old'}
            self.assertEqual(role_visual_status(old, 'old'), old_status)
            self.assertEqual(role_visual_status(old, 'new'), 'PENDING USER PIE')
        self.assertEqual(role_visual_status({}, 'new'), 'PENDING USER PIE')

    def test_receipt_rejects_stale_or_partial_review(self):
        records = [{'playerKey':'fixture', 'masterSha256':'source', 'cropMetadataSha256':'crop',
                    'roles':{'Hand':{'runtimeDerivativeSha256':'hand'}, 'Shared':{'runtimeDerivativeSha256':'pitch'}}}]
        roles = ('Hand', 'Shared')
        receipt = {'schemaVersion':1, 'status':'PASS', 'implementationHashProfile':IMPLEMENTATION_HASH_PROFILE,
               'implementationSha256':implementation_hashes(ROOT),
                   'bindings':[preflight_binding(r, roles) for r in records],
                   'inspections':{'fixture':{'left':dict.fromkeys(REGIONS, 'PASS'),
                       'right':dict.fromkeys(REGIONS, 'PASS'), 'Hand':'PASS', 'Shared':'PASS'}}}
        with tempfile.TemporaryDirectory() as folder:
            path = Path(folder)/'receipt.json'
            path.write_text(json.dumps(receipt), encoding='utf-8')
            self.assertEqual(validate_preflight(path, ROOT, records, roles), receipt)
            for case in ('status', 'source', 'output', 'code', 'hash_profile', 'partial', 'side', 'composition'):
                bad = copy.deepcopy(receipt)
                if case == 'status': bad['status'] = 'PENDING'
                if case == 'source': bad['bindings'][0]['masterSha256'] = 'changed'
                if case == 'output': bad['bindings'][0]['roles']['Hand'] = 'changed'
                if case == 'code': bad['implementationSha256'] = {}
                if case == 'hash_profile': del bad['implementationHashProfile']
                if case == 'partial': bad['inspections'] = {}
                if case == 'side': del bad['inspections']['fixture']['right']['ear']
                if case == 'composition': bad['inspections']['fixture']['Hand'] = 'FAIL'
                path.write_text(json.dumps(bad), encoding='utf-8')
                with self.subTest(case=case), self.assertRaises(RuntimeError):
                    validate_preflight(path, ROOT, records, roles)
        with self.assertRaises(RuntimeError): validate_preflight(None, ROOT, records, roles)

    def test_code_hash_normalizes_only_newline_representation(self):
        import hashlib
        source = b'# documented behavior\nvalue = 12  \nlabel = "\xc3\xa9"\n'
        expected = hashlib.sha256(source).hexdigest().upper()
        for data in (source, source.replace(b'\n', b'\r\n'), source.replace(b'\n', b'\r')):
            self.assertEqual(canonical_code_sha256(data), expected)
        for data in (source.replace(b'value', b'other'), source.replace(b'12', b'13'),
                     source.replace(b'behavior', b'contract'), source.replace(b'12  ', b'12 '),
                     source.replace(b'\xc3\xa9', b'e\xcc\x81'), source.rstrip(b'\n')):
            self.assertNotEqual(canonical_code_sha256(data), expected)

    def test_current_full_receipt_requires_full_inspection_and_exact_output(self):
        generation = json.loads((ROOT/'ArtSource/UI/PlayerMaster/Stage8_5_Generation.json').read_text(encoding='utf-8'))
        receipt = generation['productionPromotion']['currentPreflightReceipts']['Full']
        provenance = json.loads((ROOT/'ContentSource/UI/PlayerPortraitRuntime/PlayerArtProvenance.json').read_text(encoding='utf-8'))['entries']
        by_key = {r['playerKey']:r for r in provenance}
        records = [by_key[b['playerKey']] for b in receipt['bindings']]
        self.assertEqual(len(records), 12)
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory)/'receipt.json'
            path.write_text(json.dumps(receipt), encoding='utf-8')
            validate_preflight(path, ROOT, records, ('Full',))
            for case in ('missing_full_inspection', 'wrong_full_output'):
                bad = copy.deepcopy(receipt)
                if case == 'missing_full_inspection':
                    del bad['inspections'][records[0]['playerKey']]['Full']
                else:
                    bad['bindings'][0]['roles']['Full'] = 'wrong'
                path.write_text(json.dumps(bad), encoding='utf-8')
                with self.subTest(case=case), self.assertRaises(RuntimeError):
                    validate_preflight(path, ROOT, records, ('Full',))

    def test_binary_hashes_remain_byte_exact(self):
        import hashlib
        from GenerateSharedPortraitRuntimeDerivatives import sha256_bytes, sha256_file
        from PlayerPortraitForeground import digest
        data = b'\x89PNG\r\n\x1a\n\x00binary\rpayload'
        expected = hashlib.sha256(data).hexdigest().upper()
        self.assertNotEqual(canonical_code_sha256(data), expected)
        self.assertEqual(sha256_bytes(data), expected)
        self.assertEqual(digest(data), expected)
        with tempfile.TemporaryDirectory() as folder:
            path = Path(folder)/'binary.fixture'
            path.write_bytes(data)
            self.assertEqual(sha256_file(path), expected)

    def test_current_receipt_and_bindings_survive_checkout_newlines(self):
        from PlayerPortraitPreflight import IMPLEMENTATION_FILES
        provenance = json.loads((ROOT/'ContentSource/UI/PlayerPortraitRuntime/PlayerArtProvenance.json').read_text(encoding='utf-8'))
        generation = json.loads((ROOT/'ArtSource/UI/PlayerMaster/Stage8_5_Generation.json').read_text(encoding='utf-8'))
        receipt = generation['productionPromotion']['currentPreflightReceipts']['HandShared']
        by_key = {e['playerKey']:e for e in provenance['entries']}
        ordered = [by_key[b['playerKey']] for b in receipt['bindings']]
        entry = next(e for e in load_catalog(ROOT) if is_canonical(e))
        original = Path.read_bytes
        code = {ROOT/name:original(ROOT/name).replace(b'\r\n', b'\n').replace(b'\r', b'\n')
                for name in IMPLEMENTATION_FILES}
        with tempfile.TemporaryDirectory() as folder:
            path = Path(folder)/'receipt.json'
            path.write_text(json.dumps(receipt), encoding='utf-8')
            for ending in (b'\n', b'\r\n', b'\r'):
                def represented(p):
                    return code[p].replace(b'\n', ending) if p in code else original(p)
                with self.subTest(ending=ending), patch.object(Path, 'read_bytes', represented):
                    self.assertEqual(implementation_hashes(ROOT), receipt['implementationSha256'])
                    validate_preflight(path, ROOT, ordered, ('Hand', 'Shared'))
                    for role in ('Hand', 'Shared'):
                        validate_generated_source(ROOT, dict(entry, runtimeRole=role))
            changed = ROOT/'Scripts/PlayerPortraitComposition.py'
            def real_change(p):
                return code[p]+b'# changed implementation comment\n' if p == changed else original(p)
            with patch.object(Path, 'read_bytes', real_change):
                with self.assertRaisesRegex(RuntimeError, 'Stale v1.3'):
                    validate_generated_source(ROOT, dict(entry, runtimeRole='Hand'))
                with self.assertRaisesRegex(RuntimeError, 'mismatch'):
                    validate_preflight(path, ROOT, ordered, ('Hand', 'Shared'))

    def test_partial_generator_writes_canonical_hashes_without_reencoding_frozen_hand(self):
        from unittest.mock import Mock
        import GenerateSharedPortraitRuntimeDerivatives as generator
        entry = next(e for e in load_catalog(ROOT) if is_canonical(e))
        records = json.loads((ROOT/'ContentSource/UI/PlayerPortraitRuntime/PlayerArtProvenance.json').read_text(encoding='utf-8'))['entries']
        previous = next(e for e in records if e['playerKey'] == entry['playerKey'])
        data = runtime_derivative_path(ROOT, dict(entry, runtimeRole='Shared')).read_bytes()
        def encode(*args, evidence=None, **kwargs):
            if evidence is not None:
                evidence['foregroundExtractionProfile'] = 'SourceSpaceForeground_v3'
            return data
        with tempfile.TemporaryDirectory() as folder, \
                patch.object(generator, 'selected_runtime_roles', return_value=('Shared',)), \
                patch.object(generator, 'prepare_family_source', return_value=(Mock(), Mock(), None)), \
                patch.object(generator, 'encode_runtime_derivative', side_effect=encode) as encoder, \
                patch.object(generator, 'write_if_changed'):
            result = generator.generate_canonical_selected(ROOT, [entry], preview_directory=Path(folder))
        self.assertEqual(encoder.call_count, 2, 'Only selected Shared is encoded and determinism-checked')
        role = result[0]['roles']['Shared']
        self.assertEqual(role['implementationHashProfile'], IMPLEMENTATION_HASH_PROFILE)
        self.assertEqual(role['implementationSha256'], implementation_hashes(ROOT))
        self.assertEqual(role['generatorSha256'], role['implementationSha256']['Scripts/GenerateSharedPortraitRuntimeDerivatives.py'])
        self.assertEqual(role['visualStatus'], 'USER PIE ACCEPTED')
        self.assertEqual(result[0]['roles']['Hand'], previous['roles']['Hand'])
        self.assertEqual(result[0]['roles']['Full'], previous['roles']['Full'])

    def test_production_uses_explicit_v3_rendering_and_one_source_basis(self):
        entry = next(e for e in load_catalog(ROOT) if e['playerKey'].endswith('.GianluigiDonnarumma'))
        path = master_path(ROOT, entry)
        with Image.open(path) as source:
            context = prepare_family_source(source)
            prior, foreground, anchors = context
            self.assertNotEqual(prior.alpha_sha256, foreground.alpha_sha256)
            _, old_fit = compose_portrait(source, prior, anchors, resolved_crop(dict(entry, runtimeRole='Hand')), HAND_PROFILE)
            with patch('GenerateSharedPortraitRuntimeDerivatives.prepare_family_source', side_effect=AssertionError('Must reuse source basis')):
                for role, profile in (('Hand', 'BalancedBust_v3'), ('Shared', 'QuietPitchBust_v3')):
                    e = dict(entry, runtimeRole=role); evidence = {}
                    data = encode_runtime_derivative(path, runtime_size(e), resolved_crop(e), profile,
                                                     family_source=context, evidence=evidence)
                    self.assertEqual(data, runtime_derivative_path(ROOT, e).read_bytes())
                    self.assertEqual(evidence['foregroundAlphaSha256'], foreground.alpha_sha256)
                    if role == 'Hand': self.assertEqual(evidence['compositionEvidence'], old_fit)
            wrong = copy.copy(foreground)
            from dataclasses import replace
            wrong = replace(wrong, source_pixel_sha256='wrong')
            with self.assertRaises(ValueError):
                compose_portrait(source, prior, anchors, [0,.055,1,.5], HAND_PROFILE, foreground=wrong)
        for profile in ('BalancedBust_v3_candidate', 'QuietPitchBust_v3_candidate', 'unknown'):
            with self.assertRaises(ValueError): encode_runtime_derivative(path, composition=profile)


if __name__ == '__main__': unittest.main(verbosity=2)
