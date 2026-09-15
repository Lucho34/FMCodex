"""Objective source-space contracts; fixtures bind inspected regions to exact Masters.

Coordinates are analysis pixels (512x768), not crop or runtime offsets. Point
coverage checks do not grant visual acceptance. No ignored review files are inputs.
"""
import inspect
import unittest
from pathlib import Path
import cv2
import numpy as np
from PIL import Image
import PlayerPortraitForeground as v1
import PlayerPortraitForegroundV2 as v2

ROOT = Path(__file__).resolve().parent.parent
FIXTURES = [{'key': 'Prototype.Arsenal.BukayoSaka', 'sourcePixelSha256': '41B18E67D210FA45CBC66BDDCA55B28FBD9FE452ADD44C98D33C0DC951230E62', 'recoverPoints': [(174, 160)], 'backgroundPoints': [(120, 180), (375, 220)], 'protectionSample': False}, {'key': 'Prototype.Arsenal.WilliamSaliba', 'sourcePixelSha256': '4946443543A3C8CC143F18622DB12E10492A6AD6CFF5E7C3016C1EBB4BE4B2DE', 'recoverPoints': [(167, 177)], 'backgroundPoints': [(128, 175), (390, 220)], 'protectionSample': False}, {'key': 'Prototype.ManchesterCity.GianluigiDonnarumma', 'sourcePixelSha256': '2D996A3E87F9DA999A6BA0495121A87B403F348B216D057F2BAFED1A700BDE00', 'recoverPoints': [(303, 92), (328, 198), (313, 247)], 'backgroundPoints': [(350, 195), (420, 335)], 'protectionSample': False}, {'key': 'Prototype.ManchesterCity.PhilFoden', 'sourcePixelSha256': '5878A22AFEFA203E88D43F395C8CBFF3E1BECB6E2ACC7DC4CB5C117DB8C71ABE', 'recoverPoints': [(319, 108)], 'backgroundPoints': [(396, 263), (347, 107)], 'protectionSample': False}, {'key': 'Prototype.ManchesterCity.Rodri', 'sourcePixelSha256': 'CC79077A9227D4E12F6DCA1D6BD2DF89232B74A478914C5E3BDB86777D8527CA', 'recoverPoints': [(279, 65)], 'backgroundPoints': [(380, 180), (120, 200)], 'protectionSample': False}, {'key': 'Prototype.ManchesterCity.RubenDias', 'sourcePixelSha256': '978679E8E3489BFBAD6CE1AD636DAA7196986C1A0F57FE9C5587E6D4AAD150FA', 'recoverPoints': [(185, 147), (192, 277)], 'backgroundPoints': [(135, 175), (370, 180)], 'protectionSample': False}, {'key': 'Prototype.ManchesterCity.TijjaniReijnders', 'sourcePixelSha256': '9E5C470CA2364F6027BE4BE0AC74C52EAD4F851B5F5D76866C6BDBFC4C7D7A76', 'recoverPoints': [(303, 93), (174, 170)], 'backgroundPoints': [(342, 170), (150, 200)], 'protectionSample': False}, {'key': 'Prototype.ManchesterCity.NathanAke', 'sourcePixelSha256': '56AFAC5E3B671943358F1FD734755121CABC61A2B9E0382E416AC547273283F3', 'recoverPoints': [], 'backgroundPoints': [(100, 230), (405, 280)], 'protectionSample': True}, {'key': 'Prototype.Arsenal.DeclanRice', 'sourcePixelSha256': 'D8BE7FFBD16B8B68C9FE5338183CC0B711877F97029E85B63CAEAFB31D8E5496', 'recoverPoints': [], 'backgroundPoints': [(383, 215), (145, 165)], 'protectionSample': True}, {'key': 'Prototype.Arsenal.EberechiEze', 'sourcePixelSha256': '48913B4EC4F375A690C7A131865F5F005B736648C72163EB576F3274887C6078', 'recoverPoints': [], 'backgroundPoints': [(373, 242), (376, 250), (395, 270)], 'protectionSample': True}]


def coverage(alpha, point):
    x, y = point
    return float(np.asarray(alpha)[y-1:y+2, x-1:x+2].mean())


