"""Objective composition contracts, not subjective golden-image acceptance."""
from dataclasses import replace
import unittest
from unittest.mock import patch
from PIL import Image, ImageDraw

from PlayerPortraitForeground import ForegroundBasis, png_bytes
from PlayerPortraitComposition import (HAND_PROFILE, PITCH_PROFILE, measure_face_anchors,
    compose_portrait, fit_frame, current_frame)
from pathlib import Path
from PlayerPortraitForeground import extract_source_foreground
from SharedPortraitImportCatalog import load_catalog
from SharedPortraitImportCatalog import expand_runtime_entries, resolved_crop


class PlayerPortraitCompositionTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.samples = []
        root = Path(__file__).resolve().parent.parent
        # Real source fixtures; tests must not depend on ignored prototype caches.
        keys = ('TijjaniReijnders', 'OmarMarmoush', 'RubenDias', 'GianluigiDonnarumma',
                'RayanCherki', 'KaiHavertz', 'ViktorGyokeres', 'NathanAke', 'BukayoSaka', 'DeclanRice')
        catalog = {e['playerKey'].split('.')[-1]: e for e in load_catalog(root)}
        for key in keys:
            entry = catalog[key]
            source = Image.open(root/entry['masterSourcePath'])
            basis = extract_source_foreground(source)
            anchors = measure_face_anchors(source, basis)
            roles = {r['runtimeRole']: resolved_crop(r) for r in expand_runtime_entries([entry], ['Hand', 'Shared'])}
            cls.samples.append((entry, source, basis, anchors, roles))

    def test_family_fit_has_bounded_scale_position_and_torso(self):
        for profile, role, head_range, eye_range, crown_min, shoulder_max in (
                (HAND_PROFILE, 'Hand', (34, 40), (20.5, 24.5), 2, 59.5),
                (PITCH_PROFILE, 'Shared', (56, 66), (35, 43), 4, 106)):
            before, after = [], []
            for _, _, _, anchors, crops in self.samples:
                _, evidence = fit_frame(anchors, current_frame(crops[role], role), profile)
                b, a = evidence['before'], evidence['after'];before.append(b);after.append(a)
                self.assertTrue(head_range[0] <= a['headHeight'] <= head_range[1])
                self.assertTrue(eye_range[0] <= a['eyeY'] <= eye_range[1])
                self.assertGreaterEqual(a['hairTopY'], crown_min)
                self.assertLessEqual(a['shoulderY'], shoulder_max)
            for metric in ('headHeight', 'eyeY', 'centerX', 'belowChin'):
                spread = lambda rows: max(r[metric] for r in rows)-min(r[metric] for r in rows)
                self.assertLess(spread(after), .7*spread(before), (role, metric))

    def test_composition_is_deterministic_and_does_not_extract(self):
        _, source, basis, anchors, crops = self.samples[0]
        original = basis.alpha_sha256
        with patch('PlayerPortraitForeground.extract_source_foreground', side_effect=AssertionError('Composition may not extract')):
            for role, profile in (('Hand', HAND_PROFILE), ('Shared', PITCH_PROFILE)):
                first, evidence = compose_portrait(source, basis, anchors, crops[role], profile)
                second, repeat = compose_portrait(source, basis, anchors, crops[role], profile)
                self.assertEqual(png_bytes(first), png_bytes(second))
                self.assertEqual(evidence, repeat)
        self.assertEqual(original, basis.alpha_sha256)

    def test_whole_body_extent_does_not_drive_head_fit(self):
        _, source, basis, anchors, crops = self.samples[0]
        mask = basis.alpha.copy()
        ImageDraw.Draw(mask).rectangle((0, int(anchors.shoulder_y)+20, 511, 767), fill=255)
        changed = ForegroundBasis(basis.profile, basis.source_pixel_sha256, mask, basis.evidence)
        measured = measure_face_anchors(source, changed)
        self.assertEqual((anchors.eye_x, anchors.eye_y, anchors.head_height, anchors.shoulder_y),
                         (measured.eye_x, measured.eye_y, measured.head_height, measured.shoulder_y))
        original = current_frame(crops['Hand'], 'Hand')
        self.assertEqual(fit_frame(anchors, original, HAND_PROFILE)[0], fit_frame(measured, original, HAND_PROFILE)[0])

    def test_crop_is_a_bounded_preference_not_individual_alignment(self):
        _, _, basis, anchors, crops = self.samples[0]
        initial = basis.alpha_sha256
        results = [fit_frame(anchors, current_frame(crop, 'Hand'), HAND_PROFILE)[1]['after']
                   for crop in (crops['Hand'], [0, .055, 1, .5], [.04, .08, .92, .46])]
        self.assertLess(max(m['headHeight'] for m in results)-min(m['headHeight'] for m in results), 1)
        self.assertLess(max(m['centerX'] for m in results)-min(m['centerX'] for m in results), 2)
        self.assertEqual(initial, basis.alpha_sha256)

    def test_explicit_profiles_and_source_binding_reject_stale_inputs(self):
        _, source, basis, anchors, crops = self.samples[0]
        for profile in ('BalancedBust_v2', 'QuietPitchBust_v2', 'CropOnly_v1', 'unknown'):
            with self.assertRaises(ValueError):
                compose_portrait(source, basis, anchors, crops['Hand'], profile)
        changed = source.copy();changed.putpixel((0, 0), (1, 2, 3))
        with self.assertRaises(ValueError):
            compose_portrait(changed, basis, anchors, crops['Hand'], HAND_PROFILE)
        invalid = replace(anchors, evidence={**anchors.evidence, 'sourcePixelSha256': 'wrong'})
        with self.assertRaises(ValueError):
            compose_portrait(source, basis, invalid, crops['Hand'], HAND_PROFILE)

    def test_head_dominant_control_is_not_enlarged_and_hair_has_room(self):
        for entry, _, _, anchors, crops in self.samples[7:]:
            _, evidence = fit_frame(anchors, current_frame(crops['Hand'], 'Hand'), HAND_PROFILE)
            self.assertGreaterEqual(evidence['after']['hairTopY'], 2)
            if entry['playerKey'].endswith('.DeclanRice'):
                self.assertLessEqual(evidence['after']['headHeight'], evidence['before']['headHeight']+.5)
                self.assertGreater(evidence['after']['belowShoulderFlare'], evidence['before']['belowShoulderFlare'])


if __name__ == '__main__':
    unittest.main()