class PlayerPortraitForegroundV2Test(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.samples = {}
        for fixture in FIXTURES:
            source = Image.open(ROOT/'ArtSource/UI/PlayerMaster'/fixture['key']/'Master.png')
            if v1.source_pixel_hash(source) != fixture['sourcePixelSha256']:
                raise AssertionError('Contour fixture source changed; re-inspect source before updating fixture')
            basis = v2.extract_source_foreground(source)
            prior = v1.extract_source_foreground(source)
            cls.samples[fixture['key']] = (source, prior, basis)

    def test_explicit_version_and_no_purpose_crop_identity_inputs(self):
        self.assertEqual(list(inspect.signature(v2.extract_source_foreground).parameters), ['source', 'profile'])
        self.assertEqual(v2.FOREGROUND_PROFILE, 'SourceSpaceForeground_v2')
        self.assertEqual(v1.FOREGROUND_PROFILE, 'SourceSpaceForeground_v1')
        source, _, basis = self.samples[FIXTURES[0]['key']]
        for profile in ['SourceSpaceForeground_v1', 'unknown', 'SourceSpaceForeground_v2_candidate']:
            with self.assertRaises(ValueError):v2.extract_source_foreground(source, profile)
        implementation = Path(v2.__file__).read_text()
        for f in FIXTURES:self.assertNotIn(f['key'], implementation)
        self.assertFalse(basis.evidence['framingInputsUsed'])

    def test_determinism_and_source_binding(self):
        source, prior, basis = self.samples[FIXTURES[2]['key']]
        again = v2.extract_source_foreground(source)
        self.assertEqual(again.alpha_sha256, basis.alpha_sha256)
        self.assertEqual(again.evidence, basis.evidence)
        self.assertEqual(basis.source_pixel_sha256, v1.source_pixel_hash(source))
        self.assertEqual(basis.evidence['priorAlphaSha256'], prior.alpha_sha256)

    def test_invalid_source_fails_cleanly(self):
        with self.assertRaises(ValueError):v2.extract_source_foreground(Image.new('RGB', (512,768)))
        with self.assertRaises(RuntimeError):v2.extract_source_foreground(Image.new('RGB', (1024,1536), '#18232d'))

    def test_known_recovery_and_exterior_background_regions(self):
        for f in FIXTURES:
            _, prior, basis = self.samples[f['key']]
            with self.subTest(key=f['key']):
                for point in f['recoverPoints']:
                    self.assertLess(coverage(prior.alpha, point), 100, ('v1 failure fixture', point))
                    self.assertGreater(coverage(basis.alpha, point), 180, ('source-supported recovery', point))
                for point in f['backgroundPoints']:
                    self.assertLess(coverage(basis.alpha, point), 30, ('background exclusion', point))

    def test_protection_and_strictly_bounded_recovery(self):
        for f in FIXTURES:
            _, prior, basis = self.samples[f['key']]
            old, new = np.asarray(prior.alpha), np.asarray(basis.alpha)
            with self.subTest(key=f['key']):
                self.assertTrue(np.all(new >= old))
                l,t,r,b = basis.evidence['localBox']
                protected = np.ones(old.shape, bool)
                protected[max(0,t-2):min(768,b+2),max(0,l-2):min(512,r+2)] = False
                self.assertTrue(np.array_equal(new[protected], old[protected]))
                self.assertEqual(basis.evidence['newDilationIterations'], 0)
                self.assertEqual(basis.evidence['newClosingIterations'], 0)
                self.assertEqual(basis.evidence['recoveryEdgeSupportPixels'], 2)
                if f['protectionSample']:
                    self.assertLess(np.count_nonzero((new>200)&(old<40)), old.size*.001)

    def test_adjacent_ear_component_does_not_chain_background_islands(self):
        binary=np.zeros((220,220),np.uint8);seeds=np.zeros_like(binary,dtype=bool)
        binary[80:120,70:120]=1;seeds[90:105,90:105]=True
        binary[82:94,124:132]=1
        binary[82:94,137:145]=1
        binary[160:180,180:200]=1
        selected, records=v2._select_components(binary,seeds,[40,40,100,100],(0,0))
        self.assertTrue(selected[87,126])
        self.assertFalse(selected[87,140])
        self.assertFalse(selected[165,185])
        self.assertEqual([r['evidence'] for r in records],['seed','bounded-side-adjacency'])

    def test_edge_feather_preserves_unaffected_alpha_exactly(self):
        prior=Image.new('L',(512,768));arr=np.array(prior);arr[100:180,200:280]=255;arr[99,200:280]=37
        recovered=arr>127;recovered[125:135,280:286]=True
        result, count=v2._merge_recovery(Image.fromarray(arr),recovered)
        result=np.asarray(result);added=recovered&~(arr>127)
        distance=cv2.distanceTransform((~added).astype('uint8'),cv2.DIST_L2,5)
        self.assertEqual(count,60)
        self.assertTrue(np.array_equal(result[distance>2],arr[distance>2]))
        self.assertTrue(np.all(result>=arr))


if __name__=='__main__':unittest.main()
